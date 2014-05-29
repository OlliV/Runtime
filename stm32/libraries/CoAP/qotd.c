#include <ip.h>
#include <udp.h>

static char qotd[] =
    "A useful debugging and measurement tool is a quote of the day service.\n"
    "A quote of the day service simply sends a short message without regard\n"
    "to the input.";

void udp_qotd(uint8_t udp_payload[], uint16_t udp_payload_len)
{
    memcpy(udp_payload, qotd, sizeof(qotd));
    udp_output(udp_payload, sizeof(qotd));
}

DEFINE_UDP_SOCKET(UDP_PORT_QOTD, udp_qotd);
