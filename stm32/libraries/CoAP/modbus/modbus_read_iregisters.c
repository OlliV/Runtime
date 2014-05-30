#include "modbus_udp.h"

struct mbu_request {
    unsigned s_addr     : 16;
    unsigned quant      : 16;
};

struct mbu_response {
    unsigned bcount     : 8;
    uint8_t status[];
};

#define MODBUS_GET_REQ(frame)   ((struct mbu_request *)((uint8_t *)(frame) + 8))
#define MODBUS_GET_RESP(frame)  ((struct mbu_response *)((uint8_t *)(frame) + 8))
#define MODBUS_REQ_TO_RESP(req) ((struct mbu_response *)(req))

extern const struct __modbus_addr_desc __attribute__((section(__MBADDR_SECTION))) __modbus_app_mem[];
extern const struct __modbus_addr_desc __attribute__((section(__MBADDR_SECTION))) __modbus_app_mem_end[];

static struct mbu_request *ntohreq(struct modbus_frame *frame)
{
    struct mbu_request *req = MODBUS_GET_REQ(frame);

    req->s_addr = ntohs(req->s_addr);
    req->quant = ntohs(req->quant);

    return req;
}

static void htonresp(struct modbus_frame *frame)
{
    /* NOP */
}

size_t modbus_read_iregisters(struct modbus_frame *frame)
{
    struct mbu_request *req = ntohreq(frame);
    struct mbu_response *resp = MODBUS_GET_RESP(frame);
    size_t addr = req->s_addr;
    size_t wr = 0;

    if (!(1 <= req->quant && req->quant <= 0x7D))
        return modbus_generror(MODBUS_GET_ERR(frame), MODBUS_EX_ILLDV);

    if ((addr + (size_t)req->quant) > 0xffff)
        return modbus_generror(MODBUS_GET_ERR(frame), MODBUS_EX_ILLDA);

    for (size_t i = 0; i < (__modbus_app_mem_end - __modbus_app_mem); i++) {
        const struct __modbus_addr_desc *mbad = &__modbus_app_mem[i];
        if (mbad->mb_addr == addr) {
            if (!mbad->mb_handler)
                return modbus_generror(MODBUS_GET_ERR(frame), MODBUS_EX_SDV);
            if (!(mbad->mb_acc & MODBUS_RD))
                return modbus_generror(MODBUS_GET_ERR(frame), MODBUS_EX_ILLDA);
            wr =+ mbad->mb_handler(mbad, frame, wr + 1, 0);
        } else continue;
        addr++;
        if (addr > (req->s_addr + req->quant))
            break;
    }
    resp->bcount = wr;

    if (wr == 0) {
        return modbus_generror(MODBUS_GET_ERR(frame), MODBUS_EX_ILLDA);
    }

    htonresp(frame);
    return wr + 1;
}
