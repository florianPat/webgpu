#include "NetworkAbstraction.h"

#pragma comment(lib, "EOSSDK-Win64-Shipping.lib")

#include <eos_common.h>
#include <eos_init.h>
#include <eos_version.h>
#include <eos_logging.h>
#include <eos_lobby.h>
#include <eos_sdk.h>
#include "Utils.h"

void EOS_CALL logMessageFunc(const EOS_LogMessage* message)
{
	if (strlen(message->Message) < 255)
	{
		utils::logF("EOS Log: %s", message->Message);
	}
}

void EOS_CALL loginCallback(const EOS_Auth_LoginCallbackInfo* data)
{
	NetworkAbstraction::EOSStruct* thiz = (NetworkAbstraction::EOSStruct*)data->ClientData;

	switch (data->ResultCode)
	{
		case EOS_EResult::EOS_Auth_AccountLockedForUpdate:
		{
			utils::logBreak("Auth: Account locked by update operation.");
			break;
		}
		case EOS_EResult::EOS_Auth_InvalidToken:
		{
			utils::logBreak("Auth: Invalid access token, typically when switching between backend environments");
			break;
		}
		case EOS_EResult::EOS_Auth_AuthenticationFailure:
		{
			utils::logBreak("Auth: Invalid bearer token");
			break;
		}
		case EOS_EResult::EOS_Auth_InvalidPlatformToken:
		{
			utils::logBreak("Invalid platform token");
			break;
		}
		case EOS_EResult::EOS_Auth_WrongAccount:
		{
			utils::logBreak("Auth: Auth parameters are not associated with this account");
			break;
		}
		case EOS_EResult::EOS_Auth_WrongClient:
		{
			utils::logBreak("Auth: Auth parameters are not associated with this client");
			break;
		}
		case EOS_EResult::EOS_Auth_ScopeConsentRequired:
		{
			utils::logBreak("Auth: Consent has not been given by the user");
			break;
		}
		case EOS_EResult::EOS_Auth_ApplicationNotFound:
		{
			utils::logBreak("Auth: The application has no profile on the backend");
			break;
		}
		case EOS_EResult::EOS_Auth_ScopeNotFound:
		{
			utils::logBreak("Auth: The requested consent wasn't found on the backend");
			break;
		}
		case EOS_EResult::EOS_Auth_AccountFeatureRestricted:
		{
			utils::logBreak("Auth: This account has been denied access to login");
			break;
		}
		case EOS_EResult::EOS_Auth_PersistentAuth_AccountNotActive:
		{
			utils::logBreak("Auth: The account has been disabled and cannot be used for authentication");
			break;
		}
		case EOS_EResult::EOS_Auth_ParentalControls:
		{
			utils::logBreak("Auth: Parental locks are in place");
			break;
		}
		case EOS_EResult::EOS_Auth_NoRealId:
		{
			utils::logBreak("Auth: Korea real ID association required but missing");
			break;
		}
		case EOS_EResult::EOS_Auth_ExchangeCodeNotFound:
		{
			utils::logBreak("Auth: Exchange code not found");
			break;
		}
		case EOS_EResult::EOS_Auth_OriginatingExchangeCodeSessionExpired:
		{
			utils::logBreak("Auth: Originating exchange code session has expired");
			break;
		}
		case EOS_EResult::EOS_Auth_FullAccountRequired:
		{
			utils::logBreak("Auth: Full account is required");
			break;
		}
		case EOS_EResult::EOS_Auth_HeadlessAccountRequired:
		{
			utils::logBreak("Auth: Headless account is required");
			break;
		}
		case EOS_EResult::EOS_Auth_PasswordCannotBeReused:
		{
			utils::logBreak("Auth: Password was previously used and cannot be reused");
			break;
		}

		case EOS_EResult::EOS_Auth_MFARequired:
		{
			utils::log("Auth: MFA challenge required");
			break;
		}
		case EOS_EResult::EOS_Auth_InvalidRefreshToken:
		{
			utils::log("Auth: Refresh token used was invalid");
			break;
		}
		case EOS_EResult::EOS_Auth_Expired:
		{
			utils::log("Auth: Authorization code/exchange code has expired");
			break;
		}
		
		case EOS_EResult::EOS_Auth_PinGrantCode:
		{
			utils::log("Auth: Pin grant code initiated");
			break;
		}
		case EOS_EResult::EOS_Auth_PinGrantPending:
		{
			utils::log("Auth: Pin grant code attempt pending");
			break;
		}
		case EOS_EResult::EOS_Auth_PinGrantExpired:
		{
			utils::log("Auth: Pin grant code attempt expired");
			break;
		}
		case EOS_EResult::EOS_Success:
		{
			utils::log("Auth: Logged In");
			EOS_Auth_CopyUserAuthTokenOptions copyUserAuthTokenOptions = {};
			copyUserAuthTokenOptions.ApiVersion = EOS_AUTH_COPYUSERAUTHTOKEN_API_LATEST;
			EOS_Auth_Token* authToken = nullptr;
			EOS_Auth_CopyUserAuthToken(thiz->eosAuth, &copyUserAuthTokenOptions, data->LocalUserId, &authToken);
			thiz->eosAuthToken = authToken;
			break;
		}
		case EOS_EResult::EOS_InvalidAuth:
		{
			thiz->portalAuthentication = true;
			break;
		}
		default:
		{
			utils::logBreak("Auth: Other result code");
		}
	}

	thiz->searchStatus = NetworkAbstraction::SearchStatus::NOT_STARTED;
}

void EOS_CALL logoutCallback(const EOS_Auth_LogoutCallbackInfo* data)
{
	if (data->ResultCode != EOS_EResult::EOS_Success)
	{
		utils::logBreak("Could not log out from the epic account");
	}
}

void EOS_CALL deletePersistenceAuthTokenCallback(const EOS_Auth_DeletePersistentAuthCallbackInfo* data)
{
	if (data->ResultCode != EOS_EResult::EOS_Success)
	{
		utils::logBreak("Could not delete the persisted auth token");
	}
}

