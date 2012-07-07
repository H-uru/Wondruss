#include "lobby/lobby.hpp"
#include "common/logger.hpp"

int main(int argc, char** argv) {
  Wondruss::Logger::init("org.guildofwriters.wondruss.lobby");
  asio::io_service io_service;
  Wondruss::Lobby lobby(io_service);
  io_service.run();
}
