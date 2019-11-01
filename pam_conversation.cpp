#include <stdexcept>
#include <string.h>
#include <security/pam_appl.h>
#include "ipam_client.h"
#include "pam_conversation.h"

int pam_conversation(int n,
                     const struct pam_message **msg,
                     struct pam_response **resp,
                     void *data)
{
  auto pamClient = static_cast<IPamClient*>(data);
  if(!pamClient)
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
  return PAM_SUCCESS;
}