void EOS_CALL deviceCreateCallback(const EOS_Connect_CreateDeviceIdCallbackInfo* data)
{
	NetworkAbstraction::EOSStruct* thiz = (NetworkAbstraction::EOSStruct*)data->ClientData;

	switch (data->ResultCode)
	{
		case EOS_EResult::EOS_Success:
		{
			utils::log("Successfully create local account");
			thiz->searchStatus = NetworkAbstraction::SearchStatus::COMPLETE;
			break;
		}
		case EOS_EResult::EOS_DuplicateNotAllowed:
		{
			utils::log("Already created user!");
			thiz->searchStatus = NetworkAbstraction::SearchStatus::COMPLETE;
			break;
		}
		default:
		{
			utils::logFBreak("EOS Error: %d", data->ResultCode);
			break;
		}
	}
}

void EOS_CALL deviceDeleteCallback(const EOS_Connect_DeleteDeviceIdCallbackInfo* data)
{
	NetworkAbstraction::EOSStruct* thiz = (NetworkAbstraction::EOSStruct*)data->ClientData;

	switch (data->ResultCode)
	{
		case EOS_EResult::EOS_Success:
		{
			thiz->username = "";
			utils::log("Successfully deleted local user");
			break;
		}
		default:
		{
			utils::logFBreak("EOS Error: %d", data->ResultCode);
			break;
		}
	}
}

void EOS_CALL connectLoginCallback(const EOS_Connect_LoginCallbackInfo* data)
{
	NetworkAbstraction::EOSStruct* thiz = (NetworkAbstraction::EOSStruct*)data->ClientData;

	switch (data->ResultCode)
	{
		case EOS_EResult::EOS_InvalidUser:
		{
			utils::log("Connect login: Invalid user, BUT giving continuanceToken");
			thiz->continuanceToken = data->ContinuanceToken;
			break;
		}
		case EOS_EResult::EOS_Success:
		{
			utils::log("Success logging in");
			thiz->userId = data->LocalUserId;
			break;
		}
		default:
		{
			utils::logFBreak("EOS Error: %d", data->ResultCode);
			break;
		}
	}
}

void EOS_CALL connectRegisterCallback(const EOS_Connect_CreateUserCallbackInfo* data)
{
	NetworkAbstraction::EOSStruct* thiz = (NetworkAbstraction::EOSStruct*)data->ClientData;

	switch (data->ResultCode)
	{
		case EOS_EResult::EOS_Success:
		{
			utils::log("Success registering user");
			thiz->userId = data->LocalUserId;
			break;
		}
		default:
		{
			utils::logFBreak("EOS Error: %d", data->ResultCode);
			break;
		}
	}
}

void EOS_CALL expireWarning(const EOS_Connect_AuthExpirationCallbackInfo* data)
{
	utils::logBreak("Account expire soon!");
}

void EOS_CALL loginChange(const EOS_Connect_LoginStatusChangedCallbackInfo* data)
{
	utils::log("Login status changed!");
}

void EOS_CALL connectionRequest(const EOS_P2P_OnIncomingConnectionRequestInfo* data)
{
	NetworkAbstraction::EOSStruct* thiz = (NetworkAbstraction::EOSStruct*)data->ClientData;

	EOS_P2P_AcceptConnectionOptions acceptConnectionOptions = {};
	acceptConnectionOptions.ApiVersion = EOS_P2P_ACCEPTCONNECTION_API_LATEST;
	acceptConnectionOptions.LocalUserId = thiz->userId;
	acceptConnectionOptions.SocketId = &thiz->socketId;
	acceptConnectionOptions.RemoteUserId = data->RemoteUserId;
	EOS_EResult result = EOS_P2P_AcceptConnection(thiz->eosP2p, &acceptConnectionOptions);
	assert(result == EOS_EResult::EOS_Success);

	utils::log("Accept new connection");
	thiz->peers.push_back(data->RemoteUserId);
}

void EOS_CALL connectionClosed(const EOS_P2P_OnRemoteConnectionClosedInfo* data)
{
	NetworkAbstraction::EOSStruct* thiz = (NetworkAbstraction::EOSStruct*)data->ClientData;

	NetworkAbstraction::removePeerFromContainer(data->RemoteUserId, thiz->peers);

	EOS_P2P_CloseConnectionOptions closeConnectionOptions = {};
	closeConnectionOptions.ApiVersion = EOS_P2P_CLOSECONNECTION_API_LATEST;
	closeConnectionOptions.LocalUserId = data->LocalUserId;
	closeConnectionOptions.RemoteUserId = data->RemoteUserId;
	closeConnectionOptions.SocketId = data->SocketId;
	EOS_EResult result = EOS_P2P_CloseConnection(thiz->eosP2p, &closeConnectionOptions);
	assert(result == EOS_EResult::EOS_Success);
}

void EOS_CALL lobbyCreateCallback(const EOS_Lobby_CreateLobbyCallbackInfo* data)
{
	NetworkAbstraction::EOSStruct* thiz = (NetworkAbstraction::EOSStruct*)data->ClientData;

	switch (data->ResultCode)
	{
		case EOS_EResult::EOS_Success:
		{
			thiz->lobbyId = String::createIneffectivlyFrom(data->LobbyId);
			utils::logF("Created lobby: %s", thiz->lobbyId.c_str());
			break;
		}
		case EOS_EResult::EOS_Lobby_LobbyAlreadyExists:
		{
			utils::logFBreak("Lobby already exists: %s", data->LobbyId);
			break;
		}
		default:
		{
			utils::logFBreak("EOS Error: %d", data->ResultCode);
			break;
		}
	}

	thiz->searchStatus = NetworkAbstraction::SearchStatus::NOT_STARTED;
}

void EOS_CALL lobbyDestroyCallback(const EOS_Lobby_DestroyLobbyCallbackInfo* data)
{
	NetworkAbstraction::EOSStruct* thiz = (NetworkAbstraction::EOSStruct*)data->ClientData;

	switch (data->ResultCode)
	{
		case EOS_EResult::EOS_Success:
		{
			utils::log("Destroyed lobby");
			thiz->lobbyId = "";
			break;
		}
		default:
		{
			utils::logFBreak("EOS Error: %d", data->ResultCode);
			break;
		}
	}

	thiz->searchStatus = NetworkAbstraction::SearchStatus::NOT_STARTED;
}

