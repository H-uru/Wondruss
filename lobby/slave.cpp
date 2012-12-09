#include "slave.hpp"
#include "lobby.hpp"
#include "common/fds.hpp"
#include "common/logger.hpp"

Wondruss::slave::slave(asio::io_service& io_service, Lobby* lobby)
  : lobby(lobby), rdsock(io_service), wrsock(io_service)
{}

void Wondruss::slave::handle_slave_msg(const asio::error_code& error)
{
  MessageHeader header;
  rdsock.receive(asio::buffer(&header, sizeof(header)));
  char* buf;
  buf = new char[header.size];
  asio::mutable_buffers_1 buffer = asio::buffer(buf, header.size);
  rdsock.receive(buffer);
  lobby->dispatch_message(header, buffer);
  delete[] buf;
  rdsock.async_receive(asio::null_buffers(), std::bind(std::mem_fn(&db_slave::handle_slave_msg), this, std::placeholders::_1));
}

void Wondruss::slave::send_message(MessageHeader header, asio::mutable_buffers_1 buf)
{
  wrsock.send(asio::buffer(&header, sizeof(header)));
  wrsock.send(buf);
}
