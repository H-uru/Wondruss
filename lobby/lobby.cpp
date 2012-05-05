#include "lobby.hpp"

#include <functional>

using asio::ip::tcp;

#pragma pack(push, 1)
  struct ConnectionHeader 
  {
    uint8_t conn_type;
    uint16_t header_size;
    uint32_t build_id, build_type, branch_id;
  };
#pragma pack(pop)

wondruss::lobby::lobby(asio::io_service& io_service)
  : acceptor(io_service, asio::ip::tcp::endpoint(asio::ip::tcp::v6(), 14617))
    // on Linux, v6 listens to both protocols. I can't promise this works on other platforms
{
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
    socket->async_receive(asio::buffer(hdr, sizeof(ConnectionHeader)), std::bind(std::mem_fn(&lobby::handle_con_header), this, socket, hdr, std::placeholders::_1, std::placeholders::_2));
  }
  start_accept();
}

void wondruss::lobby::handle_con_header(tcp::socket* socket, ConnectionHeader* header, const asio::error_code& error, size_t bytes)
{
  if (error) {
    delete socket;
    puts("Error on header read!\n");
    // TODO: better error detection here
  } else {
    uint16_t conn_type = header->conn_type;
    printf("Got connection header:\n\
\tconn_type  = %hu\n\
\thdr_size   = %hu\n\
\tbuild_id   = %u\n\
\tbuild_type = %u\n\
\tbranch_id  = %u\n", conn_type, header->header_size, header->build_id, header->build_type, header->branch_id);
    // TODO: validate header and move on with life.
    delete socket;
  }
  delete header;
}

