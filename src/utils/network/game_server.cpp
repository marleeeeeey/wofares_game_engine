// #include "game_server.h"
// #include <cassert>
// #include <steam/isteamnetworkingutils.h>
// #include <steam/steamnetworkingsockets.h>
// #include <string>
// #include <utils/logger.h>


// GameServer* GameServer::s_pCallbackInstance = nullptr;

// GameServer::GameServer()
// {}

// void GameServer::StartListening(uint16 nPort)
// {
//     // Select instance to use.  For now we'll always use the default.
//     // But we could use SteamGameServerNetworkingSockets() on Steam.
//     pInterface = SteamNetworkingSockets();

//     // Start listening
//     SteamNetworkingIPAddr serverLocalAddr;
//     serverLocalAddr.Clear();
//     serverLocalAddr.m_port = nPort;
//     SteamNetworkingConfigValue_t opt;
//     opt.SetPtr(k_ESteamNetworkingConfig_Callback_ConnectionStatusChanged, (void*)OnConnectionStatusChanged);
//     hListenSock = pInterface->CreateListenSocketIP(serverLocalAddr, 1, &opt);
//     if (hListenSock == k_HSteamListenSocket_Invalid)
//         MY_LOG(error, "[GameServer] Failed to listen on port {}", nPort);

//     hPollGroup = pInterface->CreatePollGroup();
//     if (hPollGroup == k_HSteamNetPollGroup_Invalid)
//         MY_LOG(error, "[GameServer] Failed to listen on port {}", nPort);

//     MY_LOG(info, "[GameServer] Server listening on port {}", nPort);
// }

// void GameServer::StopListening()
// {
//     // Close all the connections
//     MY_LOG(info, "[GameServer] Closing connections...");
//     for (auto it : mapClients)
//     {
//         // Send them one more goodbye message.  Note that we also have the
//         // connection close reason as a place to send final data.  However,
//         // that's usually best left for more diagnostic/debug text not actual
//         // protocol strings.
//         SendStringToClient(it.first, "Server is shutting down. Goodbye.");

//         // Close the connection.  We use "linger mode" to ask SteamNetworkingSockets
//         // to flush this out and close gracefully.
//         pInterface->CloseConnection(it.first, 0, "Server Shutdown", true);
//     }
//     mapClients.clear();

//     pInterface->CloseListenSocket(hListenSock);
//     hListenSock = k_HSteamListenSocket_Invalid;

//     pInterface->DestroyPollGroup(hPollGroup);
//     hPollGroup = k_HSteamNetPollGroup_Invalid;

//     MY_LOG(info, "[GameServer] Server stopped listening");
// }

// void GameServer::Update()
// {
//     PollIncomingMessages();
//     PollConnectionStateChanges();
// }

// void GameServer::SendStringToClient(HSteamNetConnection conn, const char* str)
// {
//     pInterface->SendMessageToConnection(conn, str, (uint32)strlen(str), k_nSteamNetworkingSend_Reliable, nullptr);
// }

// void GameServer::SendStringToAllClients(const char* str, HSteamNetConnection except)
// {
//     for (auto& c : mapClients)
//     {
//         if (c.first != except)
//             SendStringToClient(c.first, str);
//     }
// }

// // TODO1: We also should know name of the client who sent the message.
// std::vector<std::string> GameServer::PollIncomingMessages()
// {
//     std::vector<std::string> messages;

//     while (true)
//     {
//         ISteamNetworkingMessage* pIncomingMsg = nullptr;
//         int numMsgs = pInterface->ReceiveMessagesOnPollGroup(hPollGroup, &pIncomingMsg, 1);
//         if (numMsgs == 0)
//             break;
//         if (numMsgs < 0)
//             MY_LOG(error, "[GameServer] Error checking for messages");
//         assert(numMsgs == 1 && pIncomingMsg);
//         [[maybe_unused]] auto itClient = mapClients.find(pIncomingMsg->m_conn);
//         assert(itClient != mapClients.end());

//         std::string message;
//         message.assign((const char*)pIncomingMsg->m_pData, pIncomingMsg->m_cbSize);
//         messages.push_back(std::move(message));

//         // We don't need this anymore.
//         pIncomingMsg->Release();
//     }

//     return messages;
// }

// void GameServer::SetClientNick(HSteamNetConnection hConn, const char* nick)
// {
//     // Remember their nick
//     mapClients[hConn].m_sNick = nick;

//     // Set the connection name, too, which is useful for debugging
//     pInterface->SetConnectionName(hConn, nick);
// }

// void GameServer::OnSteamNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo)
// {
//     // What's the state of the connection?
//     switch (pInfo->m_info.m_eState)
//     {
//     case k_ESteamNetworkingConnectionState_None:
//         // NOTE: We will get callbacks here when we destroy connections.  You can ignore these.
//         break;

