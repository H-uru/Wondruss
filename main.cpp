#include <asio.hpp>

#include "lobby/lobby.hpp"

int main(int argc, char** argv) {
  //TODO: setup some sort of logging provider
  asio::io_service io_service;
  wondruss::lobby lobby(io_service);
  io_service.run();
}
