#ifndef ONBOARDING_UTILITIES_H
#define ONBOARDING_UTILITIES_H

#include <stdbool.h>
#include <stdint.h>
typedef struct {
  char *serial_number;
  char *aws_ip_address;
  int port_number;
  bool update_required;
} onboarding_details_t;

onboarding_details_t *get_onboarding_details_info();

#endif