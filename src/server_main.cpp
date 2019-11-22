#include "server.h"
#include <string>
#include <exception>
#include <iostream>

int main(int argc, const char ** argv)
{
  uint16_t port = 8080;
  std::string addr = "0.0.0.0";
  bool printHelp = false;
  bool argError = false;
  bool verbose = false;
  bool unixSocket = false;
  bool addrGiven = false;
  for(int i = 0; i < argc; ++i)
  {
    std::string arg(argv[i]);
    if(arg == "--port" || arg == "-p")
    {
      ++i;
      if(i < argc)
      {
        try
        {
          port = stoi(std::string(argv[i]));
        }
        catch(std::exception ex)
        {
          std::cerr << "invalid port number " << argv[i] << std::endl;
          argError = true;
        }
      }
      else
      {
        std::cerr << "missing argument " << argv[i-1] << " PORT" << std::endl;
        argError = true;
      }
    }
    if(arg == "--socket" || arg == "-s")
    {
      unixSocket = true;
    }
    if(arg == "--addr" || arg == "-a")
    {
      ++i;
      if(i < argc)
      {
        addr = argv[i];
        addrGiven = true;
      }
      else
      {
        std::cerr << "missing argument " << argv[i-1] << " IPv4" << std::endl;
        argError = true;
      }
    }
    if(arg == "--help" || arg == "-h")
    {
      printHelp = true;
    }
    if(arg == "--verbose" || arg == "-v")
    {
      verbose = true;
    }
  }
  if(unixSocket && !addrGiven)
  {
    std::cerr << "option --socket requires --addr arguemnt" << std::endl;
    argError = true;
  }
  if(printHelp || argError)
  {
    std::cout << argv[0] << "[OPTIONS]" << std::endl;
    std::cout << "OPTIONS:" << std::endl;
    std::cout << "--port|-p PORT" << std::endl;
    std::cout << "--addr|-a IPv4" << std::endl;
    std::cout << "--socket|-s" << std::endl;
    std::cout << "--verbose|-v" << std::endl;
    std::cout << "--help|-h" << std::endl;
    if(argError)
    {
      return 1;
    }
    else
    {
      return 0;
    }
  }
  try
  {
    std::shared_ptr<PamHandshake::Server> server;

    if(unixSocket)
    {
      PamHandshake::UnixDomainAddr socketAddr(addr);
      server = std::make_shared<PamHandshake::Server>(socketAddr, 10, verbose);
    }
    else
    {
      PamHandshake::InetAddr socketAddr(addr, port);
      server = std::make_shared<PamHandshake::Server>(socketAddr, 10, verbose);
    }
    server->run();
  }
  catch(const std::exception & ex)
  {
    std::cerr << "server failed:" << ex.what() << std::endl;
    return 8;
  }
  return 0;
}
