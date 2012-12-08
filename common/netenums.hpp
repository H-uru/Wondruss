#ifndef WONDRUSS_COMMON_ENUMS
#define WONDRUSS_COMMON_ENUMS

namespace Wondruss {
  enum class NetStatus : uint32_t {
    Success,
    InternalError,
    Timeout,
    BadServerData,
    AgeNotFound,
    ConnectFailed,
    Disconnected,
    FileNotFound,
    OldBuildId,
    RemoteShutdown,
    TimeoutOdbc,
    AccountAlreadyExists,
    PlayerAlreadyExists,
    AccountNotFound,
    PlayerNotFound,
    InvalidParameter,
    NameLookupFailed,
    LoggedInElsewhere,
    VaultNodeNotFound,
    MaxPlayersOnAcct,
    AuthenticationFailed,
    StateObjectNotFound,
    LoginDenied,
    CircularReference,
    AccountNotActivated,
    KeyAlreadyUsed,
    KeyNotFound,
    ActivationCodeNotFound,
    PlayerNameInvalid,
    NotSupported,
    ServiceForbidden,
    AuthTokenTooOld,
    MustUseGameTapClient,
    TooManyFailedLogins,
    GameTapConnectionFailed,
    GTTooManyAuthOptions,
    GTMissingParameter,
    GTServerError,
    AccountBanned,
    KickedByCCR,
    ScoreWrongType,
    ScoreNotEnoughPoints,
    ScoreAlreadyExists,
    ScoreNoDataFound,
    InviteNoMatchingPlayer,
    InviteTooManyHoods,
    NeedToPay,
    ServerBusy,
    VaultNodeAccessViolation
  };
};

#endif // WONDRUSS_COMMON_ENUMS
