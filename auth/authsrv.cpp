#include "authsrv.hpp"

#pragma pack(push, 1)
  struct AuthConnectionHeader
  {
    uint32_t header_size;
    uint8_t uuid[16];
  };
#pragma pack(pop)

static void handle_protocol_cruft(asio::ip::tcp::socket& sock)
{
  AuthConnectionHeader hdr;
  sock.receive(asio::buffer(&hdr, sizeof(AuthConnectionHeader)));
  if (hdr.header_size != sizeof(AuthConnectionHeader))
    printf("[auth]\tReceived an incorrectly-sized auth header\n");
  else
    printf("[auth]\tMoving on...\n");
  
}

wondruss::authsrv::authsrv(asio::io_service& io_service, int fd)
  : listen(io_service, asio::local::stream_protocol(), fd)
{
  listen.send(asio::buffer("OK", 2));
  listen.async_receive(asio::null_buffers(), std::bind(std::mem_fn(&authsrv::handle_new_socket), this, std::placeholders::_1));
}

void wondruss::authsrv::handle_new_socket(const asio::error_code& error)
{
  if (error) {
    printf("[auth]\tError receiving new socket!\n");
    // TODO: better error handling
  } else {
    printf("[auth]\tReceiving new socket from lobby\n");
    char data[2], control[CMSG_SPACE(4)];
    struct msghdr msg;
    struct cmsghdr* cmsg;
    struct iovec iov;

    iov.iov_base = data;
    iov.iov_len = 1;

    memset(&msg, 0, sizeof(msg));
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = control;
    msg.msg_controllen = sizeof(control);
    cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_len = CMSG_LEN(4);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;

    if(recvmsg(listen.native(), &msg, MSG_WAITALL) < 0) {
      printf("[auth]\tGot recvmsg error: %s\n", strerror(errno));
    } else {
      cmsg = CMSG_FIRSTHDR(&msg);
      if(cmsg && cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SCM_RIGHTS) {
        int fd = *((int*)CMSG_DATA(cmsg));
        asio::ip::tcp::socket new_socket(listen.get_io_service(), asio::ip::tcp::v6(), fd);
        printf("[auth]\tSuccessfully transferred %s from lobby.\n", new_socket.remote_endpoint().address().to_string().c_str());
        handle_protocol_cruft(new_socket);

        std::unique_ptr<client> new_client(new client(std::move(new_socket)));
        // TODO: listen on the client
      }
    }
  }
  listen.async_receive(asio::null_buffers(), std::bind(std::mem_fn(&authsrv::handle_new_socket), this, std::placeholders::_1));  
}
