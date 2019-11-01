#pragma once
#include <security/pam_appl.h>

int pam_conversation(int n,
                     const struct pam_message **msg,
                     struct pam_response **resp,
                     void *data);
