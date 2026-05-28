#include "Request.h"

namespace http
{
	std::string Request::getHeader(const std::string& name) const
	{
		//search for header by name and return it
		auto e{ headers.find(name) };
		if (e != headers.end())
			return e->second;
		return "";
	}
	//returns number of bytes in the body of the request, declared in header
	std::size_t Request::getContentLength() const
	{
		std::string value{ getHeader("Content-Length") };
		if (value.empty())
			return 0;
		return static_cast<std::size_t>(std::stoul(value));
	}
}