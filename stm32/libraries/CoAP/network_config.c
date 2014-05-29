#include <ethernet.h>
#include <ip.h>

uint8_t ether_local_address[ETHER_ADDR_LEN] = { 0, 0, 0, 0xff, 0xff, 0 }; // XXX better default?
struct in_addr ip_local_address = { .s_bytes = { 10, 0, 0, 2 } };
