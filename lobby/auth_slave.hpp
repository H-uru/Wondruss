#ifndef wondruss_lobby_auth_slave_hpp
#define wondruss_lobby_auth_slave_hpp

#include "service_slave.hpp"

namespace Wondruss
{
  class Lobby;

  class auth_slave : public service_slave
  {
  public:
    auth_slave(asio::io_service&, Lobby*);
  };
}

#endif
