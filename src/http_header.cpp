#include "http_header.h"
#include <string.h>
#include <exception>

#define STATE_CONTENT 0
#define STATE_METHOD 1
#define STATE_URI 2
#define STATE_PROT 3
#define STATE_NL 4
#define STATE_VALUE_SPACE 5
#define STATE_HEADER 6
#define STATE_KEY 6
#define STATE_VALUE 7
#define STATE_CONTENT_NL 8

using namespace PamHandshake;
HttpHeader::HttpHeader()
{
  payload_size = 0;
  state = STATE_METHOD;
}

void HttpHeader::parse_chunk(const char * buff, std::size_t len)
{
  // @todo more efficient buffering
  const char * pos = buff;
  for(std::size_t i = 0; i < len; i++)
  {
    if(*pos == '\0')
    {
      break;
    }
    switch(state)
    {
    case STATE_METHOD:
      if(*pos == ' ')
      {
        state = STATE_URI;
      }
      else
      {
        method.push_back(*pos);
      }
      break;
    case STATE_URI:
      if(*pos == ' ')
      {
        state = STATE_PROT;
      }
      else
      {
        uri.push_back(*pos);
      }
      break;
    case STATE_PROT:
      if(*pos == '\r')
      {
        state = STATE_NL;
      }
      else
      {
        proto.push_back(*pos);
      }
      break;
    case STATE_NL:
      if(*pos == '\n')
      {
        state = STATE_HEADER;
      }
      else
      {
        throw std::runtime_error("invalid header");
      }
      break;
    case STATE_HEADER:
      if(*pos == ':')
      {
        state = STATE_VALUE_SPACE;
      }
      else if(*pos == '\r')
      {
        if(!curr_key.empty())
        {
          throw std::runtime_error("invalid header");
        }
        state = STATE_CONTENT_NL;
      }
      else
      {
        curr_key.push_back(*pos);
      }
      break;
    case STATE_VALUE_SPACE:
      if(*pos != ' ')
      {
        curr_value.push_back(*pos);
        state = STATE_VALUE;
      }
      break;
    case STATE_VALUE:
      if(*pos == '\r')
      {
        state = STATE_NL;
        values.insert(std::make_pair(curr_key, curr_value));
        if(curr_key == "Content-Length")
        {
          payload_size = atol(curr_value.c_str());
        }
        curr_key.clear();
        curr_value.clear();
      }
      else
      {
        curr_value.push_back(*pos);
      }
      break;
    case STATE_CONTENT_NL:
      if(*pos == '\n')
      {
        state = STATE_CONTENT;
      }
      else
      {
        throw std::runtime_error("invalid header");
      }
      break;
    case STATE_CONTENT:
      content.push_back(*pos);
    }
    pos++;
  }
}

bool HttpHeader::header_read() const
{
  return state == STATE_CONTENT;
}

#undef STATE_CONTENT
#undef STATE_METHOD
#undef STATE_URI
#undef STATE_PROT
#undef STATE_NL
#undef STATE_VALUE_SPACE
#undef STATE_HEADER
#undef STATE_KEY
#undef STATE_VALUE
#undef STATE_CONTENT_NL

HttpHeader::~HttpHeader()
{
}

