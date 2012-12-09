#ifndef wondruss_lobby_db_slave_hpp
#define wondruss_lobby_db_slave_hpp

#include "slave.hpp"

namespace Wondruss
{
  class Lobby;

  class db_slave : public slave
  {
  public:
    db_slave(asio::io_service&, Lobby*);
  };
}

#endif
