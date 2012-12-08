#include "common/asio.hpp"

namespace Wondruss
{
  class auth_slave
  {
  public:
    auth_slave(asio::io_service&);
    void handle_client(asio::ip::tcp::socket*);

  private:
    asio::local::stream_protocol::socket fdsock;
    asio::local::stream_protocol::socket rdsock;
    asio::local::stream_protocol::socket wrsock;

    void handle_slave_msg(const asio::error_code&);
  };
}
