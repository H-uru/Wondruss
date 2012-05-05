#include <asio.hpp>
#include <memory>

namespace wondruss
{
  class lobby_client : public std::enable_shared_from_this<lobby_client>
  {
  public:
    typedef std::shared_ptr<lobby_client> pointer;

    static pointer create(asio::io_service& io_service)
    {
      return pointer(new lobby_client(io_service));
    }

    asio::ip::tcp::socket& socket()
    {
      return socket_;
    }

  private:
    lobby_client(asio::io_service& io_service)
      : socket_(io_service) {}

    asio::ip::tcp::socket socket_;
  };
}
