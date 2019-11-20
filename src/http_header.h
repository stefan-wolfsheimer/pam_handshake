#pragma once
#include <string>
#include <map>
#include <iostream>

namespace PamHandshake
{
  struct HttpHeader
  {
    std::string method;
    std::string uri;
    std::string proto;
    std::map<std::string, std::string> values;
    std::size_t payload_size;
    const char * content;
    HttpHeader();
    HttpHeader(const char * buff, std::size_t len = 0);
    ~HttpHeader();
    static const char * response(int code);
  };
}

std::ostream & operator<<(std::ostream & ost, const PamHandshake::HttpHeader & rhs);