void EOS_CALL lobbyJoinCallback(const EOS_Lobby_JoinLobbyCallbackInfo* data)
{
	NetworkAbstraction::EOSStruct* thiz = (NetworkAbstraction::EOSStruct*)data->ClientData;
	
	switch (data->ResultCode)
	{
		case EOS_EResult::EOS_Success:
		{
			utils::log("Joined lobby");
			thiz->lobbyId = String::createIneffectivlyFrom(data->LobbyId);
			break;
		}
		default:
		{
			utils::logFBreak("EOS Error: %d", data->ResultCode);
			break;
		}
	}

	thiz->searchStatus = NetworkAbstraction::SearchStatus::NOT_STARTED;
}

void EOS_CALL lobbyLeaveCallback(const EOS_Lobby_LeaveLobbyCallbackInfo* data)
{
	NetworkAbstraction::EOSStruct* thiz = (NetworkAbstraction::EOSStruct*)data->ClientData;

	switch (data->ResultCode)
	{
		case EOS_EResult::EOS_Success:
		{
			assert(thiz->lobbyId == data->LobbyId);
			utils::log("Left lobby");
			thiz->lobbyId = "";
			
			thiz->peers.clear();
			
			EOS_P2P_CloseConnectionsOptions closeConnectionsOptions = {};
			closeConnectionsOptions.ApiVersion = EOS_P2P_CLOSECONNECTIONS_API_LATEST;
			closeConnectionsOptions.LocalUserId = thiz->userId;
			closeConnectionsOptions.SocketId = &thiz->socketId;
			EOS_EResult result = EOS_P2P_CloseConnections(thiz->eosP2p, &closeConnectionsOptions);
			assert(result == EOS_EResult::EOS_Success);
			break;
		}
		default:
		{
			utils::logFBreak("EOS Error: %d", data->ResultCode);
			break;
		}
	}
	
	thiz->searchStatus = NetworkAbstraction::SearchStatus::NOT_STARTED;
}

void EOS_CALL lobbyMemberChangedCallback(const EOS_Lobby_LobbyMemberStatusReceivedCallbackInfo* data)
{
	NetworkAbstraction::EOSStruct* thiz = (NetworkAbstraction::EOSStruct*)data->ClientData;

	assert(data->LobbyId == thiz->lobbyId);

	switch (data->CurrentStatus)
	{
		case EOS_ELobbyMemberStatus::EOS_LMS_JOINED:
		{
			utils::log("User joined the lobby");
			EOS_P2P_AcceptConnectionOptions acceptConnectionOptions = {};
			acceptConnectionOptions.ApiVersion = EOS_P2P_ACCEPTCONNECTION_API_LATEST;
			acceptConnectionOptions.LocalUserId = thiz->userId;
			acceptConnectionOptions.SocketId = &thiz->socketId;
			acceptConnectionOptions.RemoteUserId = data->TargetUserId;
			EOS_EResult result = EOS_P2P_AcceptConnection(thiz->eosP2p, &acceptConnectionOptions);
			assert(result == EOS_EResult::EOS_Success);
			thiz->peers.push_back(data->TargetUserId);
			break;
		}
		case EOS_ELobbyMemberStatus::EOS_LMS_DISCONNECTED:
		case EOS_ELobbyMemberStatus::EOS_LMS_KICKED:
		case EOS_ELobbyMemberStatus::EOS_LMS_LEFT:
		{
			utils::log("User left the lobby!");
			break;
		}
		case EOS_ELobbyMemberStatus::EOS_LMS_PROMOTED:
		{
			utils::log("New Horst!");
			break;
		}
		case EOS_ELobbyMemberStatus::EOS_LMS_CLOSED:
		{
			utils::log("Lobby closed");
			thiz->lobbyId = "";
			break;
		}
	}
}

void EOS_CALL lobbyFindCallback(const EOS_LobbySearch_FindCallbackInfo* data)
{
	NetworkAbstraction::EOSStruct* thiz = (NetworkAbstraction::EOSStruct*)data->ClientData;

	switch (data->ResultCode)
	{
		case EOS_EResult::EOS_Success:
		{
			utils::log("Found lobby");
			thiz->searchStatus = NetworkAbstraction::SearchStatus::COMPLETE;
			break;
		}
		case EOS_EResult::EOS_NotFound:
		{
			utils::logBreak("Could not find lobby. Please close and retry!");
			thiz->searchStatus = NetworkAbstraction::SearchStatus::NOT_FOUND;
			break;
		}
		default:
		{
			utils::logFBreak("EOS Error: %d", data->ResultCode);
			break;
		}
	}
}

NetworkAbstraction::NetworkAbstraction(const String& username, const char* lobbyNameIn, NetworkPacket& networkPacketIn) : lobbyName(lobbyNameIn), networkPacket(networkPacketIn)
{
#if 1
	new (&eosContainer.username) String(username);
	networkBuffer.bytes = (uint8_t*)malloc(EOS_P2P_MAX_PACKET_SIZE);
	eosContainer.socketId.ApiVersion = EOS_P2P_SOCKETID_API_LATEST;
	String socketName = "socketName";
	assert(socketName.size() <= 32);
	for (uint32_t i = 0; i < socketName.size(); ++i)
	{
		eosContainer.socketId.SocketName[i] = socketName[i];
	}
	eosContainer.socketId.SocketName[socketName.size()] = '\0';

	const char* productId = "7ee3db2d27e84b5a9d35bfcde52b2463";
	initialize(productId);
	setLogCallback();
	initializePlatform(productId);
	retrieveHandels();
	setExpireCallback();
	
	authorizeLocally();
	// eosContainer.portalAuthentication = true;
	// authorizeWithEpic();

	setConnectChangeCallback();
	setRequestEventsCallback();
	setLobbyMemberChangedCallback();

	if (lobbyName[0] != '\0')
	{
		EOS_HLobbySearch lobbySearchHandle = createLobbySearchHandle();

		while (!isJoinedLobby())
		{
			searchLobby(lobbySearchHandle);
			update();
		}
	}
	else
	{
		createLobby();
		while (!isJoinedLobby())
		{
			update();
		}
	}
#endif
}

