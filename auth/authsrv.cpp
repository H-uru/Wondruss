#include "authsrv.hpp"
#include "common/fds.hpp"
#include "common/logger.hpp"
#include "common/netenums.hpp"

// Shamelessly stolen from Dirtsand
enum class CliToAuth : uint16_t {
  PingRequest, ClientRegisterRequest, ClientSetCCRLevel,
  AcctLoginRequest, AcctSetEulaVersion, AcctSetDataRequest,
  AcctSetPlayerRequest, AcctCreateRequest, AcctChangePasswordRequest,
  AcctSetRolesRequest, AcctSetBillingTypeRequest, AcctActivateRequest,
  AcctCreateFromKeyRequest, PlayerDeleteRequest, PlayerUndeleteRequest,
  PlayerSelectRequest, PlayerRenameRequest, PlayerCreateRequest,
  PlayerSetSTatus, PlayerChat, UpgradeVisitorRequest,
  SetPlayerBanStatusRequest, KickPlayer, ChangePlayerNameRequest,
  SendFRiendInviteRequest, VaultNodeCreate, VaultNodeFetch, VaultNodeSave,
  VaultNodeDelete, VaultNodeAdd, VaultNodeRemove, VaultNodeFetchNodeRefs,
  VaultInitAgeRequest, VaultNodeFine, VaultSetSeen, VaultSendNode,
  AgeRequest, FileListRequest, FileDownloadRequest, FileDownloadChunkAck,
  PropagateBuffer, GetPublicAgeList, SetAgePublic, LogPythonTraceback,
  LogStackDump, LogClientDebuggerConnect, ScoreCreate, ScoreDelete,
  ScoreGetScores, ScoreAddPoints, ScoreTransferPoints, ScoreSetPoints,
  ScoreGetRanks, AcctExistsRequest,
};

enum class AuthToCli : uint16_t {
  PingReply, ServerAddr, NotifyNewBuild, ClientRegisterReply,
  AcctLoginReply, AcctData, AcctPlayerInfo, AcctSetPlayerReply,
  AcctCreateReply,AcctChangePasswordReply, AcctSetRolesReply,
  AcctSetBillingTypeReply, AcctActivateReply, AcctCreateFromKeyReply,
  PlayerList, PlayerChat, PlayerCreateReply, PlayerDeleteReply,
  UpgradeVisitorReply, SetPlayerBanStatusReply, ChangePlayerNameReply,
  SendFriendInviteReply, FriendNotify, VaultNodeCreated, VaultNodeFetched,
  VaultNodeChanged, VaultNodeDeleted, VaultNodeAdded, VaultNodeRemoved,
  VaultNodeRefsFetched, VaultInitAgeReply, VaultNodeFindReply,
  VaultSaveNodeReply, VaultAddNodeREply, VaultRemoveNodeReply,
  AgeReply, FileListReply, FileDownloadChunk, PropagateBuffer,
  KickedOff, PublicAgeList, ScoreCreateReply, ScoreDeleteReply,
  ScoreGetScoresReply, ScoreAddPointsReply, ScoreTransferPointsReply,
  ScoreSetPointsReply, ScoreGetRanksReply, AcctExistsReply,
};

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
    LOG_WARN("received an incorrectly sized auth header");
  // TODO: handle this failure
}

Wondruss::AuthSrv::AuthSrv(asio::io_service& io_service)
  : listen(io_service, asio::local::stream_protocol(), FD_SOCKETS), rdsock(io_service, asio::local::stream_protocol(), FD_LBY_TO_SLV), wrsock(io_service, asio::local::stream_protocol(), FD_SLV_TO_LBY)
{
  listen.send(asio::buffer("OK", 2));
  listen.async_receive(asio::null_buffers(), std::bind(std::mem_fn(&AuthSrv::handle_new_socket), this, std::placeholders::_1));
  rdsock.async_receive(asio::null_buffers(), std::bind(std::mem_fn(&AuthSrv::handle_message), this, std::placeholders::_1));
}

void Wondruss::AuthSrv::handle_new_socket(const asio::error_code& error)
{
  if (error) {
    LOG_ERROR("Lost connection to lobby process");
    // TODO: better error handling
  } else {
    LOG_DEBUG("Receiving new socket from lobby");
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
      LOG_ERROR("Could not receive socket message from lobby");
      // TODO: This is probably fatal?
    } else {
      cmsg = CMSG_FIRSTHDR(&msg);
      if(cmsg && cmsg->cmsg_level == SOL_SOCKET && cmsg->cmsg_type == SCM_RIGHTS) {
        int fd = *((int*)CMSG_DATA(cmsg));
        asio::ip::tcp::socket new_socket(listen.get_io_service(), asio::ip::tcp::v6(), fd);
        handle_protocol_cruft(new_socket);

        Client* client = new Client(std::move(new_socket));
        clients.insert(std::unique_ptr<Client>(client));
        LOG_INFO("Successfully transferred ", client->name(), " from lobby.");
        client->async_receive(asio::null_buffers(), std::bind(std::mem_fn(&AuthSrv::handle_client_message), this, client, std::placeholders::_1));
      }
    }
  }
  listen.async_receive(asio::null_buffers(), std::bind(std::mem_fn(&AuthSrv::handle_new_socket), this, std::placeholders::_1));  
}

