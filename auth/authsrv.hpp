#include "asio.hpp"

namespace wondruss {
  class authsrv {
  public:
    authsrv(asio::io_service&, int);
  private:
    asio::local::stream_protocol::socket listen;

    void handle_new_socket(const asio::error_code&);
  };
}
