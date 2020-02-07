#include <stdexcept>
#include <iostream>
#include <sstream>
#include <string.h>
#include <security/pam_appl.h>
#include "ipam_client.h"
#include "pam_conversation.h"

int PamHandshake::pam_conversation(int n,
                                   const struct pam_message **msg,
                                   struct pam_response **resp,
                                   void *data)
{
  auto pamClient = static_cast<IPamClient*>(data);
  if(!pamClient || pamClient->canceled())
  {
    return PAM_CONV_ERR;
  }
  struct pam_response *aresp;
  if (n <= 0 || n > PAM_MAX_NUM_MSG)
  {
    return PAM_CONV_ERR;
  }
  if ((aresp = (pam_response*)calloc(n, sizeof *aresp)) == NULL)
  {
    return PAM_BUF_ERR;
  }
  try
  {
    for(int i = 0; i < n; ++i)
    {
      aresp[i].resp_retcode = 0;
      aresp[i].resp = NULL;
      switch (msg[i]->msg_style)
      {
      case PAM_PROMPT_ECHO_OFF:
        pamClient->promptEchoOff(msg[i]->msg, &aresp[i]);
        break;
      case PAM_PROMPT_ECHO_ON:
        pamClient->promptEchoOn(msg[i]->msg, &aresp[i]);
        break;
      case PAM_ERROR_MSG:
        pamClient->errorMsg(msg[i]->msg);
        break;
      case PAM_TEXT_INFO:
        pamClient->textInfo(msg[i]->msg);
        break;
      default:
        throw std::invalid_argument(std::string("invalid PAM message style ") +
                                    std::to_string(msg[i]->msg_style));
      }
    }
  }
  catch(std::exception & ex)
  {
    // cleanup
    for (int i = 0; i < n; ++i)
    {
      if (aresp[i].resp != NULL)
      {
        memset(aresp[i].resp, 0, strlen(aresp[i].resp));
        free(aresp[i].resp);
      }
    }
    memset(aresp, 0, n * sizeof(*aresp));
    *resp = NULL;
    return PAM_CONV_ERR;
  }
  *resp = aresp;
  if(pamClient->canceled())
  {
    return PAM_CONV_ERR;
  }
  return PAM_SUCCESS;
}

bool PamHandshake::pam_auth_check(const std::string & pam_service,
                                  PamHandshake::IPamClient & client,
                                  bool verbose)
{
  pam_handle_t *pamh = nullptr;
  pam_conv conv = { PamHandshake::pam_conversation, &client };
  const int retval_pam_start = pam_start(pam_service.c_str(),
                                         nullptr,
                                         &conv,
                                         &pamh );
  if(verbose)
  {
    std::cout << "pam_start: " << retval_pam_start << std::endl;
  }
  if(retval_pam_start != PAM_SUCCESS)
  {
    throw std::runtime_error("pam_start_error");
  }
  const int retval_pam_authenticate = pam_authenticate( pamh, 0 );
  if(verbose)
  {
    if(retval_pam_authenticate == PAM_SUCCESS)
    {
      std::cout << "pam_authenticate: PAM_SUCCESS" << std::endl;
    }
    else
    {
      std::cout << "pam_authenticate: " << retval_pam_authenticate
                << ": "
                << pam_strerror(pamh, retval_pam_authenticate)
                << std::endl;
    }
  }
  if(pam_end( pamh, retval_pam_authenticate ) != PAM_SUCCESS)
  {
    pamh = NULL;
    throw std::runtime_error("irodsPamAuthCheck: failed to release authenticator");
  }
  if(retval_pam_authenticate != PAM_AUTH_ERR &&
     retval_pam_authenticate != PAM_SUCCESS)
  {
    std::stringstream ss;
    ss << "pam_authenticate: " << retval_pam_authenticate
       << ": "
       << pam_strerror(pamh, retval_pam_authenticate)
       << std::endl;
    throw std::runtime_error(ss.str());
  }
  // indicate success (valid username and password) or not
  return retval_pam_authenticate == PAM_SUCCESS;
}

