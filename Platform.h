#ifndef PLATFORM_H
#define PLATFORM_H
#include <string>

#ifdef _WIN32 //Windows
	#define WIN32_LEAN_AND_MEAN
	#include <winsock2.h>
	#include <ws2tcpip.h>
	#pragma comment(lib, "ws2_32.lib")

	using SocketType = SOCKET;
	using SockResult = int;
	using SockLen = int;
	using WsaData = WSADATA;
	constexpr SocketType INVALID_SOCK = INVALID_SOCKET;
	constexpr int SOCKET_ERR = SOCKET_ERROR;
	
	inline int initNetwork(WsaData& wsaData)
	{
		return WSAStartup(MAKEWORD(2, 2), &wsaData);
	}
	inline void cleanupNetwork()
	{
		WSACleanup();
	}
	inline void closeSocket(SocketType socket)
	{
		if (socket != INVALID_SOCK)
			closesocket(socket);
	}
	inline std::string getLastError()
	{
		return std::to_string(WSAGetLastError());
	}

#else // Unix
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <unistd.h>
	#include <cerrno>

	using SocketType = int;
	using SockResult = ssize_t;
	using SockLen = socklen_t;
	struct WsaData {};
	constexpr SocketType INVALID_SOCK = -1;
	constexpr int SOCKET_ERR = -1;

	inline int initNetwork(WsaData&)
	{
		return 0;  //not needed
	}
	inline void cleanupNetwork()
	{
		// not needed
	}
	inline void closeSocket(SocketType socket)
	{
		if (socket != INVALID_SOCK)
			close(socket);  // close() instead of closesocket()
	}
	inline std::string getLastError()
	{
		return std::to_string(errno);
	}
#endif

#endif
