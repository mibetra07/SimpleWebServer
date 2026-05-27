#include "Client.h"

namespace http
{
	Client::Client() : m_address{}, m_address_len{ static_cast <SockLen>(sizeof(m_address)) },
		m_client_fd{ INVALID_SOCK }, m_ip{}
	{
	}
	Client::~Client()
	{
		closeSocket(m_client_fd);
	}
	Client::Client(Client&& other) noexcept
		: m_address{ other.m_address }, m_address_len{ other.m_address_len },
		m_client_fd{ other.m_client_fd }, m_ip{ other.m_ip }
	{
		other.m_client_fd = INVALID_SOCK;
	}
	Client& Client::operator=(Client&& other) noexcept
	{
		if (this != &other)
		{
			closeSocket(m_client_fd);

			m_address = other.m_address;
			m_address_len = other.m_address_len;
			m_client_fd = other.m_client_fd;
			m_ip = other.m_ip;

			other.m_client_fd = INVALID_SOCK;
		}
		return *this;
	}
}