NetworkAbstraction::~NetworkAbstraction()
{
	if (eosContainer.eosPlatform != nullptr)
	{
		// NOTE: Could also remove the notfier callback references here. But do I have to?

		if (eosContainer.userId != nullptr)
		{
			disconnectFromAllEndpoints();

			if (eosContainer.lobbyId != "")
			{
				if (getLobbyPlayerCount() == 1)
				{
					destroyLobby();
					while (eosContainer.lobbyId != "")
					{
						update();
					}
				}
				else
				{
					leaveLobby();
					while (eosContainer.lobbyId != "")
					{
						update();
					}
				}
			}
		}

		if (eosContainer.eosAuthToken != nullptr)
		{
			EOS_Auth_Token_Release(eosContainer.eosAuthToken);
			eosContainer.eosAuthToken = nullptr;
		}

		EOS_Platform_Release(eosContainer.eosPlatform);
		EOS_EResult result = EOS_Shutdown();
		if (result != EOS_EResult::EOS_Success)
		{
			utils::logBreak("Could not deinitialize EOS!");
		}
	}

	eosContainer.~EOSStruct();
	networkBuffer.~NetworkBuffer();
}

void NetworkAbstraction::send(uint8_t* buffer, uint32_t payloadSize, bool reliable)
{
	assert(payloadSize < EOS_P2P_MAX_PACKET_SIZE);

	EOS_P2P_SendPacketOptions sendPacketOptions = {};
	sendPacketOptions.ApiVersion = EOS_P2P_SENDPACKET_API_LATEST;
	sendPacketOptions.LocalUserId = eosContainer.userId;
	sendPacketOptions.SocketId = &eosContainer.socketId;
	sendPacketOptions.Channel = 0;
	sendPacketOptions.DataLengthBytes = payloadSize;
	sendPacketOptions.Data = buffer;
	sendPacketOptions.bAllowDelayedDelivery = false;

	EOS_EResult result;
	for (auto it = eosContainer.peers.begin(); it != eosContainer.peers.end(); ++it)
	{
		sendPacketOptions.RemoteUserId = *it;
		EOS_Bool isUserValid = EOS_ProductUserId_IsValid(*it);
		assert(isUserValid == EOS_TRUE);

		result = EOS_P2P_SendPacket(eosContainer.eosP2p, &sendPacketOptions);
		assert(result == EOS_EResult::EOS_Success);
	}
}

void NetworkAbstraction::receive()
{
	EOS_P2P_ReceivePacketOptions receivePacketOptions = {};
	receivePacketOptions.ApiVersion = EOS_P2P_RECEIVEPACKET_API_LATEST;
	receivePacketOptions.LocalUserId = eosContainer.userId;
	receivePacketOptions.MaxDataSizeBytes = EOS_P2P_MAX_PACKET_SIZE;
	receivePacketOptions.RequestedChannel = 0;

	EOS_ProductUserId sendingUser;
	EOS_P2P_SocketId sendingSocket;
	uint8_t sendingChannel;
	EOS_EResult result = EOS_P2P_ReceivePacket(eosContainer.eosP2p, &receivePacketOptions, &sendingUser, &sendingSocket, &sendingChannel, networkBuffer.bytes, &networkBuffer.size);
	if (result == EOS_EResult::EOS_NotFound)
	{
		networkBuffer.size = 0;
	}

	for (uint32_t i = 0; i < eosContainer.peers.size(); ++i)
	{
		if (eosContainer.peers[i] == sendingUser)
		{
			if (networkBuffer.size != sizeof(NetworkPacket))
			{
				continue;
			}
			assert(networkBuffer.size == sizeof(NetworkPacket));
			auto networkPacket = (ReceivePacket*)networkBuffer.bytes;
			assert(i < 256);
			networkPacket->playerId = (uint8_t)i;
			break;
		}
	}
}

const NetworkAbstraction::NetworkBuffer& NetworkAbstraction::getNetworkBuffer() const
{
	return networkBuffer;
}

void NetworkAbstraction::disconnectFromAllEndpoints()
{
	assert(eosContainer.userId != nullptr);

	EOS_P2P_CloseConnectionsOptions closeConnectionsOptions = {};
	closeConnectionsOptions.ApiVersion = EOS_P2P_CLOSECONNECTIONS_API_LATEST;
	closeConnectionsOptions.LocalUserId = eosContainer.userId;
	closeConnectionsOptions.SocketId = &eosContainer.socketId;
	EOS_EResult result = EOS_P2P_CloseConnections(eosContainer.eosP2p, &closeConnectionsOptions);
	assert(result == EOS_EResult::EOS_Success);
}

void NetworkAbstraction::update()
{
	EOS_Platform_Tick(eosContainer.eosPlatform);
}

void NetworkAbstraction::authorizeWithEpic()
{
	if (getLoginStatus() == EOS_ELoginStatus::EOS_LS_NotLoggedIn)
	{
		if (eosContainer.searchStatus == SearchStatus::NOT_STARTED)
		{
			epicLogin();
		}

		while (eosContainer.eosAuthToken == nullptr)
		{
			update();
		}
	}
	eosContainer.searchStatus = SearchStatus::NOT_STARTED;
	while (getLoginStatus() != EOS_ELoginStatus::EOS_LS_LoggedIn)
	{
		if (eosContainer.eosAuthToken != nullptr && eosContainer.searchStatus != SearchStatus::PENDING)
		{
			epicConnectLogin();
		}
		else if (eosContainer.continuanceToken != nullptr)
		{
			createConnectAccount();
		}
		update();
	}
	eosContainer.searchStatus = SearchStatus::NOT_STARTED;
}

void NetworkAbstraction::authorizeLocally()
{
	while (getLoginStatus() == EOS_ELoginStatus::EOS_LS_NotLoggedIn)
	{
		if (eosContainer.searchStatus == SearchStatus::NOT_STARTED)
		{
			createDeviceLocalUser();
		}
		update();
		if (eosContainer.searchStatus == SearchStatus::COMPLETE)
		{
			deviceLocalConnectLogin();
			eosContainer.searchStatus = SearchStatus::JOINING;
		}
	}
	assert(getLoginStatus() == EOS_ELoginStatus::EOS_LS_LoggedIn);
	eosContainer.searchStatus = SearchStatus::NOT_STARTED;
}

