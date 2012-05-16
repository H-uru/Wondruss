#include "asio.hpp"

#include <memory>

namespace wondruss
{
  class auth_slave
  {
  public:
    auth_slave(asio::io_service&);
    void handleClient(std::unique_ptr<asio::ip::tcp::socket>);

  private:
    asio::local::stream_protocol::socket fdsock;
  };
}
