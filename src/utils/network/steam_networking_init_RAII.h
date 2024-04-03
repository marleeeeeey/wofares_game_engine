#pragma once
#include <functional>
#include <steam/steamnetworkingtypes.h>

class SteamNetworkingInitRAII
{
public:
    struct Options
    {
        ESteamNetworkingSocketsDebugOutputType debugSeverity = k_ESteamNetworkingSocketsDebugOutputType_Msg;
    };

    SteamNetworkingInitRAII(const Options& options);
    ~SteamNetworkingInitRAII();
public: // *** Implementa setting debug callback ***
    static void SetDebugCallback(std::function<void(ESteamNetworkingSocketsDebugOutputType, const char*)> callback);
private:
    static void OnDebugOutput(ESteamNetworkingSocketsDebugOutputType eType, const char* pszMsg);
    static std::function<void(ESteamNetworkingSocketsDebugOutputType, const char*)> debugCallback;
private: // Options
    Options options;
};