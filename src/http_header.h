#pragma once
#include <string>
#include <map>
#include <iostream>

namespace PamHandshake
{
  struct HttpHeader
  {
    HttpHeader();
    ~HttpHeader();
    static const char * response(int code);
    static const char * responseWithLength(int code);
    void parse_chunk(const char * buff, std::size_t len);
    bool header_read() const;
    std::string method;
    std::string uri;
    std::string proto;
    std::map<std::string, std::string> values;
    std::size_t payload_size;
    std::string content;
  private:
    int state;
    std::string curr_key;
    std::string curr_value;
  };
}

std::ostream & operator<<(std::ostream & ost, const PamHandshake::HttpHeader & rhs);