void NetworkAbstraction::authorizeWithDeveloperTool()
{
	while (getLoginStatus() == EOS_ELoginStatus::EOS_LS_NotLoggedIn)
	{
		update();
		if (eosContainer.searchStatus == SearchStatus::NOT_STARTED)
		{
			developerToolLogin();
		}
	}
	assert(getLoginStatus() == EOS_ELoginStatus::EOS_LS_UsingLocalProfile);
	while (eosContainer.userId == nullptr)
	{
		update();
		if (eosContainer.eosAuthToken != nullptr && eosContainer.searchStatus != SearchStatus::PENDING)
		{
			epicConnectLogin();
		}
		else if (eosContainer.continuanceToken != nullptr)
		{
			createConnectAccount();
		}
	}
	assert(getLoginStatus() == EOS_ELoginStatus::EOS_LS_LoggedIn);
	eosContainer.searchStatus = SearchStatus::NOT_STARTED;
}

void NetworkAbstraction::epicLogin()
{
	EOS_Auth_LoginOptions loginOptions = {};
	loginOptions.ApiVersion = EOS_AUTH_LOGIN_API_LATEST;
	loginOptions.ScopeFlags = EOS_EAuthScopeFlags::EOS_AS_BasicProfile;
	EOS_Auth_Credentials loginCredentails = {};
	loginCredentails.ApiVersion = EOS_AUTH_CREDENTIALS_API_LATEST;
	loginCredentails.Id = nullptr;
	loginCredentails.Token = nullptr;
	loginCredentails.Type = eosContainer.portalAuthentication ? EOS_ELoginCredentialType::EOS_LCT_AccountPortal : EOS_ELoginCredentialType::EOS_LCT_PersistentAuth;
	loginCredentails.SystemAuthCredentialsOptions = nullptr;
	loginOptions.Credentials = &loginCredentails;
	EOS_Auth_Login(eosContainer.eosAuth, &loginOptions, &eosContainer, &loginCallback);

	eosContainer.portalAuthentication = false;
	eosContainer.searchStatus = SearchStatus::PENDING;
}

void NetworkAbstraction::epicLogout()
{
	if (eosContainer.eosAuthToken)
	{
		EOS_Auth_LogoutOptions logoutOptions = {};
		logoutOptions.ApiVersion = EOS_AUTH_LOGOUT_API_LATEST;
		logoutOptions.LocalUserId = eosContainer.eosAuthToken->AccountId;
		EOS_Auth_Logout(eosContainer.eosAuth, &logoutOptions, nullptr, &logoutCallback);
	}

	EOS_Auth_DeletePersistentAuthOptions deletePersistenceOptions = {};
	deletePersistenceOptions.ApiVersion = EOS_AUTH_DELETEPERSISTENTAUTH_API_LATEST;
	deletePersistenceOptions.RefreshToken = eosContainer.eosAuthToken->RefreshToken;
	EOS_Auth_DeletePersistentAuth(eosContainer.eosAuth, &deletePersistenceOptions, nullptr, &deletePersistenceAuthTokenCallback);
}

void NetworkAbstraction::epicConnectLogin()
{
	EOS_Connect_LoginOptions loginOptions = {};
	loginOptions.ApiVersion = EOS_CONNECT_LOGIN_API_LATEST;
	EOS_Connect_Credentials credentials = {};
	credentials.ApiVersion = EOS_CONNECT_CREDENTIALS_API_LATEST;
	credentials.Token = eosContainer.eosAuthToken->AccessToken;
	credentials.Type = EOS_EExternalCredentialType::EOS_ECT_EPIC;
	loginOptions.Credentials = &credentials;
	loginOptions.UserLoginInfo = nullptr;
	EOS_Connect_Login(eosContainer.eosConnect, &loginOptions, &eosContainer, &connectLoginCallback);

	eosContainer.searchStatus = SearchStatus::PENDING;
}

void NetworkAbstraction::developerToolLogin()
{
	EOS_Auth_LoginOptions loginOptions = {};
	loginOptions.ApiVersion = EOS_AUTH_LOGIN_API_LATEST;
	EOS_Auth_Credentials loginCredentails = {};
	loginCredentails.ApiVersion = EOS_AUTH_CREDENTIALS_API_LATEST;
	loginCredentails.Id = "localhost:7560";
	loginCredentails.Token = "2cd4e641baf643cdad526e5505e782fd";
	loginCredentails.Type = EOS_ELoginCredentialType::EOS_LCT_Developer;
	loginOptions.Credentials = &loginCredentails;
	EOS_Auth_Login(eosContainer.eosAuth, &loginOptions, &eosContainer, &loginCallback);

	eosContainer.searchStatus = SearchStatus::PENDING;
}

void NetworkAbstraction::createConnectAccount()
{
	assert(eosContainer.continuanceToken != nullptr);

	EOS_Connect_CreateUserOptions createUserOptions = {};
	createUserOptions.ApiVersion = EOS_CONNECT_CREATEUSER_API_LATEST;
	createUserOptions.ContinuanceToken = eosContainer.continuanceToken;
	EOS_Connect_CreateUser(eosContainer.eosConnect, &createUserOptions, &eosContainer, &connectRegisterCallback);
	eosContainer.continuanceToken = nullptr;
}

void NetworkAbstraction::createDeviceLocalUser()
{
	EOS_Connect_CreateDeviceIdOptions deviceOptions = {};
	deviceOptions.ApiVersion = EOS_CONNECT_CREATEDEVICEID_API_LATEST;
	deviceOptions.DeviceModel = eosContainer.username.c_str();
	EOS_Connect_CreateDeviceId(eosContainer.eosConnect, &deviceOptions, &eosContainer, &deviceCreateCallback);
	eosContainer.searchStatus = SearchStatus::PENDING;
}

void NetworkAbstraction::deleteDeviceLocalUser()
{
	EOS_Connect_DeleteDeviceIdOptions deleteDeviceId = {};
	deleteDeviceId.ApiVersion = EOS_CONNECT_DELETEDEVICEID_API_LATEST;
	EOS_Connect_DeleteDeviceId(eosContainer.eosConnect, &deleteDeviceId, &eosContainer, &deviceDeleteCallback);
}

