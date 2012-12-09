#ifndef wondruss_lobby_slave_hpp
#define wondruss_lobby_slave_hpp

#include "common/asio.hpp"
#include "common/msgs.hpp"

namespace Wondruss
{
  class Lobby;

  class slave
  {
  public:
    slave(asio::io_service&, Lobby*);
    void send_message(MessageHeader, asio::mutable_buffers_1);

  protected:
    Lobby* lobby;
    asio::local::stream_protocol::socket rdsock;
    asio::local::stream_protocol::socket wrsock;

    void handle_slave_msg(const asio::error_code&);
  };
}


#endif
