#ifndef REQUEST_H
#define REQUEST_H


#include "Platform.h"
#include <string>
#include <map>
#include <vector>

namespace http
{
	struct Request
	{
		std::string method{};
		std::string path{};
		std::map<std::string, std::string> headers{};
		std::string body{};

		std::string getHeader(const std::string& name) const;
		std::size_t getContentLength() const;
		std::string getContentDisposition() const;
	};
}

#endif