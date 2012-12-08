#include "common/asio.hpp"
#include "common/msgs.hpp"

namespace Wondruss {
  class DbSrv {
  public:
    DbSrv(asio::io_service&);
  private:

    // ASIO protocol callbacks
    void handle_message(const asio::error_code&);

    void send_response(MessageHeader, MessageId, Message*);
    void handle_login_request(const MessageHeader&, const Db::LoginRequestMsg&);

    asio::local::stream_protocol::socket rdsock;
    asio::local::stream_protocol::socket wrsock;
  };
}
