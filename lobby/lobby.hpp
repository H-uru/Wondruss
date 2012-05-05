#include <asio.hpp>
#include "lobby_client.hpp"

#include <set>

namespace wondruss
{
  class lobby {
  public:
    lobby(asio::io_service&);
  private:
    void start_accept();
    void handle_accept(lobby_client::pointer, const asio::error_code&);

    asio::ip::tcp::acceptor acceptor;
    std::set<lobby_client::pointer> clients;
  };
}