const char * HttpHeader::responseWithLength(int code)
{
  switch(code)
  { 
  case 100: return "HTTP/1.1 100 Continue\r\nConnection: close\r\nContent-Length: %d\r\n\r\n";
  case 101: return "HTTP/1.1 101 Switching Protocols\r\nConnection: close\r\nContent-Length: %d\r\n\r\n";
  case 200: return "HTTP/1.1 200 OK\r\nConnection: close\r\nContent-Length: %d\r\n\r\n";
  case 201: return "HTTP/1.1 201 Created\r\nConnection: close\r\nContent-Length: %d\r\n\r\n";
  case 202: return "HTTP/1.1 202 Accepted\r\nConnection: close\r\nContent-Length: %d\r\n\r\n";
  case 203: return "HTTP/1.1 203 Non-Authoritative Information\r\nConnection: close\r\nContent-Length: %d\r\n\r\n";
  case 204: return "HTTP/1.1 204 No Content\r\nConnection: close\r\nContent-Length: %d\r\n\r\n";
  case 205: return "HTTP/1.1 205 Reset Content\r\nConnection: close\r\nContent-Length: %d\r\n\r\n";
  case 206: return "HTTP/1.1 206 Partial Content\r\nConnection: close\r\nContent-Length: %d\r\n\r\n";
  case 300: return "HTTP/1.1 300 Multiple Choices\r\nConnection: close\r\nContent-Length: %d\r\n\r\n";
  case 301: return "HTTP/1.1 301 Moved Permanently\r\nConnection: close\r\nContent-Length: %d\r\n\r\n";
  case 302: return "HTTP/1.1 302 Found\r\nConnection: close\r\nContent-Length: %d\r\n\r\n";
  case 303: return "HTTP/1.1 303 See Other\r\nConnection: close\r\nContent-Length: %d\r\n\r\n";
  case 304: return "HTTP/1.1 304 Not Modified\r\nConnection: close\r\nContent-Length: %d\r\n\r\n";
  case 305: return "HTTP/1.1 305 Use Proxy\r\nConnection: close\r\nContent-Length: %d\r\n\r\n";
  case 307: return "HTTP/1.1 307 Temporary Redirect\r\nConnection: close\r\nContent-Length: %d\r\n\r\n";
  case 400: return "HTTP/1.1 400 Bad Request\r\nConnection: close\r\nContent-Length: %d\r\n\r\n";
  case 401: return "HTTP/1.1 401 Unauthorized\r\nConnection: close\r\nContent-Length: %d\r\n\r\n";
  case 402: return "HTTP/1.1 402 Payment Required\r\nConnection: close\r\nContent-Length: %d\r\n\r\n";
  case 403: return "HTTP/1.1 403 Forbidden\r\nConnection: close\r\nContent-Length: %d\r\n\r\n";
  case 404: return "HTTP/1.1 404 Not Found\r\nConnection: close\r\nContent-Length: %d\r\n\r\n";
  case 405: return "HTTP/1.1 405 Method Not Allowed\r\nConnection: close\r\nContent-Length: %d\r\n\r\n";
  case 406: return "HTTP/1.1 406 Not Acceptable\r\nConnection: close\r\nContent-Length: %d\r\n\r\n";
  case 407: return "HTTP/1.1 407 Proxy Authentication Required\r\nConnection: close\r\nContent-Length: %d\r\n\r\n";
  case 408: return "HTTP/1.1 408 Request Timeout\r\nConnection: close\r\nContent-Length: %d\r\n\r\n";
  case 409: return "HTTP/1.1 409 Conflict\r\nConnection: close\r\nContent-Length: %d\r\n\r\n";
  case 410: return "HTTP/1.1 410 Gone\r\nConnection: close\r\nContent-Length: %d\r\n\r\n";
  case 411: return "HTTP/1.1 411 Length Required\r\nConnection: close\r\nContent-Length: %d\r\n\r\n";
  case 412: return "HTTP/1.1 412 Precondition Failed\r\nConnection: close\r\nContent-Length: %d\r\n\r\n";
  case 413: return "HTTP/1.1 413 Payload Too Large\r\nConnection: close\r\nContent-Length: %d\r\n\r\n";
  case 414: return "HTTP/1.1 414 URI Too Long\r\nConnection: close\r\nContent-Length: %d\r\n\r\n";
  case 415: return "HTTP/1.1 415 Unsupported Media Type\r\nConnection: close\r\nContent-Length: %d\r\n\r\n";
  case 416: return "HTTP/1.1 416 Range Not Satisfiable\r\nConnection: close\r\nContent-Length: %d\r\n\r\n";
  case 417: return "HTTP/1.1 417 Expectation Failed\r\nConnection: close\r\nContent-Length: %d\r\n\r\n";
  case 426: return "HTTP/1.1 426 Upgrade Required\r\nConnection: close\r\nContent-Length: %d\r\n\r\n";
  case 500: return "HTTP/1.1 500 Internal Server Error\r\nConnection: close\r\nContent-Length: %d\r\n\r\n";
  case 501: return "HTTP/1.1 501 Not Implemented\r\nConnection: close\r\nContent-Length: %d\r\n\r\n";
  case 502: return "HTTP/1.1 502 Bad Gateway\r\nConnection: close\r\nContent-Length: %d\r\n\r\n";
  case 503: return "HTTP/1.1 503 Service Unavailable\r\nConnection: close\r\nContent-Length: %d\r\n\r\n";
  case 504: return "HTTP/1.1 504 Gateway Timeout\r\nConnection: close\r\nContent-Length: %d\r\n\r\n";
  case 505: return "HTTP/1.1 505 HTTP Version Not Supported\r\nConnection: close\r\nContent-Length: %d\r\n\r\n";
  }
  return "";
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
