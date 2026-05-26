#ifndef CLIENT_H
#define CLIENT_H

#include "Platform.h"
#include <array>

namespace http
{
	class WebServer;

	class Client
	{
	private:
		sockaddr_in m_address;
		int m_address_len;
		SocketType m_client_fd;
		std::array<char, INET_ADDRSTRLEN> m_ip;
	public:
		Client();
		~Client();
		//remove copy
		Client(const Client&) = delete;
		Client& operator=(const Client& other) = delete;
		//enable move
		Client(Client&& other) noexcept;
		Client& operator=(Client&& other) noexcept;

		friend class WebServer;
	};
}

#endif

