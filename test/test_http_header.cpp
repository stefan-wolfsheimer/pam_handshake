#include <catch2/catch.hpp>
#include <iostream>
#include "http_header.h"

TEST_CASE("http_header_fixed_length", "[HTTP]")
{
    const char * msg =
    "PUT / HTTP/1.1\r\n"
    "Host: localhost:8080\r\n"
    "User-Agent: curl/7.58.0\r\n"
    "Content-Length: 3\r\n"
    "Accept: */*\r\n"
    "X-MyHeader: 123\r\n\r\n"
    "content\r\ntest";
  PamHandshake::HttpHeader header(msg);
  REQUIRE(header.method == "PUT");
  REQUIRE(header.uri == "/");
  REQUIRE(header.proto == "HTTP/1.1");
  auto itr =  header.values.find("X-MyHeader");
  REQUIRE(itr != header.values.end());
  REQUIRE(itr->second == "123");
  REQUIRE(std::string(header.content, header.content + header.payload_size) == "con");
}

TEST_CASE("http_header_variable_length", "[HTTP]")
{
    const char * msg =
    "PUT / HTTP/1.1\r\n"
    "Host: localhost:8080\r\n"
    "User-Agent: curl/7.58.0\r\n"
    "Accept: */*\r\n"
    "X-MyHeader: 123\r\n\r\n"
    "content\r\ntest";
  PamHandshake::HttpHeader header(msg);
  REQUIRE(header.method == "PUT");
  REQUIRE(header.uri == "/");
  REQUIRE(header.proto == "HTTP/1.1");
  auto itr =  header.values.find("X-MyHeader");
  REQUIRE(itr != header.values.end());
  REQUIRE(itr->second == "123");
  REQUIRE(std::string(header.content, header.content + header.payload_size) == "content\r\ntest");
}

