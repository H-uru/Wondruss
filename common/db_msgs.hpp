#ifndef wondruss_common_db_msgs_hpp
#define wondruss_common_db_msgs_hpp

#include "msgs.hpp"

namespace Wondruss {
  namespace Db {
    struct LoginRequestMsg : public MessageBase {
      std::string username;
      std::array<uint32_t, 5> password;

      virtual void write(asio::local::stream_protocol::socket& socket) {
        uint32_t len;
        len = username.size();
        socket.send(asio::buffer(&len, 4));
        socket.send(asio::buffer(username.c_str(), len));
        socket.send(asio::buffer(password.data(), 20));
      }

      virtual void read(asio::local::stream_protocol::socket& socket) {
        uint32_t len;
        len = username.size();
        socket.receive(asio::buffer(&len, 4));
        username.resize(len, ' ');
        socket.receive(asio::buffer(const_cast<char*>(username.data()), len));
        socket.receive(asio::buffer(password.data(), 20));
      }

      virtual uint32_t size() {
        return 20 + 4 + username.size();
      }

      virtual Message id() { return Message::DbLoginRequest; }

      virtual ~LoginRequestMsg();
    };

    struct LoginResponseMsg : public MessageBase {
      uint8_t ok;

      virtual void write(asio::local::stream_protocol::socket& socket) {
        socket.send(asio::buffer(&ok, 1));
      }

      virtual void read(asio::local::stream_protocol::socket& socket) {
        socket.receive(asio::buffer(&ok, 1));
      }

      virtual uint32_t size() { return 1; }
      virtual Message id() { return Message::DbLoginResponse; }

      virtual ~LoginResponseMsg();
    };
  }
}

#endif

