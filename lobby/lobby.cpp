#include "lobby.hpp"
#include "gate_slave.hpp"

#include <functional>

using asio::ip::tcp;

enum ConnectionType {
  ConnCliToAuth = 10,
  ConnCliToGame = 11,
  ConnCliToFile = 16,
  ConnCliToCsr  = 20,
  ConnCliToGate = 22,
};

#pragma pack(push, 1)
  struct ConnectionHeader 
  {
    uint8_t conn_type;
    uint16_t header_size;
    uint32_t build_id, build_type, branch_id;
    uint8_t uuid[16];
  };
#pragma pack(pop)

wondruss::lobby::lobby(asio::io_service& io_service)
  : acceptor(io_service, asio::ip::tcp::endpoint(asio::ip::tcp::v6(), 14617))
    // on Linux, v6 listens to both protocols. I can't promise this works on other platforms
{
  //TODO: only launch gate/auth when we're the master host
  gate = new gate_slave(io_service);
  // TODO: do we need to connect to a master host?
  start_accept();
}

void wondruss::lobby::start_accept()
{
  tcp::socket* socket = new tcp::socket(acceptor.get_io_service());
  acceptor.async_accept(*socket, std::bind(std::mem_fn(&lobby::handle_accept), this, socket, std::placeholders::_1));
}

void wondruss::lobby::handle_accept(tcp::socket* socket, const asio::error_code& error)
{
  if (!error) {
    ConnectionHeader* hdr = new ConnectionHeader;
    int oldflags = fcntl(socket->native(), F_GETFD, 0);
    fcntl(socket->native(), F_SETFD, FD_CLOEXEC | oldflags);
    socket->async_receive(asio::buffer(hdr, sizeof(ConnectionHeader)), std::bind(std::mem_fn(&lobby::handle_con_header), this, socket, hdr, std::placeholders::_1, std::placeholders::_2));
  }
  start_accept();
}

void wondruss::lobby::handle_con_header(tcp::socket* socket, ConnectionHeader* header, const asio::error_code& error, size_t bytes)
{
  if (error) {
    puts("Error on header read!\n");
    // TODO: better error detection here
  } else {
    if(header->header_size > sizeof(header)) {
      // For some reason extra data was written to the header.
      // we don't know what it was, but we should get rid of it and try to move on somehow
      size_t size = header->header_size-sizeof(header);
      char* buf = new char[size];
      socket->receive(asio::buffer(buf, size));
      delete[] buf;
    }
    switch(header->conn_type) {
    case ConnCliToAuth:
      break;
    case ConnCliToGame:
      break;
    case ConnCliToFile:
      break;
    case ConnCliToCsr:
      break;
    case ConnCliToGate:
      gate->handleClient(socket);
      break;
    default:
      //TODO: print a warning
      break;
    }
  }
  // At this point we've sent the socket to the slave process. It can go away.
  delete socket;
  delete header;
}

