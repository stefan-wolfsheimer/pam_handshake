#pragma once
#include <security/pam_appl.h>
#include "ipam_client.h"

int pam_conversation(int n,
                     const struct pam_message **msg,
                     struct pam_response **resp,
                     void *data);

bool pam_auth_check(const std::string & pam_service,
                    IPamClient & client,
                    bool verbose);