void NetworkAbstraction::deviceLocalConnectLogin()
{
	EOS_Connect_LoginOptions loginOptions = {};
	loginOptions.ApiVersion = EOS_CONNECT_LOGIN_API_LATEST;
	EOS_Connect_Credentials credentials = {};
	credentials.ApiVersion = EOS_CONNECT_CREDENTIALS_API_LATEST;
	credentials.Token = nullptr;
	credentials.Type = EOS_EExternalCredentialType::EOS_ECT_DEVICEID_ACCESS_TOKEN;
	loginOptions.Credentials = &credentials;
	EOS_Connect_UserLoginInfo userLoginInfo = {};
	userLoginInfo.ApiVersion = EOS_CONNECT_USERLOGININFO_API_LATEST;
	userLoginInfo.DisplayName = eosContainer.username.c_str();
	loginOptions.UserLoginInfo = &userLoginInfo;
	EOS_Connect_Login(eosContainer.eosConnect, &loginOptions, &eosContainer, &connectLoginCallback);
	
	eosContainer.searchStatus = SearchStatus::PENDING;
}

void NetworkAbstraction::createLobby()
{
	assert(eosContainer.userId != nullptr);

	if (eosContainer.searchStatus == SearchStatus::PENDING)
	{
		return;
	}

	EOS_Lobby_CreateLobbyOptions createLobbyOptions = {};
	createLobbyOptions.ApiVersion = EOS_LOBBY_CREATELOBBY_API_LATEST;
	createLobbyOptions.LocalUserId = eosContainer.userId;
	createLobbyOptions.MaxLobbyMembers = 10;
	createLobbyOptions.PermissionLevel = EOS_ELobbyPermissionLevel::EOS_LPL_PUBLICADVERTISED;
	createLobbyOptions.bPresenceEnabled = false;
	EOS_Lobby_CreateLobby(eosContainer.eosLobby, &createLobbyOptions, &eosContainer, &lobbyCreateCallback);
	eosContainer.searchStatus = SearchStatus::PENDING;
}

void NetworkAbstraction::destroyLobby()
{
	assert(eosContainer.lobbyId != "");

	if (eosContainer.searchStatus == SearchStatus::PENDING)
	{
		return;
	}

	EOS_Lobby_DestroyLobbyOptions destroyLobbyOptions = {};
	destroyLobbyOptions.ApiVersion = EOS_LOBBY_DESTROYLOBBY_API_LATEST;
	destroyLobbyOptions.LocalUserId = eosContainer.userId;
	destroyLobbyOptions.LobbyId = eosContainer.lobbyId.c_str();
	EOS_Lobby_DestroyLobby(eosContainer.eosLobby, &destroyLobbyOptions, &eosContainer, &lobbyDestroyCallback);

	eosContainer.searchStatus = SearchStatus::PENDING;
}

void NetworkAbstraction::initialize(const char* productId)
{
	EOS_InitializeOptions initOptions = {};
	initOptions.ApiVersion = EOS_INITIALIZE_API_LATEST;
	initOptions.ProductName = productId;
	initOptions.ProductVersion = "0.0.1";

	EOS_EResult result = EOS_Initialize(&initOptions);
	if (result != EOS_EResult::EOS_Success)
	{
		utils::logBreak("Could not initialize EOS!");
	}
}

void NetworkAbstraction::setLogCallback()
{
	EOS_EResult result = EOS_Logging_SetCallback(&logMessageFunc);
	if (result != EOS_EResult::EOS_Success)
	{
		utils::logBreak("Could not initialize the EOS callback!");
	}
}

void NetworkAbstraction::initializePlatform(const char* productId)
{
	EOS_Platform_Options platformOptions = {};
	platformOptions.ApiVersion = EOS_PLATFORM_OPTIONS_API_LATEST;
	platformOptions.Flags = EOS_PF_DISABLE_OVERLAY;
	platformOptions.ProductId = productId;
	platformOptions.SandboxId = "663c498246c94e2ea727ce7154677f7c";
	platformOptions.DeploymentId = "916cf6b4ce4941a69d197e895d8ec53a";
	EOS_Platform_ClientCredentials clientCredentials = {};
	clientCredentials.ClientId = EOS_HORST_CLIENT_ID;
	clientCredentials.ClientSecret = EOS_HORST_CLIENT_NAME;
	platformOptions.ClientCredentials = clientCredentials;
	platformOptions.bIsServer = EOS_FALSE;
	platformOptions.EncryptionKey = nullptr;
	platformOptions.CacheDirectory = nullptr;
	platformOptions.TickBudgetInMilliseconds = 0;
	eosContainer.eosPlatform = EOS_Platform_Create(&platformOptions);
	if (eosContainer.eosPlatform == nullptr)
	{
		utils::logBreak("Could not initialize the EOS platform!");
	}
}

void NetworkAbstraction::retrieveHandels()
{
	eosContainer.eosAuth = EOS_Platform_GetAuthInterface(eosContainer.eosPlatform);
	if (eosContainer.eosAuth == nullptr)
	{
		utils::logBreak("Could not retrieve the EOS auth handle!");
	}

	eosContainer.eosConnect = EOS_Platform_GetConnectInterface(eosContainer.eosPlatform);
	if (eosContainer.eosConnect == nullptr)
	{
		utils::logBreak("Could not retrieve the EOS connect handle!");
	}

	eosContainer.eosP2p = EOS_Platform_GetP2PInterface(eosContainer.eosPlatform);
	if (eosContainer.eosP2p == nullptr)
	{
		utils::logBreak("Could not retrieve the EOS P2P handle!");
	}

	eosContainer.eosLobby = EOS_Platform_GetLobbyInterface(eosContainer.eosPlatform);
	if (eosContainer.eosLobby == nullptr)
	{
		utils::logBreak("Could not retrieve the EOS lobby handle!");
	}
}

void NetworkAbstraction::setExpireCallback()
{
	EOS_Connect_AddNotifyAuthExpirationOptions expireCallbackOptions = {};
	expireCallbackOptions.ApiVersion = EOS_CONNECT_ADDNOTIFYAUTHEXPIRATION_API_LATEST;
	EOS_Connect_AddNotifyAuthExpiration(eosContainer.eosConnect, &expireCallbackOptions, nullptr, &expireWarning);
}

void NetworkAbstraction::setConnectChangeCallback()
{
	EOS_Connect_AddNotifyLoginStatusChangedOptions loginStatusChangedOptions = {};
	loginStatusChangedOptions.ApiVersion = EOS_CONNECT_ADDNOTIFYLOGINSTATUSCHANGED_API_LATEST;
	EOS_Connect_AddNotifyLoginStatusChanged(eosContainer.eosConnect, &loginStatusChangedOptions, nullptr, &loginChange);
}

