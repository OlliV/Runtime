/*
 * Copyright (c) 2014 ELL-i co-operative.
 *
 * This file is part of ELL-i software.
 *
 * ELL-i software is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ELL-i software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ELL-i software.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * Minimal standalone UDP
 *
 * @author: Pekka Nikander <pekka.nikander@ell-i.org>  2014
 */

#ifndef  _ETHERNET_UDP_H
# define _ETHERNET_UDP_H

# include <system_init.h>

struct udp {
    uint16_t udp_sport;
    uint16_t udp_dport;
    uint16_t udp_len;
    uint16_t udp_sum;
    uint8_t  udp_data[0];
};

/**
 * XXX
 */

#ifndef LINKER_ALIGNMENT
#define LINKER_ALIGNMENT 16
#endif

struct udp_socket {
    void (*udp_socket_handler)(void *enclosing, uint8_t data[], uint16_t len);
    const uint16_t udp_port;
    /* XXX Add pointer to statistics, in RAM */
} __attribute__((aligned(LINKER_ALIGNMENT)));

# ifdef __MACH__
#  define UDP_SOCKET_SECTION(port)  ".text,.socket"
# else
#  define UDP_SOCKET_SECTION(port)  ".socket.udp." # port
# endif

# define __EXPAND_SOCKET_NAME(port) __udp_socket ## port

# define DECLARE_UDP_SOCKET(port)                              \
    extern const struct udp_socket  __EXPAND_SOCKET_NAME(port) \
    __attribute__((section(UDP_SOCKET_SECTION(port)))) 

# define DEFINE_UDP_SOCKET(port, handler)                  \
    DECLARE_UDP_SOCKET(port);                              \
    const struct udp_socket __EXPAND_SOCKET_NAME(port)     \
        __attribute__((section(UDP_SOCKET_SECTION(port)))) \
        = {                                                \
        IF(udp_socket_handler) handler,                    \
        IF(udp_port) port,                                 \
    }

extern const struct udp_socket __attribute__((section(UDP_SOCKET_SECTION()))) __udp_sockets[];
extern const struct udp_socket __attribute__((section(UDP_SOCKET_SECTION()))) __udp_sockets_end[];


# ifdef __cplusplus
extern "C" {
# endif

/**
 * XXX
 *
 * The pointed packet MUST be a pointer to inside an Ethernet packet,
 * containing valid IP and Ethernet frames.
 */
extern void udp_output(struct udp *const udp_packet);

/**
 * XXX
 */
extern void udp_input(struct udp *const udp_packet);

# ifdef __cplusplus
}
# endif

#endif //_ETHERNET_UDP_H
