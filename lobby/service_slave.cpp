#include "service_slave.hpp"
#include "lobby.hpp"
#include "common/logger.hpp"

Wondruss::service_slave::service_slave(asio::io_service& io_service, Lobby* lobby)
  : slave(io_service, lobby), fdsock(io_service)
{}

void Wondruss::service_slave::handle_client(asio::ip::tcp::socket*sock)
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
    LOG_ERROR("Could not send socket to slave: ", strerror(errno));
}