void NetworkAbstraction::setRequestEventsCallback()
{
	EOS_P2P_AddNotifyPeerConnectionRequestOptions eventConnectionRequestOptions = {};
	eventConnectionRequestOptions.ApiVersion = EOS_P2P_ADDNOTIFYPEERCONNECTIONREQUEST_API_LATEST;
	eventConnectionRequestOptions.LocalUserId = eosContainer.userId;
	eventConnectionRequestOptions.SocketId = &eosContainer.socketId;
	EOS_P2P_AddNotifyPeerConnectionRequest(eosContainer.eosP2p, &eventConnectionRequestOptions, &eosContainer, &connectionRequest);

	EOS_P2P_AddNotifyPeerConnectionClosedOptions eventConnectionClosedOptions = {};
	eventConnectionClosedOptions.ApiVersion = EOS_P2P_ADDNOTIFYPEERCONNECTIONCLOSED_API_LATEST;
	eventConnectionClosedOptions.LocalUserId = eosContainer.userId;
	eventConnectionClosedOptions.SocketId = &eosContainer.socketId;
	EOS_P2P_AddNotifyPeerConnectionClosed(eosContainer.eosP2p, &eventConnectionClosedOptions, &eosContainer, &connectionClosed);
}

void NetworkAbstraction::setLobbyMemberChangedCallback()
{
	EOS_Lobby_AddNotifyLobbyMemberStatusReceivedOptions lobbyMemberStatusReceivedOptions = {};
	lobbyMemberStatusReceivedOptions.ApiVersion = EOS_LOBBY_ADDNOTIFYLOBBYMEMBERSTATUSRECEIVED_API_LATEST;
	EOS_Lobby_AddNotifyLobbyMemberStatusReceived(eosContainer.eosLobby, &lobbyMemberStatusReceivedOptions, &eosContainer, &lobbyMemberChangedCallback);
}

EOS_ELoginStatus NetworkAbstraction::getLoginStatus()
{
	return EOS_Connect_GetLoginStatus(eosContainer.eosConnect, eosContainer.userId);
}

void NetworkAbstraction::establishConnection(const EOS_ProductUserId& peer)
{
	EOS_P2P_AcceptConnectionOptions acceptConnectionOptions = {};
	acceptConnectionOptions.ApiVersion = EOS_P2P_ACCEPTCONNECTION_API_LATEST;
	acceptConnectionOptions.LocalUserId = eosContainer.userId;
	acceptConnectionOptions.SocketId = &eosContainer.socketId;
	acceptConnectionOptions.RemoteUserId = peer;
	EOS_EResult result = EOS_P2P_AcceptConnection(eosContainer.eosP2p, &acceptConnectionOptions);
	assert(result == EOS_EResult::EOS_Success);

	eosContainer.peers.push_back(peer);
	utils::log("Established connection");
}

void NetworkAbstraction::closeConnection(const EOS_ProductUserId& peer)
{
	EOS_P2P_CloseConnectionOptions closeConnectionOptions = {};
	closeConnectionOptions.ApiVersion = EOS_P2P_CLOSECONNECTION_API_LATEST;
	closeConnectionOptions.LocalUserId = eosContainer.userId;
	closeConnectionOptions.RemoteUserId = peer;
	closeConnectionOptions.SocketId = &eosContainer.socketId;
	EOS_EResult result = EOS_P2P_CloseConnection(eosContainer.eosP2p, &closeConnectionOptions);
	assert(result == EOS_EResult::EOS_Success);

	removePeerFromContainer(peer, eosContainer.peers);
}

void NetworkAbstraction::joinLobby(EOS_HLobbyDetails lobby)
{
	assert(eosContainer.userId != nullptr);
	assert(eosContainer.lobbyId == "");

	EOS_Lobby_JoinLobbyOptions joinLobbyOptions = {};
	joinLobbyOptions.ApiVersion = EOS_LOBBY_JOINLOBBY_API_LATEST;
	joinLobbyOptions.LocalUserId = eosContainer.userId;
	joinLobbyOptions.LobbyDetailsHandle = lobby;
	joinLobbyOptions.bPresenceEnabled = false;
	EOS_Lobby_JoinLobby(eosContainer.eosLobby, &joinLobbyOptions, &eosContainer, &lobbyJoinCallback);
}

EOS_LobbyDetails_Info* NetworkAbstraction::getLobbyDetails(const EOS_HLobbyDetails& lobbyDetails)
{
	EOS_LobbyDetails_CopyInfoOptions copyInfoOptions = {};
	copyInfoOptions.ApiVersion = EOS_LOBBYDETAILS_COPYINFO_API_LATEST;
	EOS_LobbyDetails_Info* detailsInfo = nullptr;
	EOS_EResult result = EOS_LobbyDetails_CopyInfo(lobbyDetails, &copyInfoOptions, &detailsInfo);
	assert(result == EOS_EResult::EOS_Success);
	
	return detailsInfo;
}

void NetworkAbstraction::leaveLobby()
{
	assert(eosContainer.userId != nullptr);
	assert(eosContainer.lobbyId != "");

	if (eosContainer.searchStatus == SearchStatus::PENDING)
	{
		return;
	}

	EOS_Lobby_LeaveLobbyOptions leaveLobbyOptions = {};
	leaveLobbyOptions.ApiVersion = EOS_LOBBY_LEAVELOBBY_API_LATEST;
	leaveLobbyOptions.LocalUserId = eosContainer.userId;
	leaveLobbyOptions.LobbyId = eosContainer.lobbyId.c_str();
	EOS_Lobby_LeaveLobby(eosContainer.eosLobby, &leaveLobbyOptions, &eosContainer, &lobbyLeaveCallback);

	eosContainer.searchStatus = SearchStatus::PENDING;
}

EOS_HLobbySearch NetworkAbstraction::createLobbySearchHandle()
{
	EOS_HLobbySearch lobbySearchHandle = nullptr;

	EOS_Lobby_CreateLobbySearchOptions createLobbySearchOptions = {};
	createLobbySearchOptions.ApiVersion = EOS_LOBBY_CREATELOBBYSEARCH_API_LATEST;
	createLobbySearchOptions.MaxResults = 1;

	EOS_EResult result = EOS_Lobby_CreateLobbySearch(eosContainer.eosLobby, &createLobbySearchOptions, &lobbySearchHandle);
	assert(result == EOS_EResult::EOS_Success);

	return lobbySearchHandle;
}

