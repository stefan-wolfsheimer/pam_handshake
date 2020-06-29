#pragma once
#include <memory>
#include <exception>
#include <stdexcept>
#include <sys/poll.h>

namespace PamHandshake
{
  class Server;

  class ConnectionTimeout : public ::std::runtime_error {
  public:
    using ::std::runtime_error::runtime_error;
  };

  class Connection
  {
  public:
    Connection(int _connfd, std::size_t _connectionId,
               std::shared_ptr<Server> _server);
    ~Connection();
    std::shared_ptr<Server> getServer() const;
    std::size_t getConnectionId() const;
    int read(char * buff, size_t len);
    void write(const char * buff, size_t len);
    void write(const std::string & buff);

    uint64_t read_uint64();
    uint8_t read_uint8();

    void write_uint64(const uint64_t & value);
    void write_uint8(const uint8_t & value);

  private:
    int connfd;
    std::size_t connectionId;
    std::shared_ptr<Server> server;
  };
}
