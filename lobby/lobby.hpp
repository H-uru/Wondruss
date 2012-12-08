#include "auth_slave.hpp"
#include "db_slave.hpp"
#include "common/asio.hpp"

#include <set>
#include <memory>

struct ConnectionHeader;

namespace Wondruss
{
  class Lobby {
  public:
    Lobby(asio::io_service&);

    void dispatch_message(MessageHeader, asio::mutable_buffers_1);
  private:
    void start_accept();
    void handle_accept(asio::ip::tcp::socket*, const asio::error_code&);
    void handle_con_header(asio::ip::tcp::socket*, ConnectionHeader*, const asio::error_code&, size_t);

    asio::ip::tcp::acceptor acceptor;
    std::unique_ptr<auth_slave> auth;
    std::unique_ptr<db_slave> db;
  };
}
