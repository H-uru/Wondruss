#include "dbsrv.hpp"
#include "common/fds.hpp"
#include "common/logger.hpp"

Wondruss::DbSrv::DbSrv(asio::io_service& io_service)
  : rdsock(io_service, asio::local::stream_protocol(), FD_LBY_TO_SLV), wrsock(io_service, asio::local::stream_protocol(), FD_SLV_TO_LBY)
{
  wrsock.send(asio::buffer("OK", 2));
  rdsock.async_receive(asio::null_buffers(), std::bind(std::mem_fn(&DbSrv::handle_message), this, std::placeholders::_1));
  //TODO: listen on rdsock for messages from other processes
}

void Wondruss::DbSrv::handle_message(const asio::error_code& error)
{
  // TODO: read the message from lobby and respond
}
