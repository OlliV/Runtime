#include <ip.h>
#include <udp.h>
#include "modbus_udp.h"

/*
 * Reference:
 * http://www.modbus.org/docs/Modbus_Application_Protocol_V1_1b3.pdf
 * http://jamod.sourceforge.net/kbase/modbus_udp.html
 */

#ifndef num_elem
#define num_elem(x) (sizeof(x) / sizeof(*(x)))
#endif

static const modbus_fn_t modbus_fn[25] = {
    /* Data Access */
    [2]     = 0,                        /* Read Discrete Inputs */
    [1]     = 0,                        /* Read Coils */
    [5]     = 0,                        /* Write Single Coil */
    [15]    = 0,                        /* Write Multiple Coils */
    [4]     = modbus_read_iregisters,   /* Read Input Registers */
    [3]     = 0,                        /* Read Holding Registers */
    [6]     = modbus_write_register,    /* Write Single Register */
    [16]    = 0,                        /* Write Multiple Registers */
    [23]    = 0,                        /* Read/Write Multiple Registers */
    [22]    = 0,                        /* Mask Write Register */
    [24]    = 0,                        /* Read FIFO Queue */
    [20]    = 0,                        /* Read File Record */
    [21]    = 0,                        /* Write File Record */
    /* Diagnostics */
    [7]     = 0,                        /* Read Exception Status */
    [8]     = 0,                        /* Diagnostic (Not valid for IP) */
    [11]    = 0,                        /* Get Com Event Counter (!IP) */
    [12]    = 0,                        /* Get Com Event Log (!IP) */
    [17]    = 0,                        /* Report Server ID (!IP) */
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
    } else  if (frame->fn_code == 43) { /* MEI */
        switch (frame->data[0]) {
        case 0x0E:
            /* Read Device Identification / Encapsulated Interface Transport */
            wr = modbus_read_devid(frame);
            break;
        default:
            wr = modbus_generror(errmsg, MODBUS_EX_ILLFN);
        }
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

    return 1;
}

size_t modbus_reg_handler(const struct __modbus_addr_desc *desc,
        struct modbus_frame *req, size_t index, int wr)
{
    if (!wr) { /* Read */
        req->data[index]     = (*(uint16_t *)desc->mb_var & 0xff00) >> 8;
        req->data[index + 1] = (*(uint16_t *)desc->mb_var & 0x00ff);
    } else { /* Write */
        *((uint16_t *)(desc->mb_var)) =
            (req->data[index + 1] << 8) | (req->data[index]);
    }

    return 2;
}
