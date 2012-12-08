#ifndef wondruss_common_msgs_hpp
#define wondruss_common_msgs_hpp

#include "common/asio.hpp"

#include <boost/uuid/uuid.hpp>

namespace Wondruss {
  enum class Message : uint32_t {
    DbLoginRequest,
    // Responses below here
    ResponseBase,
    DbLoginResponse,
  };
  struct MessageHeader {
    boost::uuids::uuid destination;
    boost::uuids::uuid sender;
    uint32_t transaction;
    uint32_t size;
    Message message;
  };

  struct MessageBase {
    virtual ~MessageBase();
    virtual void read(asio::local::stream_protocol::socket&) =0;
    virtual void write(asio::local::stream_protocol::socket&) =0;
    virtual uint32_t size() =0;
    virtual Message id() =0;
  };
}

#endif
