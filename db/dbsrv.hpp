#include "common/asio.hpp"

namespace Wondruss {
  class DbSrv {
  public:
    DbSrv(asio::io_service&);
  private:

    // ASIO protocol callbacks
    void handle_message(const asio::error_code&);

    asio::local::stream_protocol::socket rdsock;
    asio::local::stream_protocol::socket wrsock;
  };
}
