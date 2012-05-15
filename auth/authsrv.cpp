#include "authsrv.hpp"

wondruss::authsrv::authsrv(asio::io_service& io_service, int fd)
  : listen(io_service, asio::local::stream_protocol(), fd)
{
  puts("Auth startup!\n");
  listen.send(asio::buffer("OK", 2));
  listen.async_receive(asio::null_buffers(), std::bind(std::mem_fn(&authsrv::handle_new_socket), this, std::placeholders::_1));
}

void wondruss::authsrv::handle_new_socket(const asio::error_code& error)
{
  if (error) {
    puts("Error receiving new socket!\n");
    // TODO: better error handling
  } else {
    char data[2], control[CMSG_SPACE(4)];
    puts("Holy crap, it's a socket!\n");
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
      printf("recvmsg error: %s\n", strerror(errno));
    } else {
      cmsg = CMSG_FIRSTHDR(&msg);
      if(cmsg && cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SCM_RIGHTS) {
        int fd = *((int*)CMSG_DATA(cmsg));
        close(fd);
        puts("Got a socket and closed it! we're awesome!");
      }
    }
  }
  listen.async_receive(asio::null_buffers(), std::bind(std::mem_fn(&authsrv::handle_new_socket), this, std::placeholders::_1));  
}
