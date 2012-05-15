#include "auth/authsrv.hpp"

int main(int argc, char** argv)
{
  asio::io_service io_service;
  int fd = atoi(argv[1]);
  wondruss::authsrv authsrv(io_service, fd);
  io_service.run();
}