//     case k_ESteamNetworkingConnectionState_ClosedByPeer:
//     case k_ESteamNetworkingConnectionState_ProblemDetectedLocally:
//         {
//             // Ignore if they were not previously connected.  (If they disconnected
//             // before we accepted the connection.)
//             if (pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connected)
//             {
//                 // Locate the client.  Note that it should have been found, because this
//                 // is the only codepath where we remove clients (except on shutdown),
//                 // and connection change callbacks are dispatched in queue order.
//                 auto itClient = mapClients.find(pInfo->m_hConn);
//                 assert(itClient != mapClients.end());

//                 std::string whatHappened;

//                 if (pInfo->m_info.m_eState == k_ESteamNetworkingConnectionState_ProblemDetectedLocally)
//                 {
//                     whatHappened = MY_FMT(
//                         "[GameServer] Client {}: Problem detected locally. Desc={}. EndReason={}. EndDebug={}",
//                         itClient->second.m_sNick, pInfo->m_info.m_szConnectionDescription, pInfo->m_info.m_eEndReason,
//                         pInfo->m_info.m_szEndDebug);
//                 }
//                 else
//                 {
//                     whatHappened = MY_FMT(
//                         "[GameServer] Client {}: Closed by peer. Desc={}. EndReason={}. EndDebug={}",
//                         itClient->second.m_sNick, pInfo->m_info.m_szConnectionDescription, pInfo->m_info.m_eEndReason,
//                         pInfo->m_info.m_szEndDebug);
//                 }

//                 mapClients.erase(itClient);

//                 // Send a message so everybody else knows what happened
//                 SendStringToAllClients(whatHappened.c_str());
//             }
//             else
//             {
//                 assert(pInfo->m_eOldState == k_ESteamNetworkingConnectionState_Connecting);
//             }

//             // Clean up the connection.  This is important!
//             // The connection is "closed" in the network sense, but
//             // it has not been destroyed.  We must close it on our end, too
//             // to finish up.  The reason information do not matter in this case,
//             // and we cannot linger because it's already closed on the other end,
//             // so we just pass 0's.
//             pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
//             break;
//         }

//     case k_ESteamNetworkingConnectionState_Connecting:
//         {
//             // This must be a new connection
//             assert(mapClients.find(pInfo->m_hConn) == mapClients.end());

//             MY_LOG(info, "[GameServer] Connection request from {}", pInfo->m_info.m_szConnectionDescription);

//             // A client is attempting to connect
//             // Try to accept the connection.
//             if (pInterface->AcceptConnection(pInfo->m_hConn) != k_EResultOK)
//             {
//                 // This could fail.  If the remote host tried to connect, but then
//                 // disconnected, the connection may already be half closed.  Just
//                 // destroy whatever we have on our side.
//                 pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
//                 MY_LOG(
//                     warn, "[GameServer] Failed to accept connection from {}. (It was already closed?)",
//                     pInfo->m_info.m_szConnectionDescription);
//                 break;
//             }

//             // Assign the poll group
//             if (!pInterface->SetConnectionPollGroup(pInfo->m_hConn, hPollGroup))
//             {
//                 pInterface->CloseConnection(pInfo->m_hConn, 0, nullptr, false);
//                 MY_LOG(warn, "[GameServer] Failed to set poll group?");
//                 break;
//             }

//             // Generate a random nick.  A random temporary nick
//             // is really dumb and not how you would write a real chat server.
//             // You would want them to have some sort of signon message,
//             // and you would keep their client in a state of limbo (connected,
//             // but not logged on) until them.  I'm trying to keep this example
//             // code really simple.
//             char nick[64];
//             sprintf(nick, "BraveWarrior%d", 10000 + (rand() % 100000));

//             // Send them a welcome message
//             std::string welcomeMsg = MY_FMT(
//                 "Welcome, stranger. Thou art known to us for now as '{}'; upon thine command '/nick' we shall know thee otherwise.",
//                 nick);
//             SendStringToClient(pInfo->m_hConn, welcomeMsg.c_str());

//             // Also send them a list of everybody who is already connected
//             if (mapClients.empty())
//             {
//                 SendStringToClient(pInfo->m_hConn, "You are alone in the chat.");
//             }
//             else
//             {
//                 for (auto& c : mapClients)
//                     SendStringToClient(pInfo->m_hConn, c.second.m_sNick.c_str());
//             }

//             // Let everybody else know who they are for now
//             std::string greetingFromClient =
//                 MY_FMT("Hark! A stranger hath joined this merry host. For now we shall call them '{}'", nick);
//             SendStringToAllClients(greetingFromClient.c_str(), pInfo->m_hConn);

//             // Add them to the client list, using std::map wacky syntax
//             mapClients[pInfo->m_hConn];
//             SetClientNick(pInfo->m_hConn, nick);
//             break;
//         }

//     case k_ESteamNetworkingConnectionState_Connected:
//         // We will get a callback immediately after accepting the connection.
//         // Since we are the server, we can ignore this, it's not news to us.
//         break;

//     default:
//         // Silences -Wswitch
//         break;
//     }
// }

// void GameServer::OnConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t* pInfo)
// {
//     s_pCallbackInstance->OnSteamNetConnectionStatusChanged(pInfo);
// }

// void GameServer::PollConnectionStateChanges()
// {
//     s_pCallbackInstance = this;
//     pInterface->RunCallbacks();
// }