#include "auth/authsrv.hpp"
#include "common/fds.hpp"
#include "common/logger.hpp"

int main(int argc, char** argv)
{
  Wondruss::Logger::init("org.guildofwriters.wondruss.auth");
  asio::io_service io_service;
  Wondruss::AuthSrv authsrv(io_service, FD_SOCKETS);
  io_service.run();
}
