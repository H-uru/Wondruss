#ifndef WONDRUSS_COMMON_CLIENT
#define WONDRUSS_COMMON_CLIENT

// this is the base class for all client wrappers
// it exists primarily to handle encryption

#include "asio.hpp"
#include "logger.hpp"

#include <iconv.h>

namespace Wondruss {
  struct Client {
  public:
    Client(asio::ip::tcp::socket&& s) : socket(std::move(s)), data(0), data_size(0) {
      {
        EncryptionHeader hdr;
        socket.receive(asio::buffer(&hdr, sizeof(EncryptionHeader)));
        if (hdr.size != 2 || hdr.msg_id != 0) {
          LOG_ERROR("Client tried to enable encryption");
          socket.close();
        }
        hdr.msg_id = 1;
        socket.send(asio::buffer(&hdr, sizeof(EncryptionHeader)));
      }
    }

    const char* address() const {
      return socket.remote_endpoint().address().to_string().c_str();
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

    void flush() {
      LOG_DEBUG("Flushing ", data_size, " bytes of data to ", address());
      socket.send(asio::buffer(data, data_size));
      delete[] data;
      data = 0;
      data_size = 0;
    }

    template<typename ConstBufferSequence>
    void send(const ConstBufferSequence& buffers) {
      for (auto i = buffers.begin(); i != buffers.end(); ++i) {
        size_t buf_size = asio::buffer_size(*i);
        size_t new_size = buf_size + data_size;
        unsigned char* buf = const_cast<unsigned char*>(asio::buffer_cast<const unsigned char*>(*i));
        unsigned char* new_data = new unsigned char[new_size];
        memcpy(new_data, data, data_size);
        memcpy(new_data+data_size, buf, buf_size);
        delete[] data;
        data = new_data;
        data_size = new_size;
      }
    }

    template<typename MutableBufferSequence>
    std::size_t receive(const MutableBufferSequence& buffers) {
      return socket.receive(buffers);
    }

    template<typename T>
    inline void write(const T& v) {
      send(asio::buffer(&v, sizeof(T)));
    }

    template<typename T>
    inline T read() {
      T v;
      receive(asio::buffer(&v, sizeof(T)));
      return v;
    }

    asio::ip::tcp::socket socket;
    unsigned char* data;
    size_t data_size;

#pragma pack(push, 1)
    struct EncryptionHeader {
      uint8_t msg_id;
      uint8_t size;
    };
#pragma pack(pop)
  };

  template<>
  inline void Client::write<std::string>(const std::string& v) {
  }
  
  template<>
  inline std::string Client::read<std::string>() {
    std::string result;
    uint16_t len = read<uint16_t>();
    LOG_DEBUG("Reading a string of length ", len);
    uint16_t* buf = new uint16_t[len+1];
    char* newbuf = new char[len*3+1]; // assumes all characters are in the BMP
    buf[len] = 0;
    newbuf[len*3] = 0;
    receive(asio::buffer(buf, len*2)); // TODO: error checking
    iconv_t conv = iconv_open("UTF-8", "UTF-16");
    size_t reallen = len*2;
    size_t realnewlen = len*3;
    char* realbuf = (char*)buf;
    char* realnewbuf = newbuf;
    iconv(conv, &realbuf, &reallen, &realnewbuf, &realnewlen);
    iconv_close(conv);
    result = newbuf;
    LOG_DEBUG("Read string: ", newbuf);
    delete[] buf;
    delete[] newbuf;
    return result;
  }
}

#endif // WONDRUSS_COMMON_CLIENT
