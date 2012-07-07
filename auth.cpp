#include "auth/authsrv.hpp"
#include "common/logger.hpp"

int main(int argc, char** argv)
{
  Wondruss::Logger::init("org.guildofwriters.wondruss.auth");
  asio::io_service io_service;
  int fd = atoi(argv[1]);
  Wondruss::AuthSrv authsrv(io_service, fd);
  io_service.run();
}
