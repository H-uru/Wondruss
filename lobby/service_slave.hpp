#ifndef wondruss_lobby_service_slave_hpp
#define wondruss_lobby_service_slave_hpp

#include "slave.hpp"

namespace Wondruss
{
  class service_slave : public slave
  {
  public:
    service_slave(asio::io_service&, Lobby*);
    void handle_client(asio::ip::tcp::socket*);

  protected:
    asio::local::stream_protocol::socket fdsock;
  };
}

#endif
