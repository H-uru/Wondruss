#include "lobby.hpp"

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

wondruss::lobby::lobby(asio::io_service& io_service)
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

void wondruss::lobby::start_accept()
{
  asio::ip::tcp::socket* socket = new asio::ip::tcp::socket(acceptor.get_io_service());
  acceptor.async_accept(*socket, std::bind(std::mem_fn(&lobby::handle_accept), this, std::unique_ptr<asio::ip::tcp::socket>(socket), std::placeholders::_1));
}

void wondruss::lobby::handle_accept(std::unique_ptr<asio::ip::tcp::socket>& socket, const asio::error_code& error)
{
  if (!error) {
    printf("[lobby]\tGot connection from %s\n", socket->remote_endpoint().address().to_string().c_str());
    ConnectionHeader* hdr = new ConnectionHeader;
    int oldflags = fcntl(socket->native(), F_GETFD, 0);
    fcntl(socket->native(), F_SETFD, FD_CLOEXEC | oldflags);
    asio::ip::tcp::socket* s = socket.get();
    s->async_receive(asio::buffer(hdr, sizeof(ConnectionHeader)), std::bind(std::mem_fn(&lobby::handle_con_header), this, std::move(socket), std::unique_ptr<ConnectionHeader>(hdr), std::placeholders::_1, std::placeholders::_2));
  } else {
    printf("[lobby]\tGot error on accept!\n");
    // TODO: better error handling here
  }
  start_accept();
}

void wondruss::lobby::handle_con_header(std::unique_ptr<asio::ip::tcp::socket>& socket, std::unique_ptr<ConnectionHeader>& header, const asio::error_code& error, size_t bytes)
{
  if (error) {
    printf("[lobby]\tGot error on header read from %s\n", socket->remote_endpoint().address().to_string().c_str());
    // TODO: better error detection here
  } else {
    if(header->header_size > sizeof(ConnectionHeader)) {
      // For some reason extra data was written to the header.
      // we don't know what it was, but we should get rid of it and try to move on somehow
      size_t size = header->header_size-sizeof(ConnectionHeader);
      char* buf = new char[size];
      socket->receive(asio::buffer(buf, size));
      delete[] buf;
      printf("[lobby]\tRead %lu extra header bytes from %s\n", size, socket->remote_endpoint().address().to_string().c_str());
    }
    switch(header->conn_type) {
    case ConnectionType::CliToGate:
      break;
    case ConnectionType::CliToAuth:
      auth->handleClient(std::move(socket)); // TODO: fail if we have no auth on this host
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
}

