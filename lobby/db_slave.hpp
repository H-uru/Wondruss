#ifndef wondruss_lobby_db_slave_hpp
#define wondruss_lobby_db_slave_hpp

#include "common/asio.hpp"
#include "common/msgs.hpp"

namespace Wondruss
{
  class Lobby;

  class db_slave
  {
  public:
    db_slave(asio::io_service&, Lobby*);
    void send_message(MessageHeader, asio::mutable_buffers_1);

  private:
    Lobby* lobby;
    asio::local::stream_protocol::socket rdsock;
    asio::local::stream_protocol::socket wrsock;

    void handle_slave_msg(const asio::error_code&);
  };
}

#endif
