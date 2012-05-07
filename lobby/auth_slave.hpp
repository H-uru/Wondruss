#include <asio.hpp>

namespace wondruss
{
  class auth_slave
  {
  public:
    auth_slave(asio::io_service&);
    void handleClient(asio::ip::tcp::socket*);

  private:
    asio::local::stream_protocol::socket fdsock;
  };
}