void Wondruss::AuthSrv::handle_client_message(Client* client, const asio::error_code& error)
{
  if (error) {
    LOG_ERROR("could not receive message from client");
    return;
  }

  try {
    CliToAuth msgid = client->read<CliToAuth>();
    LOG_DEBUG("Got message ", uint16_t(msgid), " from client ", client->name());
    switch(msgid) {
      case CliToAuth::PingRequest:
        handle_ping(client);
        break;
      case CliToAuth::ClientRegisterRequest:
        handle_client_register(client);
        break;
      case CliToAuth::AcctLoginRequest:
        handle_acct_login(client);
        break;
      case CliToAuth::AcctSetPlayerRequest:
        handle_set_player(client);
        break;
      default:
        murder_client(client, true);
        return;
    }

    // Send the buffer we've been accumulating out to the client
    client->flush();

  } catch (asio::system_error e) {
    LOG_ERROR(e.what(), " while reading from socket for ", client->account);
    murder_client(client, false);
    return;
  }

  client->async_receive(asio::null_buffers(), std::bind(std::mem_fn(&AuthSrv::handle_client_message), this, client, std::placeholders::_1));
}

void Wondruss::AuthSrv::send_message(const std::string& msg, std::function<void(std::string)> callback)
{
  uint32_t trans=0;
  trans++;
  uint32_t len;
  len = msg.size();
  wrsock.send(asio::buffer(&trans, 4));
  wrsock.send(asio::buffer(&len, 4));
  wrsock.send(asio::buffer(msg.c_str(), len));
  callbacks[trans] = callback;
}

void Wondruss::AuthSrv::handle_message(const asio::error_code& error)
{
  // TODO: Handle requests as well as responses here
  uint32_t trans;
  uint32_t len;
  char* msg;
  rdsock.receive(asio::buffer(&trans, 4));
  rdsock.receive(asio::buffer(&len, 4));
  msg = new char[len+1];
  msg[len] = 0;
  rdsock.receive(asio::buffer(msg, len));
  callbacks[trans](msg);
  callbacks.erase(trans);
  delete[] msg;
  rdsock.async_receive(asio::null_buffers(), std::bind(std::mem_fn(&AuthSrv::handle_message), this, std::placeholders::_1));
}

void Wondruss::AuthSrv::murder_client(Client* client, bool kick)
{
  LOG_DEBUG("Murdering that asshat, ", client->name());
  auto it = std::find_if(clients.begin(), clients.end(),
    [&client] (const std::unique_ptr<Client>& ptr) {
      return ptr.get() == client;
    }
  );
  if (it == clients.end())
    LOG_ERROR("What that tits?! ", client->name(), " isn't in the set?");
  else {
    if (kick) {
      client->write(AuthToCli::KickedOff);
      client->write<uint32_t>(NetStatus::Disconnected);
    }
    clients.erase(it);
  }
}

void Wondruss::AuthSrv::handle_ping(Client* client)
{
  uint32_t time, trans, payload_size;
  char* payload = 0;
  
  time = client->read<uint32_t>();
  trans = client->read<uint32_t>();
  payload_size = client->read<uint32_t>();
  if (payload_size) {
    payload = new char[payload_size];
    client->receive(asio::buffer(payload, payload_size));
  }

  client->write(AuthToCli::PingReply);
  client->write(time);
  client->write(trans);
  client->write(payload_size);
  if (payload) {
    client->send(asio::buffer(payload, payload_size));
    delete[] payload;
  }

  LOG_DEBUG("Ponging ", client->name());
}

void Wondruss::AuthSrv::handle_client_register(Client* client)
{
  client->server_challenge = rand();
  client->build_id = client->read<uint32_t>();
  client->write(AuthToCli::ClientRegisterReply);
  client->write(client->server_challenge);
  // TODO: error handling

  LOG_INFO(client->name(), " is build # ", client->build_id);
}

void Wondruss::AuthSrv::handle_acct_login(Client* client)
{
  uint32_t trans = client->read<uint32_t>();
  client->client_challenge = client->read<uint32_t>();
  client->account = client->read<std::string>();
  uint32_t pass_hash[5];
  client->receive(asio::buffer(pass_hash, 20));
  std::string token = client->read<std::string>();
  std::string os = client->read<std::string>();

  send_message(client->account, [=] (std::string msg) {
    client->write(AuthToCli::AcctLoginReply);
    client->write(trans);
    client->write<uint32_t>(NetStatus::Success); // net success
    client->send(asio::buffer(client->account_uuid, 16));
    client->write<uint32_t>(0); // flags
    client->write<uint32_t>(0); // billing type
    uint32_t dummy_droid[4];
    client->send(asio::buffer(dummy_droid, 16));
    client->flush();
  });
}

void Wondruss::AuthSrv::handle_set_player(Wondruss::AuthSrv::Client* client)
{
  uint32_t trans = client->read<uint32_t>();
  client->read<uint32_t>(); // player ID
  client->write(AuthToCli::AcctSetPlayerReply);
  client->write(trans);
  client->write<uint32_t>(NetStatus::Success);
}
