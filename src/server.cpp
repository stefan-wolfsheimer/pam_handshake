#include "server.h"
#include "connection.h"
#include "http_header.h"
#include "session.h"
#include <iostream>
#include <functional>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace PamHandshake;

InetAddr::InetAddr(const std::string & _ip,
                   uint16_t _port) : ip(_ip), port(_port)
{
}

UnixDomainAddr::UnixDomainAddr(const std::string & _addr) : addr(_addr)
{
}

Server::Server(const InetAddr & inetaddr,
               std::size_t connection_pool_size,
               bool _verbose)
    : ip(inetaddr.ip),
      port(inetaddr.port),
      verbose(_verbose),
      running(false),
      gen(rd()),
      maxConnections(connection_pool_size)

{
  init();
  socklen_t len;

  // socket create and verification 
  sockfd = socket(AF_INET, SOCK_STREAM, 0); 
  if (sockfd == -1)
  {
    throw std::runtime_error("socket creation failed");
  }
  else
  {
    if(verbose)
    {
      std::cout << "Socket successfully created" << std::endl;
    }
  }
  servaddr = new sockaddr_in;
  bzero(servaddr, sizeof(struct sockaddr_in)); 
  
  // assign IP, PORT 
  servaddr->sin_family = AF_INET;
  if(ip == "0.0.0.0")
  {
    servaddr->sin_addr.s_addr = htonl(INADDR_ANY);
  }
  else
  {
    servaddr->sin_addr.s_addr = inet_addr(ip.c_str());
  }
  servaddr->sin_port = htons(port);
  int option = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
  if ((::bind(sockfd, (struct sockaddr*)servaddr, sizeof(sockaddr_in))) != 0)
  {
    throw std::runtime_error("socket bind failed");
  }
  else
  {
    if(verbose)
    {
      std::cout << "socket successfully bound" << std::endl;
    }
  }
}

Server::Server(const UnixDomainAddr & uda,
               std::size_t connection_pool_size,
               bool _verbose)
  : socketFile(uda.addr),
    port(0),
    verbose(_verbose),
    running(false),
    gen(rd()),
    maxConnections(connection_pool_size)
{
  init();

  // socket create and verification
  sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
  if (sockfd == -1)
  {
    throw std::runtime_error("socket creation failed");
  }
  else
  {
    if(verbose)
    {
      std::cout << "Socket successfully created" << std::endl;
    }
  }
  sockaddr = new sockaddr_un;
  bzero(sockaddr, sizeof(struct sockaddr_un));
  sockaddr->sun_family = AF_UNIX;
  strncpy(sockaddr->sun_path,
          socketFile.c_str(),
          sizeof(sockaddr->sun_path)-1);
  unlink(socketFile.c_str());

  int option = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

  if ((::bind(sockfd, (struct sockaddr*)this->sockaddr, sizeof(sockaddr_un))) != 0)
  {
    throw std::runtime_error("socket bind failed");
  }
  else
  {
    if(verbose)
    {
      std::cout << "socket successfully bound" << std::endl;
    }
  }
}

Server::~Server()
{
  shutdown(sockfd, 2);
  if(servaddr)
  {
    delete servaddr;
  }
  if(sockaddr)
  {
    delete sockaddr;
  }
}

void Server::init()
{
  readTimeout.tv_sec = 0;
  readTimeout.tv_usec = 10000000; // 10 seconds
  writeTimeout.tv_sec = 0;
  writeTimeout.tv_usec = 10000000;
  servaddr = nullptr;
  sockaddr = nullptr;
}

bool Server::isVerbose() const
{
  return verbose;
}

bool Server::isUnixDomainSocket() const
{
  return (sockaddr != nullptr);
}

