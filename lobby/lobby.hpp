#include <asio.hpp>

#include <set>

struct ConnectionHeader;

namespace wondruss
{
  class lobby {
  public:
    lobby(asio::io_service&);
  private:
    void start_accept();
    void handle_accept(asio::ip::tcp::socket*, const asio::error_code&);
    void handle_con_header(asio::ip::tcp::socket*, ConnectionHeader*, const asio::error_code&, size_t);

    asio::ip::tcp::acceptor acceptor;
  };
}
