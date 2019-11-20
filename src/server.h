#pragma once
#include <memory>
#include <mutex>
#include <random>
#include <thread>
#include <map>
#include <vector>

struct sockaddr_in;

namespace PamHandshake
{
  class HttpHeader;
  class Session;
  class Connection;

  class Server
  {
  public:
    friend class Connection;
    Server(const std::string & _ip = "0.0.0.0",
           uint16_t _port=8080,
           std::size_t connection_pool_size=10,
           bool _verbose=false);
    bool isVerbose() const;
    void run();
    void handle(Connection * conn);
    std::string createSession();
    void get(Connection * conn,
             std::shared_ptr<const HttpHeader> header);
    std::string randomString(std::size_t len);
  private:
    void bind();
    mutable std::mutex mutex;
    std::string ip;
    uint16_t port;
    struct ::sockaddr_in * servaddr;
    bool verbose;
    bool running;
    int sockfd;
    std::random_device rd;
    std::mt19937 gen;

    struct timeval readTimeout;
    struct timeval writeTimeout;

    std::map<std::size_t, std::thread> connections;
    std::size_t maxConnections;

    std::map<std::string, std::shared_ptr<Session>> sessions;
    std::vector<std::thread> threads;
  };

}
