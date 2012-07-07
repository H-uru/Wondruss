#include "asio.hpp"

#include <memory>

namespace Wondruss
{
  class auth_slave
  {
  public:
    auth_slave(asio::io_service&);
    void handle_client(std::unique_ptr<asio::ip::tcp::socket>);

  private:
    asio::local::stream_protocol::socket fdsock;
  };
}