void Server::run()
{
  // Now server is ready to listen and verification
  if ((listen(sockfd, 5)) != 0)
  {
    throw std::runtime_error("Listen failed");
  }
  else
  {
    if(verbose)
    {
      std::cout << "Server listening" << std::endl;
    }
  }
  // Ignore SIGCHLD to avoid zombie threads
  signal(SIGCHLD, SIG_IGN);

  running = true;
  while(running)
  {
    struct sockaddr_in cli_addr;
    socklen_t clilen = sizeof(cli_addr);
    int connfd = accept(sockfd, 
                        (struct sockaddr *) &cli_addr, 
                        &clilen);
    if (connfd < 0)
    {
      throw std::runtime_error("ERROR on accept");
    }
    if(connections.size() >= maxConnections)
    {
      shutdown(connfd, SHUT_RDWR);
      close(connfd);
    }
    else
    {
      std::size_t connection_id = connections.empty() ? 0 : (connections.crbegin()->first + 1);
      if(verbose)
      {
        {
          std::lock_guard<std::mutex> lock(mutex);
          std::cout << "new connection " << connection_id << std::endl;
        }
      }
      auto p = connections.insert(std::make_pair(connection_id,
                                                 std::thread(std::bind(&Server::handle, this, std::placeholders::_1),
                                                             new Connection(connfd, connection_id, this))));
      p.first->second.detach();
    }
  }
  for(auto & p : connections)
  {
    if(p.second.joinable())
    {
      p.second.join();
    }
  }
}


void Server::handle(Connection * conn)
{
#define BUFFSIZE 1025
  // @todo using static buffer (pooled connection)
  char buff[BUFFSIZE]; 
  try
  {
    std::size_t l = conn->read(buff, BUFFSIZE);
    if(l >= BUFFSIZE)
    {
      const char * msg = HttpHeader::response(413);
      conn->write(msg, strlen(msg));
    }
    else
    {
      auto header = std::make_shared<HttpHeader>(buff, l);
      if(verbose)
      {
        {
          std::lock_guard<std::mutex> lock(mutex);
          std::cout << *header << std::endl;
        }
      }
      if(header->method == "POST")
      {
        std::string token(std::move(createSession()));
        sprintf(buff, "%s%s",
                HttpHeader::response(200),
                token.c_str());
        conn->write(buff);
      }
      else if(header->method == "GET")
      {
        get(conn, header);
      }
      else
      {
        conn->write(HttpHeader::response(405));
      }
    }
  }
  catch(ConnectionTimeout & ex)
  {
    {
      std::lock_guard<std::mutex> lock(mutex);
      std::cerr << ex.what() << std::endl;
    }
    try
    {
      conn->write(HttpHeader::response(504));
    }
    catch(std::exception & ex)
    {
      {
        std::lock_guard<std::mutex> lock(mutex);
        std::cerr << ex.what() << std::endl;
      }
    }
  }
  catch(std::exception & ex)
  {
    {
      std::lock_guard<std::mutex> lock(mutex);
      std::cerr << ex.what() << std::endl;
    }
  }
  std::size_t connectionId = conn->getConnectionId();
  delete conn;
  {
    std::lock_guard<std::mutex> lock(mutex);
    connections.erase(connectionId);
  }
#undef BUFFSIZE
}

std::string Server::createSession()
{
  auto token = randomString(32);
  {
    std::lock_guard<std::mutex> lock(mutex);
    auto session = std::make_shared<Session>(this);
    sessions.insert(std::make_pair(token, session));
  }
  return token;
}

void Server::get(Connection * conn,
                             std::shared_ptr<const HttpHeader> header)
{
  std::string token = std::string(header->uri.begin() + 1,
                                  header->uri.end());
  {
    std::lock_guard<std::mutex> lock(mutex);
    auto itr = sessions.find(token);
    if(itr != sessions.end())
    {
      auto p = itr->second->pull(header->content, header->payload_size);
      int http_code = 200;
      if(p.first == Session::State::NotAuthenticated)
      {
        http_code = 401;
      }
      else if(p.first == Session::State::Authenticated)
      {
        http_code = 202;
      }
      std::string msg(HttpHeader::response(200));
      msg.append(Session::StateToString(p.first));
      msg.append("\r\n");
      msg.append(p.second);
      conn->write(msg.c_str());
    }
    else
    {
      conn->write(HttpHeader::response(404));
    }
  }
}

std::string Server::randomString(std::size_t len)
{
  std::uniform_int_distribution<> dis(0, 15);
  std::string str;
  std::size_t i = 0;
  int val;
  str.reserve(len);
  for(std::size_t j = 0; j < len; j++)
  {
    {
      std::lock_guard<std::mutex> lock(mutex);
      val = dis(gen);
    }
    str.push_back((char)(val < 10 ? ('0' + val) : ('a' + (val-10))));
  }
  return str;
}
