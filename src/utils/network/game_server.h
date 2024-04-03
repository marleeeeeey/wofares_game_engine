#pragma once
#include <map>
#include <steam/isteamnetworkingsockets.h>
#include <steam/steamnetworkingtypes.h>
#include <string>
#include <vector>

// WIP
class GameServer
{
    HSteamListenSocket hListenSock;
    HSteamNetPollGroup hPollGroup;
    ISteamNetworkingSockets* pInterface;
    struct Client_t
    {
        std::string m_sNick;
    };
    std::map<HSteamNetConnection, Client_t> mapClients;
public:
    GameServer();
    void StartListening(uint16 nPort);
    void StopListening();
    // Should called from the main loop on every frame.
    void Update();
private:
    void SendStringToClient(HSteamNetConnection conn, const char* str);
    void SendStringToAllClients(const char* str, HSteamNetConnection except = k_HSteamNetConnection_Invalid);
    std::vector<std::string> PollIncomingMessages();
    void SetClientNick(HSteamNetConnection hConn, const char* nick);
private: // OnSteamNetConnectionStatusChanged stuff.
    void OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo);
    static GameServer* s_pCallbackInstance;
    static void OnConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo);
    void PollConnectionStateChanges();
};
