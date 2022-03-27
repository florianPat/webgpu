#pragma once

#include "INetworkAbstraction.h"
#include <eos_types.h>
#include <eos_auth.h>
#include <eos_p2p.h>
#include <eos_lobby_types.h>
#include "String.h"
#include "NetworkPacket.h"

class NetworkAbstraction : public INetworkAbstraction
{
    static constexpr const char* EOS_HORST_CLIENT_ID = "xyza7891kB0EbQkQx0SfQ2fGP7tNkgbW";
    static constexpr const char* EOS_HORST_CLIENT_NAME = "g10E6uFTUgslEfYoNyTIddF4hZA3KPO5Ahn7gPFe2vE";
    static constexpr const char* EOS_PLAYER_CLIENT_ID = "xyza7891xNE7Xr0efmkxmQzxzjUjQaX4";
    static constexpr const char* EOS_PLAYER_CLIENT_NAME = "fbn4DZmvuN9iFA3ulr8P1Xr4iUkAPLPDr1RgpXiSuu8";
public:
    enum class SearchStatus
    {
        NOT_STARTED,
        PENDING,
        COMPLETE,
        JOINING,
        NOT_FOUND,
    };
    struct EOSStruct
    {
        EOS_HPlatform eosPlatform = nullptr;
        EOS_HAuth eosAuth = nullptr;
        EOS_HP2P eosP2p = nullptr;
        EOS_HConnect eosConnect = nullptr;
        EOS_HLobby eosLobby = nullptr;
        EOS_Auth_Token* eosAuthToken = nullptr;
        String username = "";
        bool portalAuthentication = false;
        EOS_ProductUserId userId = nullptr;
        EOS_P2P_SocketId socketId = {};
        Vector<EOS_ProductUserId> peers;
        LongString lobbyId = "";
        SearchStatus searchStatus = SearchStatus::NOT_STARTED;
        EOS_ContinuanceToken continuanceToken = nullptr;
    };
    struct NetworkBuffer
    {
        uint8_t* bytes = nullptr;
        uint32_t size = 0;

        ~NetworkBuffer();
    };
private:
    EOSStruct eosContainer;
    NetworkBuffer networkBuffer;
    EOS_LobbyId lobbyName = nullptr;
    NetworkPacket& networkPacket;
public:
    NetworkAbstraction(const String& username, const char* lobbyName, NetworkPacket& networkPacket);
    ~NetworkAbstraction();

    void send(uint8_t* buffer, uint32_t payloadSize, bool reliable);
    void receive();
    const NetworkBuffer& getNetworkBuffer() const;
    void disconnectFromAllEndpoints();

    void update();
    void authorizeWithEpic();
    void authorizeLocally();
    void authorizeWithDeveloperTool();
    void createLobby();
    void destroyLobby();
    void leaveLobby();
    EOS_HLobbySearch createLobbySearchHandle();
    SearchStatus searchLobby(EOS_HLobbySearch lobbySearchHandle);
    bool isJoinedLobby();
    uint32_t getLobbyPlayerCount();
    uint32_t getConnectedPeersCount() const;
    static String getUserIdString(EOS_ProductUserId userId);
    static void removePeerFromContainer(const EOS_ProductUserId& peer, Vector<EOS_ProductUserId>& peers);
private:
    void initialize(const char* productId);
    void setLogCallback();
    void initializePlatform(const char* productId);
    void retrieveHandels();
    void setExpireCallback();
    void setConnectChangeCallback();
    void setRequestEventsCallback();
    void setLobbyMemberChangedCallback();
private:
    EOS_ELoginStatus getLoginStatus();
    void establishConnection(const EOS_ProductUserId& peer);
    void closeConnection(const EOS_ProductUserId& peer);
    void joinLobby(EOS_HLobbyDetails lobby);
    EOS_LobbyDetails_Info* getLobbyDetails(const EOS_HLobbyDetails& lobbyDetails);

    void createDeviceLocalUser();
    void deleteDeviceLocalUser();
    void deviceLocalConnectLogin();

    void epicLogin();
    void epicLogout();
    void epicConnectLogin();
    void developerToolLogin();
    void createConnectAccount();
};