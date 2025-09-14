#include "ipadress.h"
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>

// Get the first non-loopback IPv4 address
void get_ip_address(char *buffer, size_t buflen) {
    struct ifaddrs *ifaddr, *ifa;
    int family;
    buffer[0] = '\0'; // default empty

    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return;
    }

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) continue;

        family = ifa->ifa_addr->sa_family;

        if (family == AF_INET) { // IPv4 only
            char addr[INET_ADDRSTRLEN];
            if (inet_ntop(AF_INET, &((struct sockaddr_in *)ifa->ifa_addr)->sin_addr,
                          addr, sizeof(addr)) != NULL) {
                if (strcmp(ifa->ifa_name, "lo") != 0) { // skip loopback
                    strncpy(buffer, addr, buflen);
                    buffer[buflen - 1] = '\0'; // ensure null-termination
                    break;
                }
            }
        }
    }

    freeifaddrs(ifaddr);
}
