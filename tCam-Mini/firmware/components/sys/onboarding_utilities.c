#include "onboarding_utilities.h"

#include "ps_utilities.h"

static char aws_ip_adress[PS_IP_ADDRESS_MAX_LENGTH + 1];
static char serial_number[PS_PW_MAX_LEN + 1];

static onboarding_details_t onboarding_details_info = {serial_number,
                                                       aws_ip_adress, 0, false};

onboarding_details_t *get_onboarding_details_info() {
  return &onboarding_details_info;
}