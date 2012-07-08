#include "lobby.hpp"
#include "common/logger.hpp"

#include <functional>

enum class ConnectionType : uint8_t {
  CliToAuth = 10,
  CliToGame = 11,
  CliToFile = 16,
  CliToCsr  = 20,
  CliToGate = 22,
};

#pragma pack(push, 1)
  struct ConnectionHeader 
  {
    ConnectionType conn_type;
    uint16_t header_size;
    uint32_t build_id, build_type, branch_id;
    uint8_t product_id[16]; // TODO: UUID type
  };
#pragma pack(pop)

Wondruss::Lobby::Lobby(asio::io_service& io_service)
  : acceptor(io_service, asio::ip::tcp::endpoint(asio::ip::tcp::v6(), 14617))
    // on Linux, v6 listens to both protocols. I can't promise this works on other platforms
{
  int oldflags = fcntl(acceptor.native(), F_GETFD, 0);
  fcntl(acceptor.native(), F_SETFD, FD_CLOEXEC | oldflags);
  //TODO: only launch gate/auth when we're the master host
  auth = std::unique_ptr<auth_slave>(new auth_slave(io_service));
  // TODO: do we need to connect to a master host?
  start_accept();
}

void Wondruss::Lobby::start_accept()
{
  asio::ip::tcp::socket* socket = new asio::ip::tcp::socket(acceptor.get_io_service());
  acceptor.async_accept(*socket, std::bind(std::mem_fn(&Lobby::handle_accept), this, socket, std::placeholders::_1));
}

void Wondruss::Lobby::handle_accept(asio::ip::tcp::socket* socket, const asio::error_code& error)
{
  if (!error) {
    LOG_INFO("Got connection from ", socket->remote_endpoint().address().to_string().c_str());
    ConnectionHeader* hdr = new ConnectionHeader;
    int oldflags = fcntl(socket->native(), F_GETFD, 0);
    fcntl(socket->native(), F_SETFD, FD_CLOEXEC | oldflags);
    socket->async_receive(asio::buffer(hdr, sizeof(ConnectionHeader)), std::bind(std::mem_fn(&Lobby::handle_con_header), this, socket, hdr, std::placeholders::_1, std::placeholders::_2));
  } else {
    LOG_FATAL("Error on accept socket");
    // TODO: better error handling here
  }
  start_accept();
}

void Wondruss::Lobby::handle_con_header(asio::ip::tcp::socket* socket, ConnectionHeader* header, const asio::error_code& error, size_t bytes)
{
  if (error) {
    LOG_ERROR("Got error on header read from ", socket->remote_endpoint().address().to_string().c_str());
    // TODO: better error detection here
  } else {
    if(header->header_size > sizeof(ConnectionHeader)) {
      // For some reason extra data was written to the header.
      // we don't know what it was, but we should get rid of it and try to move on somehow
      size_t size = header->header_size-sizeof(ConnectionHeader);
      char* buf = new char[size];
      socket->receive(asio::buffer(buf, size));
      delete[] buf;
      LOG_WARN("Read ", size, "extra header byts from ", socket->remote_endpoint().address().to_string().c_str());
    }
    switch(header->conn_type) {
    case ConnectionType::CliToGate:
      break;
    case ConnectionType::CliToAuth:
      auth->handle_client(socket); // TODO: fail if we have no auth on this host
      break;
    case ConnectionType::CliToGame:
      break;
    case ConnectionType::CliToFile:
    case ConnectionType::CliToCsr:
    default:
      //TODO: print a warning
      break;
    }
  }
  delete socket;
  delete header;
}
