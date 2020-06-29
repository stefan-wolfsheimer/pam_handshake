#include "connection.h"
#include "server.h"
#include <unistd.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>

using namespace PamHandshake;

Connection::Connection(int _connfd,
                       std::size_t _connectionId,
                       std::shared_ptr<Server> _server)
  : connfd(_connfd),
    connectionId(_connectionId),
    server(_server)
{
}

Connection::~Connection()
{
  shutdown(connfd, SHUT_RDWR);
  close(connfd);
}

std::shared_ptr<Server> Connection::getServer() const
{
  return server;
}

std::size_t Connection::getConnectionId() const
{
  return connectionId;
}

int Connection::read(char * buff, size_t len)
{
  fd_set set;
  struct timeval timeout = server->readTimeout;
  FD_ZERO(&set);
  FD_SET(connfd, &set);
  int rv = select(connfd + 1, &set, NULL, NULL, &timeout);
  if(rv == -1)
  {
    char buff[64];
    strerror_r(rv, buff, 63);
    std::cout << "select for read failed:" << buff << std::endl;
    throw std::runtime_error(buff);
  }
  else if(rv == 0)
  {
    throw ConnectionTimeout("timeout");
  }
  else
  {
    return ::read(connfd, buff, len);
  }
}

void Connection::write(const char * buff, size_t len)
{
  fd_set set;
  struct timeval timeout = server->writeTimeout;
  FD_ZERO(&set);
  FD_SET(connfd, &set);
  int rv = select(connfd + 1, nullptr, &set, nullptr, &timeout);
  if(rv == -1)
  {
    char buff[64];
    strerror_r(rv, buff, 63);
    std::cout << "select for write failed:" << buff << std::endl;
    throw std::runtime_error(buff);
  }
  else if(rv == 0)
  {
    throw ConnectionTimeout("timeout");
  }
  else
  {
    ::write(connfd, buff, len);
  }
}

void Connection::write(const std::string & buff)
{
  write(buff.c_str(), buff.size());
}

uint64_t Connection::read_uint64()
{
  char buff[8];
  this->read(buff, 8);
  uint64_t res = 0;
  for(int i = 0; i < 8; i++)
  {
    res += uint64_t(buff[i]) << 8*(7-i);
  }
  return res;
}

uint8_t Connection::read_uint8()
{
  char buff;
  this->read(&buff, 1);
  return (uint8_t)buff;
}

void Connection::write_uint64(const uint64_t & value)
{
  char buff[8];
  for(int i = 0; i < 8; i++)
  {
    buff[i] = uint8_t((value >> 8*(7 - i)) & 0xFF);
  }
  this->write(buff, 8);
}

void Connection::write_uint8(const uint8_t & value)
{
  this->write((const char*)&value, 1);
}

