#include "WebServer.h"

namespace http
{
	//log
	void WebServer::log(const std::string& message)
	{
		std::lock_guard<std::mutex> lock{ m_cout_mutex };
		std::cout << message << '\n';
	}

	void WebServer::logError(const std::string& message)
	{
		std::lock_guard<std::mutex> lock{ m_cout_mutex };
		std::cerr << message << '\n';
	}

	void WebServer::cleanUp() const
	{
		closeSocket(m_server_fd);
		cleanupNetwork();
	}
	//Constructor
	bool WebServer::createSocket()
	{
		m_server_fd = socket(AF_INET, SOCK_STREAM, 0);
		if (m_server_fd == INVALID_SOCK)
			return false;
		return true;
	}
	bool WebServer::bindSocket()
	{
		m_address.sin_family = AF_INET;
		m_address.sin_addr.s_addr = INADDR_ANY;
		m_address.sin_port = htons(m_port);
		if (bind(m_server_fd, reinterpret_cast<sockaddr*>(&m_address), sizeof(m_address)) == SOCKET_ERR)
			return false;
		return true;
	}
	bool WebServer::startListening() const
	{
		if (listen(m_server_fd, SOMAXCONN) == SOCKET_ERR)
			return false;
		return true;
	}
	WebServer::WebServer(int port, int buffer_size) 
		: m_port{ port }, m_buffer_size{ buffer_size }
	{
		int result{ initNetwork(m_wsaData) };
		if (result != 0)
			throw std::runtime_error("Startup failed! ErrorCode: " + std::to_string(result) + '\n');
		//Cleanup() needs to be called
		if (!createSocket())
		{
			std::string error{ getLastError() };
			cleanupNetwork(); //Socket hasnt been created, only network cleanup
			throw std::runtime_error("Socket creation failed! ErrorCode: " + error + '\n');
		}
		if (!bindSocket())
		{
			std::string error{ getLastError() };
			cleanUp();
			throw std::runtime_error("bind() failed! ErrorCode: " + error + '\n');
		}
		if (!startListening())
		{
			std::string error{ getLastError() };
			cleanUp();
			throw std::runtime_error("Listen failed! ErrorCode: " + error + '\n');
		}
		log("Server is listening on port " + std::to_string(m_port) + "...");
		log("Thread pool started with "
			+ std::to_string(std::thread::hardware_concurrency() * 2)
			+ " threads.");
	}
	//Destructor
	WebServer::~WebServer()
	{ 
		cleanUp(); 
	}

	//Response building
	std::string WebServer::buildResponse(const std::string& body, int status_code, const std::string& content_type)
	{
		std::string status_message{};
		//set status_message according to status_code
		switch (status_code)
		{
		case 200: status_message = "OK";        break;
		case 403: status_message = "Forbidden";             break;
		case 404: status_message = "Not Found"; break;
		case 405: status_message = "Method Not Allowed";    break;
		case 500: status_message = "Internal Server Error"; break;
		default:  status_message = "Unknown";   break;
		}
		//return fully built response
		return	"HTTP/1.1 " + std::to_string(status_code) + " " + status_message + "\r\n"
			"Content-Type: " + content_type + "\r\n"
			"Content-Length: " + std::to_string(body.size()) + "\r\n"
			"Connection: close\r\n"
			"\r\n" +
			body;
	}

	std::string WebServer::getContentType(const std::string& path)
	{
		//extract file suffix
		std::size_t dot_pos{ path.rfind('.') };
		if (dot_pos == std::string::npos)
			return "application/octet-stream"; //unknown type

		std::string extension{ path.substr(dot_pos) };

		if (extension == ".html" || extension == ".htm")
			return "text/html; charset=UTF-8";
		if (extension == ".css")
			return "text/css";
		if (extension == ".js")
			return "application/javascript";
		if (extension == ".json")
			return "application/json";
		if (extension == ".png")
			return "image/png";
		if (extension == ".jpg" || extension == ".jpeg")
			return "image/jpeg";
		if (extension == ".ico")
			return "image/x-icon";
		if (extension == ".txt")
			return "text/plain";

		return "application/octet-stream"; //unknown type
	}

	std::string WebServer::resolvePath(const std::string& request_path)
	{
		//root directory of server
		std::string base_path{ "./public" };
		//"/" -> "/index.html"
		if (request_path == "/")
			return base_path + "/index.html";
		//stop path traversal
		if (request_path.find("..") != std::string::npos)
			return "";

		return base_path + request_path;
	}

	std::string WebServer::buildErrorHTML(const std::string& message)
	{
		return "<html><body><h1>" + message + "</h1></body></html>";
	}

