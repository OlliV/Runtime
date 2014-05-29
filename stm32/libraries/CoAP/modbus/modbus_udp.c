#include <ip.h>
#include <udp.h>
#include "modbus_udp.h"

/* http://www.modbus.org/docs/Modbus_Application_Protocol_V1_1b3.pdf */

#ifndef num_elem
#define num_elem(x) (sizeof(x) / sizeof(*(x)))
#endif

static const modbus_fn_t modbus_fn[25] = {
    /* Data Access */
    [2]     = 0, /* Read Discrete Inputs */
    [1]     = 0, /* Read Coils */
    [5]     = 0, /* Write Single Coil */
    [15]    = 0, /* Write Multiple Coils */
    [4]     = modbus_read_iregisters, /* Read Input Registers */
    [3]     = 0, /* Read Holding Registers */
    [6]     = 0, /* Write Single Register */
    [16]    = 0, /* Write Multiple Registers */
    [23]    = 0, /* Read/Write Multiple Registers */
    [22]    = 0, /* Mask Write Register */
    [24]    = 0, /* Read FIFO Queue */
    [20]    = 0, /* Read File Record */
    [21]    = 0, /* Write File Record */
    /* Diagnostics */
    [7]     = 0, /* Read Exception Status */
    [8]     = 0, /* Diagnostic */
    [11]    = 0, /* Get Com Event Counter */
    [12]    = 0, /* Get Com Event Log */
    [17]    = 0, /* Report Slave ID */
};

static int ntohframe(uint8_t udp_payload[], uint16_t udp_payload_len)
{
    struct modbus_frame *frame = (struct modbus_frame *)udp_payload;

    frame->trans_id = ntohs(frame->trans_id);
    frame->prot_id = ntohs(frame->prot_id);
    frame->len = ntohs(frame->len);

    if (frame->len < 2)
        return 1; /* Minimum len is 2 */

    return 0;
}

static void htonframe(struct modbus_frame *frame)
{
    frame->trans_id = htons(frame->trans_id);
    frame->prot_id = htons(frame->prot_id);
    frame->len = htons(frame->len);
}

void modbus_udp(uint8_t udp_payload[], uint16_t udp_payload_len)
{
    struct modbus_frame *frame = (struct modbus_frame *)udp_payload;
    struct modbus_error *errmsg = MODBUS_GET_ERR(frame);
    size_t wr = 0; /* Bytes written */

    if (ntohframe(udp_payload, udp_payload_len)) {
        /* Malformed frame */
        wr = modbus_generror(errmsg, MODBUS_EX_ILLFN);
        goto out;
    }

    if (((size_t)frame->fn_code < num_elem(modbus_fn)) && modbus_fn[frame->fn_code]) {
        wr = modbus_fn[frame->fn_code](frame);
        goto out;
    } else  if (frame->fn_code == 43) {
         /* Read Device Identification / Encapsulated Interface Transport */
         /* XXX */
        wr = modbus_generror(errmsg, MODBUS_EX_ILLFN);
    } else {
        wr = modbus_generror(errmsg, MODBUS_EX_ILLFN);
    }

out:
    frame->len = MODBUS_FRAME_LEN(wr);
    udp_payload_len = MODBUS_UDP_DG_LEN(wr);
    htonframe(frame);
    udp_output(udp_payload, udp_payload_len);
}
DEFINE_UDP_SOCKET(UDP_PORT_MODBUS, modbus_udp);

size_t modbus_generror(struct modbus_error *errmsg, int8_t exception)
{
    errmsg->fn_code += 0x80;
    errmsg->exception = exception;

    return 2;
}
