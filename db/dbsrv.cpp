#include "dbsrv.hpp"
#include "common/fds.hpp"
#include "common/logger.hpp"

Wondruss::DbSrv::DbSrv(asio::io_service& io_service)
  : rdsock(io_service, asio::local::stream_protocol(), FD_LBY_TO_SLV), wrsock(io_service, asio::local::stream_protocol(), FD_SLV_TO_LBY)
{
  wrsock.send(asio::buffer("OK", 2));
  rdsock.async_receive(asio::null_buffers(), std::bind(std::mem_fn(&DbSrv::handle_message), this, std::placeholders::_1));

  db.connect("10.254.0.199");
}

void Wondruss::DbSrv::send_response(MessageHeader header, MessageId id, Message* msg)
{
  std::stringstream stream;
  msg->SerializeToOstream(&stream);
  std::swap(header.destination, header.sender);
  header.size = stream.str().size();
  header.message = id;
  wrsock.send(asio::buffer(&header, sizeof(header)));
  wrsock.send(asio::buffer(stream.str().data(), header.size));
}

void Wondruss::DbSrv::handle_message(const asio::error_code& error)
{
  MessageHeader header;
  rdsock.receive(asio::buffer(&header, sizeof(header)));
  switch(header.message) {
  case MessageId::DbLoginRequest: {
    Db::LoginRequestMsg msg;
    msg.ParseFromFileDescriptor(rdsock.native());
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

  //TODO: verify password hash
  mongo::BSONObj query = BSON( "name" << request.username() );

  std::auto_ptr<mongo::DBClientCursor> result =
    db.query("wondruss.users", query);
  Db::LoginResponseMsg response;
  response.set_result(result->more());
  send_response(header, MessageId::DbLoginResponse, &response);
}
