// this is the base class for all client wrappers
// it exists primarily to handle encryption

#include "asio.hpp"

namespace wondruss {
  class client {
  public:
    client(asio::ip::tcp::socket&& s) : socket(std::move(s)) {
      {
        EncryptionHeader hdr;
        socket.receive(asio::buffer(&hdr, sizeof(EncryptionHeader)));
        if (hdr.size != 2 || hdr.msg_id != 0) {
          printf("[client]\tEncryption is not yet supported or bad message ID!\n");
          socket.close();
        }
        hdr.msg_id = 1;
        socket.send(asio::buffer(&hdr, sizeof(EncryptionHeader)));
      }
    }

    //TODO: encryption
    template<typename ConstBufferSequence, typename WriteHandler>
    void async_send(const ConstBufferSequence& buffers, WriteHandler handler) {
      socket.async_send(buffers, handler);
    }
    template<typename MutableBufferSequence, typename ReadHandler>
    void async_receive(const MutableBufferSequence& buffers, ReadHandler handler) {
      socket.async_receive(buffers, handler);
    }

  protected:
    typedef std::function<void> HandshakeHandler;
    void async_handshake(HandshakeHandler handler);
  private:
    asio::ip::tcp::socket socket;

#pragma pack(push, 1)
    struct EncryptionHeader {
      uint8_t msg_id;
      uint8_t size;
    };
#pragma pack(pop)
  };
}
