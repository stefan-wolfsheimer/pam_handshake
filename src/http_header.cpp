#include "http_header.h"
#include <string.h>
#include <exception>

using namespace PamHandshake;
HttpHeader::HttpHeader()
{
  content = nullptr;
  payload_size = 0;
}

HttpHeader::HttpHeader(const char * buff, std::size_t len)
{
#define STATE_CONTENT 0
#define STATE_METHOD 1
#define STATE_URI 2
#define STATE_PROT 3
#define STATE_HEADER 4
#define STATE_VALUE 5
  if(len == 0)
  {
    len = strlen(buff);
  }
  int state = STATE_METHOD;
  const char * last = buff;
  const char * pos = buff;
  std::string key;
  std::string value;
  while(state)
  {
    switch(state)
    {
    case STATE_METHOD:
      if(*pos == ' ')
      {
        state = STATE_URI;
        method.append(last, pos);
        ++pos;
        last = pos;
      }
      else
      {
        ++pos;
      }
      break;
    case STATE_URI:
      if(*pos == ' ')
      {
        state = STATE_PROT;
        uri.append(last, pos);
        ++pos;
        last = pos;
      }
      else
      {
        ++pos;
      }
      break;
    case STATE_PROT:
      if(*pos == '\r')
      {
        proto.append(last, pos);
        ++pos;
        if(*pos == '\n')
        {
          ++pos;
          state = STATE_HEADER;
          last = pos;
        }
        else
        {
          throw std::runtime_error("invalid header");
        }
      }
      else
      {
        ++pos;
      }
      break;
    case STATE_HEADER:
      if(*pos == ':')
      {
        key.append(last, pos);
        ++pos;
        if(*pos == ' ')
        {
          ++pos;
        }
        state = STATE_VALUE;
        last = pos;
      }
      else if(*pos == '\r')
      {
        if(!key.empty())
        {
          throw std::runtime_error("invalid header");
        }
        ++pos;
        if(*pos == '\n')
        {
          ++pos;
          content = pos;
          state = STATE_CONTENT;
          auto itr = values.find("Content-Length");
          payload_size = (itr == values.end()) ? (len - (pos - buff)) : atol(itr->second.c_str());
        }
        else
        {
          throw std::runtime_error("invalid header");
        }
      }
      else
      {
        ++pos;
      }
      break;
    case STATE_VALUE:
      if(*pos == '\r')
      {
        value.append(last, pos);
        values.insert(std::make_pair(key, value));
        key.clear();
        value.clear();
        ++pos;
        if(*pos == '\n')
        {
          ++pos;
          state = STATE_HEADER;
          last = pos;
        }
      }
      else
      {
        ++pos;
      }
      break;
    case STATE_CONTENT:
      break;
    }
  }

#undef STATE_CONTENT
#undef STATE_METHOD
#undef STATE_URI
#undef STATE_PROT
#undef STATE_HEADER
#undef STATE_VALUE
}

HttpHeader::~HttpHeader()
{
}

const char * HttpHeader::response(int code)
{
  switch(code)
  { 
  case 100: return "HTTP/1.1 100 Continue\r\n\r\n";
  case 101: return "HTTP/1.1 101 Switching Protocols\r\n\r\n";
  case 200: return "HTTP/1.1 200 OK\r\n\r\n";
  case 201: return "HTTP/1.1 201 Created\r\n\r\n";
  case 202: return "HTTP/1.1 202 Accepted\r\n\r\n";
  case 203: return "HTTP/1.1 203 Non-Authoritative Information\r\n\r\n";
  case 204: return "HTTP/1.1 204 No Content\r\n\r\n";
  case 205: return "HTTP/1.1 205 Reset Content\r\n\r\n";
  case 206: return "HTTP/1.1 206 Partial Content\r\n\r\n";
  case 300: return "HTTP/1.1 300 Multiple Choices\r\n\r\n";
  case 301: return "HTTP/1.1 301 Moved Permanently\r\n\r\n";
  case 302: return "HTTP/1.1 302 Found\r\n\r\n";
  case 303: return "HTTP/1.1 303 See Other\r\n\r\n";
  case 304: return "HTTP/1.1 304 Not Modified\r\n\r\n";
  case 305: return "HTTP/1.1 305 Use Proxy\r\n\r\n";
  case 307: return "HTTP/1.1 307 Temporary Redirect\r\n\r\n";
  case 400: return "HTTP/1.1 400 Bad Request\r\n\r\n";
  case 401: return "HTTP/1.1 401 Unauthorized\r\n\r\n";
  case 402: return "HTTP/1.1 402 Payment Required\r\n\r\n";
  case 403: return "HTTP/1.1 403 Forbidden\r\n\r\n";
  case 404: return "HTTP/1.1 404 Not Found\r\n\r\n";
  case 405: return "HTTP/1.1 405 Method Not Allowed\r\n\r\n";
  case 406: return "HTTP/1.1 406 Not Acceptable\r\n\r\n";
  case 407: return "HTTP/1.1 407 Proxy Authentication Required\r\n\r\n";
  case 408: return "HTTP/1.1 408 Request Timeout\r\n\r\n";
  case 409: return "HTTP/1.1 409 Conflict\r\n\r\n";
  case 410: return "HTTP/1.1 410 Gone\r\n\r\n";
  case 411: return "HTTP/1.1 411 Length Required\r\n\r\n";
  case 412: return "HTTP/1.1 412 Precondition Failed\r\n\r\n";
  case 413: return "HTTP/1.1 413 Payload Too Large\r\n\r\n";
  case 414: return "HTTP/1.1 414 URI Too Long\r\n\r\n";
  case 415: return "HTTP/1.1 415 Unsupported Media Type\r\n\r\n";
  case 416: return "HTTP/1.1 416 Range Not Satisfiable\r\n\r\n";
  case 417: return "HTTP/1.1 417 Expectation Failed\r\n\r\n";
  case 426: return "HTTP/1.1 426 Upgrade Required\r\n\r\n";
  case 500: return "HTTP/1.1 500 Internal Server Error\r\n\r\n";
  case 501: return "HTTP/1.1 501 Not Implemented\r\n\r\n";
  case 502: return "HTTP/1.1 502 Bad Gateway\r\n\r\n";
  case 503: return "HTTP/1.1 503 Service Unavailable\r\n\r\n";
  case 504: return "HTTP/1.1 504 Gateway Timeout\r\n\r\n";
  case 505: return "HTTP/1.1 505 HTTP Version Not Supported\r\n\r\n";
  }
  return "";
}

::std::ostream & operator<<(::std::ostream & ost, const HttpHeader & rhs)
{
  ost << rhs.method << " " << rhs.uri << " " << rhs.proto << "\r\n";
  for(auto & kv : rhs.values)
  {
    ost << kv.first << ": " <<  kv.second << "\r\n"; 
  }
  return ost;
}
