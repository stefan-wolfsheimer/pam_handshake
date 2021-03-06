#pragma once
#include <memory>
#include <mutex>
#include <random>
#include <thread>
#include <map>
#include <vector>
#include <set>

struct sockaddr_in;
struct sockaddr_un;

namespace PamHandshake
{
  class HttpHeader;
  class Session;
  class Connection;

  class InetAddr
  {
  public:
    friend class Server;
    InetAddr(const std::string & _ip = "0.0.0.0",
             uint16_t _port=8080);
  private:
    std::string ip;
    uint16_t port;
  };

  class UnixDomainAddr
  {
  public:
    friend class Server;
    UnixDomainAddr(const std::string & _addr,
                   const std::string & _chgrp="");
  private:
    std::string addr;
    std::string chgrp;
  };

  /**
   * Server that is listening on port or unix domain socket.
   */
  class Server : public std::enable_shared_from_this<Server>
  {
  public:
    friend class Connection;

    /**
     * Construct server listening on port
     *
     * \param inetaddr
     * \param pam_stack_name       name of the PAM stack in /etc/pam.d
     * \param connection_pool_size numnber of simultanious connections
     * \param connection_timeout   connection timeout (in milliseconds)
     * \param session_timeout      lifetime on interactive PAM conversation session
     * \param verbose              verbose server log
     *
     */
    Server(const InetAddr & inetaddr,
           const std::string & pam_stack_name,
           std::size_t connection_pool_size=10,
           std::size_t connection_timeout=10000, // milliseconds
           std::size_t session_timeout=3600, // seconds
           bool _verbose=false);

    /**
     * Construct server listening on unix domain socket
     *
     */
    Server(const UnixDomainAddr & uda,
           const std::string & pam_stack_name,
           std::size_t connection_pool_size=10,
           std::size_t connection_timeout=10000, // milliseconds
           std::size_t session_timeout=3600, // seconds
           bool _verbose=false);
    ~Server();

    void setConversationProgram(const std::string & exe);
    std::string getConversationProgram() const;
    bool hasConversationProgram() const;

    /**
     * \param return true if server is in verbose mode
     */
    bool isVerbose() const;

    /**
     * \param return true if server runs on unix domain socket
     */
    bool isUnixDomainSocket() const;

    /**
     * \return name of the PAM stack in /etc/pam.d
     */
    const std::string getPamStackName() const;

    /**
     * Start the server
     */
    void run();
    void handle(std::shared_ptr<Connection> conn);

    std::string createSession();

    /**
     * Give answer to active PAM conversation
     */
    void post(Connection * conn,
              std::shared_ptr<const HttpHeader> header);
    void put(Connection * conn,
             std::shared_ptr<const HttpHeader> header);
    void get(Connection * conn,
             std::shared_ptr<const HttpHeader> header);
    void deleteSession(Connection * conn,
                       std::shared_ptr<const HttpHeader> header);
    std::string randomString(std::size_t len);
    void response(Connection * conn,
                  int code,
                  const std::string & content);
    void response_not_found(Connection * conn,
                            const std::string & content);
    void start_housekeeping();
  private:
    void init();
    void housekeeping();
    mutable std::mutex mutex;
    std::string pam_stack_name;
    std::string ip;
    std::string socketFile;
    std::string socketFileChgrp;
    uint16_t port;
    struct ::sockaddr_in * servaddr;
    struct ::sockaddr_un * sockaddr;
    bool verbose;
    bool running;
    int sockfd;
    std::random_device rd;
    std::mt19937 gen;

    // conversation program
    std::string conversationProgram;

    struct timeval readTimeout;
    struct timeval writeTimeout;

    std::set<std::size_t> connections;
    std::size_t maxConnections;
    std::size_t connectionTimeout;
    std::size_t sessionTimeout;

    std::map<std::string, std::shared_ptr<Session>> sessions;
    std::vector<std::thread> threads;
    std::shared_ptr<std::thread> housekeeper;
  };

}
