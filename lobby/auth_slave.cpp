#include "auth_slave.hpp"
#include "common/fds.hpp"
#include "common/logger.hpp"

Wondruss::auth_slave::auth_slave(asio::io_service& io_service)
  : fdsock(io_service), rdsock(io_service), wrsock(io_service)
{
  asio::local::stream_protocol::socket child_fdsock(io_service);
  asio::local::stream_protocol::socket child_rdsock(io_service);
  asio::local::stream_protocol::socket child_wrsock(io_service);
  asio::local::connect_pair(fdsock, child_fdsock);
  asio::local::connect_pair(rdsock, child_wrsock);
  asio::local::connect_pair(wrsock, child_rdsock);
  fcntl(fdsock.native(), F_SETFD, FD_CLOEXEC);
  fcntl(rdsock.native(), F_SETFD, FD_CLOEXEC);
  fcntl(wrsock.native(), F_SETFD, FD_CLOEXEC);
  io_service.notify_fork(asio::io_service::fork_prepare);
  pid_t newpid = fork();
  if (newpid == 0) {
    io_service.notify_fork(asio::io_service::fork_child);
    if(FD_SOCKETS != dup2(child_fdsock.native(), FD_SOCKETS)) {
      child_fdsock.send(asio::buffer("KO", 0));
      exit(0);
    }
    if(FD_SLV_TO_LBY != dup2(child_rdsock.native(), FD_SLV_TO_LBY)) {
      child_fdsock.send(asio::buffer("KO", 0));
      exit(0);
    }
    if(FD_LBY_TO_SLV != dup2(child_wrsock.native(), FD_LBY_TO_SLV)) {
      child_fdsock.send(asio::buffer("KO", 0));
      exit(0);
    }
    // TODO: find wondruss_auth a bit more sanely
    execl("./wondruss_auth", "wondruss_auth", nullptr);
    // if we get here, execl failed for some reason. Let's tell the lobby we failed.
    child_fdsock.send(asio::buffer("KO", 0));
    exit(0);
  }
  io_service.notify_fork(asio::io_service::fork_parent);
  char buf[3] = {0};
  fdsock.receive(asio::buffer(buf, 2)); //TODO: timeout
  if(strcmp(buf, "OK") != 0) {
    LOG_FATAL("Auth process gave us a bad startup msg. WTF?");
    // TODO: fail spectacularly
  } else {
    LOG_INFO("Auth startup OK! continuing...");
  }

  rdsock.async_receive(asio::null_buffers(), std::bind(std::mem_fn(&auth_slave::handle_slave_msg), this, std::placeholders::_1));
}

void Wondruss::auth_slave::handle_client(asio::ip::tcp::socket*sock)
{
  struct msghdr msg;
  struct cmsghdr *cmsg;
  struct iovec iov;
  char buf[CMSG_SPACE(4)];

  iov.iov_base = &sock;
  iov.iov_len = 1;

  memset(&msg, 0, sizeof(msghdr));
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;
  msg.msg_control = buf;
  msg.msg_controllen = CMSG_SPACE(4);
  cmsg = CMSG_FIRSTHDR(&msg);
  cmsg->cmsg_level = SOL_SOCKET;
  cmsg->cmsg_type = SCM_RIGHTS;
  cmsg->cmsg_len = CMSG_LEN(sizeof(int));
  *(int *) CMSG_DATA(cmsg) = sock->native();
  msg.msg_controllen = cmsg->cmsg_len;

  if(sendmsg(fdsock.native(), &msg, 0) < 0)
    LOG_ERROR("Could not send socket to auth: ", strerror(errno));
}

void Wondruss::auth_slave::handle_slave_msg(const asio::error_code& error)
{
  //TODO: Read message from rdsock and pass it on to some other process
}
