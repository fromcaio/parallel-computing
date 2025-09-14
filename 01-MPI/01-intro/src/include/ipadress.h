#ifndef IPADRESS_H
#define IPADRESS_H

#include <stddef.h>

// Get the first non-loopback IPv4 address
void get_ip_address(char *buffer, size_t buflen);

#endif // IPADRESS_H
