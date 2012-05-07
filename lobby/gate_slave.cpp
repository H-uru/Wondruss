#include "gate_slave.hpp"

wondruss::gate_slave::gate_slave(asio::io_service& io_service)
  : fdsock(io_service)
{
  asio::local::stream_protocol::socket child_sock(io_service);
  asio::local::connect_pair(fdsock, child_sock);
  fcntl(fdsock.native(), F_SETFD, FD_CLOEXEC);
  io_service.notify_fork(asio::io_service::fork_prepare);
  pid_t newpid = fork();
  if (newpid == 0) {
    io_service.notify_fork(asio::io_service::fork_child);
    char buf[10];
    sprintf(buf, "%u", child_sock.native());
    //execvp("wondruss_gate", "wondruss_gate", buf, nullptr);
    puts("Should be running 'exec' right now, but gatekeeper is still unimplemented\n");
    exit(0);
  }
  io_service.notify_fork(asio::io_service::fork_parent);
  //TODO: read from the socket so we know when the gatekeeper is ready.
}

void wondruss::gate_slave::handleClient(asio::ip::tcp::socket* sock)
{
  //TODO: forward the socket to the child process.
}
