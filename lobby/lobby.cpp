#include "lobby.hpp"

#include <functional>

wondruss::lobby::lobby(asio::io_service& io_service)
  : acceptor(io_service, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 14617))
{
  start_accept();
}

void wondruss::lobby::start_accept()
{
  lobby_client::pointer new_conn = lobby_client::create(acceptor.get_io_service());
  acceptor.async_accept(new_conn->socket(), std::bind(std::mem_fn(&lobby::handle_accept), this, new_conn, std::placeholders::_1));
}

void wondruss::lobby::handle_accept(lobby_client::pointer client, const asio::error_code& error)
{
  if (!error) {
    clients.insert(client);
  }
  start_accept();
}

