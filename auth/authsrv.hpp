#include "asio.hpp"

#include "common/client.hpp"

namespace wondruss {
  class authsrv {
  public:
    authsrv(asio::io_service&, int);
  private:
    asio::local::stream_protocol::socket listen;

    void handle_new_socket(const asio::error_code&);

    struct client : public wondruss::client {
      client(asio::ip::tcp::socket&&s) : wondruss::client(std::move(s)) {}
    };
  };
}
