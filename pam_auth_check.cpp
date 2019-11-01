#include <string>
#include <iostream>
#include <string.h>
#include <unistd.h>
#include "pam_conversation.h"

class PamLocalClient : public IPamClient
{
public:
  virtual void promptEchoOn(const char * msg, pam_response_t * resp) override
  {
    std::string buf;
    std::cout << msg << std::flush;
    std::getline(std::cin, buf);
    if(buf.size() + 1 >  PAM_MAX_RESP_SIZE)
    {
      throw std::range_error(std::string("Answer too long (") +
                             std::to_string(buf.size()));
    }
    resp->resp = strdup(buf.c_str());
    if(!resp->resp)
    {
      throw std::bad_alloc();
    }
  }

  virtual void promptEchoOff(const char * msg, pam_response_t * resp)  override
  {
    resp->resp = strdup(getpass(msg));
    if(!resp->resp)
    {
      throw std::bad_alloc();
    }
  }

  virtual void errorMsg(const char * msg)  override
  {
    std::cout << msg << std::endl;
  }

  virtual void textInfo(const char * msg) override
  {
    std::cout << msg << std::endl;
  }
};

int main(int argc, const char ** argv)
{
  std::string pam_service = "default";
  bool printHelp = false;
  bool argError = false;
  bool verbose = false;
  for(int i = 0; i < argc; ++i)
  {
    std::string arg(argv[i]);
    if(arg == "--service" || arg == "-s")
    {
      ++i;
      if(i < argc)
      {
        pam_service = std::string(argv[i]);
      }
      else
      {
        std::cerr << "missing argument " << argv[i-1] << " SERVICE" << std::endl;
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
  if(printHelp || argError)
  {
    std::cout << argv[0] << "[OPTIONS]" << std::endl;
    std::cout << "OPTIONS:" << std::endl;
    std::cout << "--service|-s SERVICE" << std::endl;
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
  PamLocalClient client;
  bool ret = pam_auth_check(pam_service, client, verbose);
  if(ret)
  {
    return 0;
  }
  else
  {
    std::cerr << "authentication failed" << std::endl;
    return 8;
  }
}