	std::string WebServer::serveFile(const std::string& request_path)
	{
		std::string resolved_path{ resolvePath(request_path) };
		//path is invalid 
		if (resolved_path.empty())
		{
			return buildResponse(buildErrorHTML("403 Forbidden"), 403);
		}
		//open file
		std::ifstream file{ resolved_path, std::ios::binary };
		//file not open = not found
		if (!file.is_open())
		{
			return buildResponse(buildErrorHTML("404 Not Found"), 404);
		}
		//get file size
		file.seekg(0, std::ios::end);
		std::streamsize file_size{ file.tellg() };
		file.seekg(0, std::ios::beg);
		//convert file into string
		std::string body(static_cast<std::size_t>(file_size), '\0');
		if (!file.read(&body[0], file_size))
		{
			return buildResponse(buildErrorHTML("500 Internal Server Error"), 500); //Internal Server Error
		}

		return buildResponse(body, 200, getContentType(resolved_path));
	}
	//configure request struct
	void WebServer::parseHeader(const std::string& raw, Request& request)
	{
		std::istringstream stream{ raw };
		std::string line{};
		//first line: "GET /path HTTP/1.1"
		if (std::getline(stream, line))
		{
			//delete \r if at the end
			if (!line.empty() && line.back() == '\r')
				line.pop_back();
			std::istringstream first_line{ line };
			//write method and path into request
			first_line >> request.method >> request.path;
		}
		//read rest of the lines and write headers and their content into request
		while (std::getline(stream, line))
		{
			//delete \r if at the end
			if (!line.empty() && line.back() == '\r')
				line.pop_back();
			//find start of the content of that header
			std::size_t colon{ line.find(':') };
			if (colon != std::string::npos)
			{
				std::string name{ line.substr(0, colon) };
				std::string value{ line.substr(colon + 2) };
				request.headers[name] = value;
			}
		}
	}

	Request WebServer::receiveRequest(SocketType client_fd)
	{
		Request request{};
		std::string raw_header{};
		//read header till \r\n\r\n
		std::vector<char> chunk(m_buffer_size);
		while (true)
		{
			//append m_buffer_size bytes from client to string
			SockResult bytes{ recv(client_fd, chunk.data(), m_buffer_size - 1, 0) };
			if (bytes == SOCKET_ERR)
			{
				logError("recv() failed while reading header: " + getLastError());
				return Request{};
			}
			if (bytes <= 0)
				return request;
			raw_header.append(chunk.data(), static_cast<std::size_t>(bytes));
			//check if header is complete
			std::size_t header_end{ raw_header.find("\r\n\r\n") };
			if (header_end != std::string::npos)
			{
				//start of the body that got sent with end of header
				std::string body_start{ raw_header.substr(header_end + 4) };
				//complete raw header
				raw_header = raw_header.substr(0, header_end);

				parseHeader(raw_header, request);
				//complete body
				std::size_t content_length{ request.getContentLength() };
				if (content_length > 0)
				{
					//append start of the body
					request.body.reserve(content_length);
					request.body.append(body_start);
					//append rest of the body
					while (request.body.size() < content_length)
					{
						std::size_t remaining{ content_length - request.body.size() };
						std::size_t to_read{ std::min<std::size_t>(
											 remaining, static_cast<std::size_t>(m_buffer_size)) };
						//receive next chunk of bytes
						SockResult body_bytes{ recv(client_fd, chunk.data(), static_cast<int>(to_read), 0) };
						if (bytes == SOCKET_ERR)
						{
							logError("recv() failed while reading body: " + getLastError());
							return Request{};
						}
						//no bytes were sent
						if (body_bytes <= 0)
							break;
						//append chunk to body
						request.body.append(chunk.data(), static_cast<std::size_t>(body_bytes));
					}
				}
				break;
			}
		}
		return request;
	}
	//Client handling
	//TODO: rewrite so response is sent in multiple segments, if needed (check size of file in GET)
	void WebServer::handleClient(Client client)
	{
		//receive request
		Request request{ receiveRequest(client.m_client_fd) };
		if (request.method.empty())
		{
			log("Client disconnected.");
			return;
		}

		log("Method: " + request.method + "\nPath: " + request.path);
		//build response according to method in request
		std::string response{};
		if (request.method == "GET")
		{
			response = serveFile(request.path);
		}
		else
		{
			response = buildResponse(buildErrorHTML("405 Method Not Allowed"), 405);
		}
		//send response
		SockResult bytes_sent{ send(client.m_client_fd, response.c_str(), static_cast<int>(response.size()), 0) };
		if (bytes_sent == SOCKET_ERR)
		{
			logError("send() failed: " + getLastError());
			return;
		}
	}

	void WebServer::run()
	{
		while (true)
		{
			Client client{};
			client.m_client_fd = accept(m_server_fd, reinterpret_cast<sockaddr*>(&client.m_address), &client.m_address_len);
			if (client.m_client_fd == INVALID_SOCK)
			{
				logError("Accept failed! ErrorCode: " + getLastError());
				continue;
			}
			inet_ntop(AF_INET, &client.m_address.sin_addr, client.m_ip.data(), INET_ADDRSTRLEN);
			//Output client IP
			log("Client connected: " + std::string{ client.m_ip.data() } + ":" + std::to_string(ntohs(client.m_address.sin_port)));
			//move handleClient(client) into queue for threadpool
			m_thread_pool.enqueue(std::move(client));
		}
	}
}