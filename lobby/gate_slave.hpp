#include <asio.hpp>

namespace wondruss
{
  class gate_slave
  {
  public:
    gate_slave(asio::io_service&);
    void handleClient(asio::ip::tcp::socket*);

  private:
    asio::local::stream_protocol::socket fdsock;
  };
}
