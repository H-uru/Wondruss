#ifndef wondruss_common_msgs_hpp
#define wondruss_common_msgs_hpp

#include "common/asio.hpp"
#include "db.pb.h"

#include <boost/uuid/uuid.hpp>

using google::protobuf::Message;

namespace Wondruss {
  enum class MessageId : uint32_t {
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
    MessageId message;
  };
}

#endif
