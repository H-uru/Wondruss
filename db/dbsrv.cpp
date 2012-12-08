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

void Wondruss::DbSrv::send_response(MessageHeader header, MessageBase* msg)
{
  std::swap(header.destination, header.sender);
  header.size = msg->size();
  header.message = msg->id();
  wrsock.send(asio::buffer(&header, sizeof(header)));
  msg->write(wrsock);
}

void Wondruss::DbSrv::handle_message(const asio::error_code& error)
{
  MessageHeader header;
  rdsock.receive(asio::buffer(&header, sizeof(header)));
  switch(header.message) {
  case Message::DbLoginRequest: {
    Db::LoginRequestMsg msg;
    msg.read(rdsock);
    handle_login_request(header, msg);
    break;
  }
  default:
    LOG_ERROR("Got unknown response id: ", uint32_t(header.message));
    // TODO: Use the size() field to purge the message from the queue
  }

  rdsock.async_receive(asio::null_buffers(), std::bind(std::mem_fn(&DbSrv::handle_message), this, std::placeholders::_1));
}

void Wondruss::DbSrv::handle_login_request(const MessageHeader& header, const Db::LoginRequestMsg& request) {
  LOG_DEBUG("Got login request at DB");
  Db::LoginResponseMsg response;
  response.ok = 1;
  send_response(header, &response);
}
