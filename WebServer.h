#ifndef WEBSERVER_H
#define WEBSERVER_H

#include "Platform.h"
#include "Client.h" 
#include "ThreadPool.h"
#include "Request.h"

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <mutex>
#include <algorithm>
#include <sstream>

namespace http
{
	class WebServer
	{
	private:
		//multithreading variables
		std::mutex m_cout_mutex{};
		ThreadPool m_thread_pool{ [this](Client client)
								{
									handleClient(std::move(client));
								} }; 
		//network variables
		WsaData m_wsaData{};
		SocketType m_server_fd{ INVALID_SOCK };
		const int m_port{};
		const int m_buffer_size{};
		sockaddr_in m_address{};
		//setup
		void cleanUp() const;
		bool createSocket();
		bool bindSocket();
		bool startListening() const;
		//run
		void handleClient(Client client);
		Request receiveRequest(SocketType client_fd);
		std::string buildResponse(const std::string& body, int status_code, const std::string& content_type = "text/html; charset=UTF-8");
		std::string serveFile(const std::string& request_path);
		static std::string getContentType(const std::string& path);
		static std::string resolvePath(const std::string& request_path);
		static std::string buildErrorHTML(const std::string& message);
		static void parseHeader(const std::string& raw, Request& request);
		
		//log
		void log(const std::string& message);
		void logError(const std::string& message);
	public:
		WebServer(int port, int buffer_size);
		~WebServer();
		//delete copy/move
		WebServer(const WebServer&) = delete;
		WebServer& operator=(const WebServer&) = delete;
		WebServer(WebServer&&) = delete;
		WebServer& operator=(WebServer&&) = delete;

		void run();
	};
}

#endif
