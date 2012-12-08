#include "common/asio.hpp"

namespace Wondruss
{
  class db_slave
  {
  public:
    db_slave(asio::io_service&);

  private:
    asio::local::stream_protocol::socket rdsock;
    asio::local::stream_protocol::socket wrsock;

    void handle_slave_msg(const asio::error_code&);
  };
}
