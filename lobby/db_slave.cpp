#include "db_slave.hpp"
#include "lobby.hpp"
#include "common/fds.hpp"
#include "common/logger.hpp"

Wondruss::db_slave::db_slave(asio::io_service& io_service, Lobby* lobby)
  : lobby(lobby), rdsock(io_service), wrsock(io_service)
{
  asio::local::stream_protocol::socket child_rdsock(io_service);
  asio::local::stream_protocol::socket child_wrsock(io_service);
  asio::local::connect_pair(child_wrsock, rdsock);
  asio::local::connect_pair(wrsock, child_rdsock);
  fcntl(rdsock.native(), F_SETFD, FD_CLOEXEC);
  fcntl(wrsock.native(), F_SETFD, FD_CLOEXEC);
  io_service.notify_fork(asio::io_service::fork_prepare);
  pid_t newpid = fork();
  if (newpid == 0) {
    io_service.notify_fork(asio::io_service::fork_child);
    if(FD_LBY_TO_SLV != dup2(child_rdsock.native(), FD_LBY_TO_SLV)) {
      child_wrsock.send(asio::buffer("KO", 0));
      exit(0);
    }
    if(FD_SLV_TO_LBY != dup2(child_wrsock.native(), FD_SLV_TO_LBY)) {
      child_wrsock.send(asio::buffer("KO", 0));
      exit(0);
    }
    // TODO: find wondruss_auth a bit more sanely
    execl("./wondruss_db", "wondruss_db", nullptr);
    // if we get here, execl failed for some reason. Let's tell the lobby we failed.
    child_wrsock.send(asio::buffer("KO", 2));
    exit(0);
  }
  io_service.notify_fork(asio::io_service::fork_parent);
  char buf[3] = {0};
  rdsock.receive(asio::buffer(buf, 2)); //TODO: timeout
  if(strcmp(buf, "OK") != 0) {
    LOG_FATAL("DB process gave us a bad startup msg. WTF?");
    // TODO: fail spectacularly
  } else {
    LOG_INFO("DB startup OK! continuing...");
  }

  rdsock.async_receive(asio::null_buffers(), std::bind(std::mem_fn(&db_slave::handle_slave_msg), this, std::placeholders::_1));
}

void Wondruss::db_slave::handle_slave_msg(const asio::error_code& error)
{
  MessageHeader header;
  rdsock.receive(asio::buffer(&header, sizeof(header)));
  char* buf;
  buf = new char[header.size];
  asio::mutable_buffers_1 buffer = asio::buffer(buf, header.size);
  rdsock.receive(buffer);
  lobby->dispatch_message(header, buffer);
  delete[] buf;
  rdsock.async_receive(asio::null_buffers(), std::bind(std::mem_fn(&db_slave::handle_slave_msg), this, std::placeholders::_1));
}

void Wondruss::db_slave::send_message(MessageHeader header, asio::mutable_buffers_1 buf)
{
  wrsock.send(asio::buffer(&header, sizeof(header)));
  wrsock.send(buf);
}
