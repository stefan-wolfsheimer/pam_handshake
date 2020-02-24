/***********************************************************
 * Auxilarry stateful server for PAM stack conversation
 *
 * ./server [OPTIONS]
 * OPTIONS:
 * --port|-p PORT
 * --addr|-a IPv4
 * --connection_pool_size N (default: 10)
 * --connection_timeout MILLISECONDS (default: 10000)
 * --session_timeout SECONDS (default: 3600)
 * --socket|-s
 * --verbose|-v
 * --help|-h
 *
 **/
#include "server.h"
#include <string>
#include <exception>
#include <iostream>

/**
 * Parse an integer value from argv[i]
 *
 * \param argc total number of arguments
 * \param const char ** argv argument values
 * \param int & i current possition. (the value is incremented by 1
 * \param bool & argError set to true if an error occurred
 * \return the value parsed from argv[i]
 */
static int parseInt(int argc, const char ** argv, int & i, bool & argError)
{
  int ret = 0;
  ++i;
  if(i < argc)
  {
    try
    {
      ret = stoi(std::string(argv[i]));
    }
    catch(std::exception ex)
    {
      std::cerr << "invalid number " << argv[i] << std::endl;
      argError = true;
    }
  }
  else
  {
    std::cerr << "missing argument " << argv[i-1] << " N" << std::endl;
    argError = true;
  }
  return ret;
}

/**
 * Parse a string value from argv[i]
 *
 * \param argc total number of arguments
 * \param const char ** argv argument values
 * \param int & i current possition. (the value is incremented by 1
 * \param bool & argError set to true if an error occurred
 * \return the value parsed from argv[i]
 */
static std::string parseString(int argc, const char ** argv, int & i, bool & argError)
{
  int ret = 0;
  ++i;
  if(i < argc)
  {
    return std::string(argv[i]);
  }
  else
  {
    std::cerr << "missing argument " << argv[i-1] << " N" << std::endl;
    argError = true;
  }
  return std::string("");
}

/**
 * Server's main function.
 * Parse arguments from argv and run server.
 */
int main(int argc, const char ** argv)
{
  /* set default arguments */
  uint16_t port = 8080;
  std::size_t connectionPoolSize = 10;
  std::size_t connectionTimeout = 10000; //milliseconds
  std::size_t sessionTimeout = 3600; // seconds
  std::string addr = "0.0.0.0";
  std::string pamStackName = "irods";
  std::string conversationProgram;
  bool printHelp = false;
  bool argError = false;
  bool verbose = false;
  bool unixSocket = false;
  bool addrGiven = false;

  /* parse and validate arguments  */
  for(int i = 1; i < argc; ++i)
  {
    std::string arg(argv[i]);
    if(arg == "--port" || arg == "-p")
    {
      port = parseInt(argc, argv, i, argError);
    }
    else if(arg == "--socket" || arg == "-s")
    {
      unixSocket = true;
    }
    else if(arg == "--addr" || arg == "-a")
    {
      bool addrError = false;
      addr = parseString(argc, argv, i, addrError);
      if(!addrError)
      {
        addrGiven = true;
      }
      else
      {
        argError = true;
      }
    }
    else if(arg == "--connection_pool_size")
    {
      connectionPoolSize = parseInt(argc, argv, i, argError);
    }
    else if(arg == "--connection_timeout")
    {
      connectionTimeout = parseInt(argc, argv, i, argError);
    }
    else if(arg == "--session_timeout")
    {
      sessionTimeout = parseInt(argc, argv, i, argError);
    }
    else if(arg == "--stack")
    {
      pamStackName = parseString(argc, argv, i, argError);
    }
    else if(arg == "--help" || arg == "-h")
    {
      printHelp = true;
    }
    else if(arg == "--verbose" || arg == "-v")
    {
      verbose = true;
    }
    else if(arg == "--conversation")
    {
      conversationProgram = parseString(argc, argv, i, argError);
    }
    else
    {
      std::cerr << "invalid argument: " << arg << std::endl;
      argError = true;
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
    std::cout << "--connection_pool_size N (default: 10)" << std::endl;
    std::cout << "--connection_timeout MILLISECONDS (default: 10000)" << std::endl;
    std::cout << "--session_timeout SECONDS (default: 3600)" << std::endl;
    std::cout << "--socket|-s" << std::endl;
    std::cout << "--stack PAM_STACK_NAME" << std::endl;
    std::cout << "--conversation CONV_BINARY" << std::endl;
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

  /* run server  */
  try
  {
    std::shared_ptr<PamHandshake::Server> server;

    if(unixSocket)
    {
      PamHandshake::UnixDomainAddr socketAddr(addr);
      server = std::make_shared<PamHandshake::Server>(socketAddr,
                                                      pamStackName,
                                                      connectionPoolSize,
                                                      connectionTimeout,
                                                      sessionTimeout,
                                                      verbose);
    }
    else
    {
      PamHandshake::InetAddr socketAddr(addr, port);
      server = std::make_shared<PamHandshake::Server>(socketAddr,
                                                      pamStackName,
                                                      connectionPoolSize,
                                                      connectionTimeout,
                                                      sessionTimeout,
                                                      verbose);
    }
    if(!conversationProgram.empty())
    {
      server->setConversationProgram(conversationProgram);
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
