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
#include "pam_conversation.h"
#include <string>
#include <exception>
#include <iostream>

class PamClient : public ::PamHandshake::IPamClient
{
public:
  virtual void promptEchoOn(const char * msg, pam_response_t * resp) override
  {
    std::cout << "promptEchoOn \"" << msg << "\"" << std::endl;
  }

  virtual void promptEchoOff(const char * msg, pam_response_t * resp) override
  {
    std::cout << "promptEchoOff \"" << msg << "\"" << std::endl;
  }

  virtual void errorMsg(const char * msg) override
  {
    std::cout << "errorMsg \"" << msg << "\"" << std::endl;
  }

  virtual void textInfo(const char * msg) override
  {
    std::cout << "textInfo \"" << msg << "\"" << std::endl;
  }

  virtual bool canceled()
  {
    return false;
  }

};


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
 * Execute PAM conversation on command line
 */
int main(int argc, const char ** argv)
{
  /* set default arguments */
  std::string pamStackName = "irods";
  bool printHelp = false;
  bool argError = false;
  bool verbose = false;

  /* parse and validate arguments  */
  for(int i = 0; i < argc; ++i)
  {
    std::string arg(argv[i]);
    if(arg == "--stack")
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
  }
  if(printHelp || argError)
  {
    std::cout << argv[0] << "[OPTIONS]" << std::endl;
    std::cout << "OPTIONS:" << std::endl;
    std::cout << "--stack PAM_STACK_NAME" << std::endl;
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

  /* run PAM conversation  */
  PamClient client;
  bool result = ::PamHandshake::pam_auth_check(pamStackName, client, verbose);
  if(result)
  {
    std::cout << "Authenticated" << std::endl;
  }
  else
  {
    std::cout << "Not Authenticated" << std::endl;
  }
  return 0;
}