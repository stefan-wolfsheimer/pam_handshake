#pragma once

class IPamClient
{
public:
  typedef struct pam_response pam_response_t;
  virtual ~IPamClient() {};
  virtual void promptEchoOn(const char * msg, pam_response_t * resp) = 0;
  virtual void promptEchoOff(const char * msg, pam_response_t * resp) = 0;
  virtual void errorMsg(const char * msg) = 0;
  virtual void textInfo(const char * msg) = 0; 
};

