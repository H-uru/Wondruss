#include "common/asio.hpp"
#include "common/client.hpp"

#include <set>

namespace Wondruss {
  class AuthSrv {
  public:
    AuthSrv(asio::io_service&, int);
  private:
    struct Client : public Wondruss::Client {
      Client(asio::ip::tcp::socket&&s) : Wondruss::Client(std::move(s)) {}

      std::string name() const {
        if(account.size() == 0)
          return address();
        else
          return address() + " <" + account + ">";
      }

      uint32_t build_id;
      uint32_t server_challenge;
      uint32_t client_challenge;

      std::string account;

      uint8_t account_uuid[16];
    };

    // ASIO protocol callbacks
    void handle_new_socket(const asio::error_code&);
    void handle_client_message(Client*, const asio::error_code&);
    void murder_client(Client*);

    // message functions
    void handle_ping(Client*);
    void handle_client_register(Client*);
    void handle_acct_login(Client*);
    void handle_set_player(Client*);

    asio::local::stream_protocol::socket listen;
    std::set<std::unique_ptr<Client>> clients;
  };
}