NetworkAbstraction::SearchStatus NetworkAbstraction::searchLobby(EOS_HLobbySearch lobbySearchHandle)
{
	if (eosContainer.searchStatus == SearchStatus::NOT_FOUND || eosContainer.searchStatus == SearchStatus::JOINING)
	{
		return eosContainer.searchStatus;
	}
	assert(lobbyName[0] != '\0');

	switch (eosContainer.searchStatus)
	{
		case SearchStatus::NOT_STARTED:
		{
			EOS_LobbySearch_SetMaxResultsOptions maxResultOptions = {};
			maxResultOptions.ApiVersion = EOS_LOBBYSEARCH_SETMAXRESULTS_API_LATEST;
			maxResultOptions.MaxResults = 1;
			EOS_EResult result = EOS_LobbySearch_SetMaxResults(lobbySearchHandle, &maxResultOptions);
			assert(result == EOS_EResult::EOS_Success);

			EOS_LobbySearch_SetLobbyIdOptions setLobbyIdOptions = {};
			setLobbyIdOptions.ApiVersion = EOS_LOBBYSEARCH_SETLOBBYID_API_LATEST;
			setLobbyIdOptions.LobbyId = lobbyName;
			result = EOS_LobbySearch_SetLobbyId(lobbySearchHandle, &setLobbyIdOptions);
			assert(result == EOS_EResult::EOS_Success);

			EOS_LobbySearch_FindOptions lobbyFindOptions = {};
			lobbyFindOptions.ApiVersion = EOS_LOBBYSEARCH_FIND_API_LATEST;
			lobbyFindOptions.LocalUserId = eosContainer.userId;
			EOS_LobbySearch_Find(lobbySearchHandle, &lobbyFindOptions, &eosContainer, &lobbyFindCallback);

			eosContainer.searchStatus = SearchStatus::PENDING;
			break;
		}
		case SearchStatus::PENDING:
		{
			break;
		}
		case SearchStatus::COMPLETE:
		{
			EOS_LobbySearch_GetSearchResultCountOptions resultCountOptions = {};
			resultCountOptions.ApiVersion = EOS_LOBBYSEARCH_GETSEARCHRESULTCOUNT_API_LATEST;
			uint32_t nSearchResults = EOS_LobbySearch_GetSearchResultCount(lobbySearchHandle, &resultCountOptions);

			assert(nSearchResults > 0);
			EOS_LobbySearch_CopySearchResultByIndexOptions copyResultOptions = {};
			copyResultOptions.ApiVersion = EOS_LOBBYSEARCH_COPYSEARCHRESULTBYINDEX_API_LATEST;
			copyResultOptions.LobbyIndex = 0;
			EOS_HLobbyDetails lobbyDetails;
			EOS_EResult result = EOS_LobbySearch_CopySearchResultByIndex(lobbySearchHandle, &copyResultOptions, &lobbyDetails);
			assert(result == EOS_EResult::EOS_Success);

			EOS_LobbyDetails_Info* detailsInfo = getLobbyDetails(lobbyDetails);
			assert(detailsInfo->AvailableSlots > 0);
			EOS_LobbyDetails_Info_Release(detailsInfo);
			detailsInfo = nullptr;

			joinLobby(lobbyDetails);
			EOS_LobbyDetails_Release(lobbyDetails);

			eosContainer.searchStatus = SearchStatus::JOINING;
			break;
		}
	}

	return eosContainer.searchStatus;
}

bool NetworkAbstraction::isJoinedLobby()
{
	return (eosContainer.lobbyId != "");
}

uint32_t NetworkAbstraction::getLobbyPlayerCount()
{
	assert(eosContainer.userId != nullptr);
	assert(eosContainer.lobbyId != "");

	EOS_Lobby_CopyLobbyDetailsHandleOptions lobbyDetailHanldeOptions = {};
	lobbyDetailHanldeOptions.ApiVersion = EOS_LOBBY_COPYLOBBYDETAILSHANDLE_API_LATEST;
	lobbyDetailHanldeOptions.LocalUserId = eosContainer.userId;
	lobbyDetailHanldeOptions.LobbyId = eosContainer.lobbyId.c_str();

	EOS_HLobbyDetails lobbyDetails;
	EOS_EResult eResult = EOS_Lobby_CopyLobbyDetailsHandle(eosContainer.eosLobby, &lobbyDetailHanldeOptions, &lobbyDetails);
	assert(eResult == EOS_EResult::EOS_Success);

	EOS_LobbyDetails_Info* detailsInfo = getLobbyDetails(lobbyDetails);
	uint32_t result = detailsInfo->MaxMembers - detailsInfo->AvailableSlots;
	EOS_LobbyDetails_Info_Release(detailsInfo);
	detailsInfo = nullptr;

	EOS_LobbyDetails_Release(lobbyDetails);

	return result;
}

String NetworkAbstraction::getUserIdString(EOS_ProductUserId userId)
{
	EOS_Bool result = EOS_ProductUserId_IsValid(userId);
	assert(result == EOS_TRUE);
	char userIdString[64];
	int32_t userIdStringSize = arrayCount(userIdString);
	EOS_EResult eResult = EOS_ProductUserId_ToString(userId, userIdString, &userIdStringSize);
	assert(eResult == EOS_EResult::EOS_Success);
	return String(userIdString);
}

void NetworkAbstraction::removePeerFromContainer(const EOS_ProductUserId& peer, Vector<EOS_ProductUserId>& peers)
{
	for (auto it = peers.begin(); it != peers.end(); ++it)
	{
		if (*it == peer)
		{
			utils::log("Erase connected peer");
			if (peers.size() == 1)
			{
				peers.clear();
			}
			else
			{
				peers.erasePop_back(it);
			}
			break;
		}
	}
}

uint32_t NetworkAbstraction::getConnectedPeersCount() const
{
	return eosContainer.peers.size();
}

NetworkAbstraction::NetworkBuffer::~NetworkBuffer()
{
	if (bytes != nullptr)
	{
		free(bytes);
		bytes = nullptr;
	}
	size = 0;
}
