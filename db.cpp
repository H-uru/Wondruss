#include "db/dbsrv.hpp"
#include "common/logger.hpp"

int main(int argc, char** argv)
{
  Wondruss::Logger::init("org.guildofwriters.wondruss.db");
  asio::io_service io_service;
  Wondruss::DbSrv dbsrv(io_service);
  io_service.run();
}
