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

    for (size_t i = 0; i < (__modbus_app_mem_end - __modbus_app_mem); i++) {
        const struct __modbus_addr_desc *mbad = &__modbus_app_mem[i];
        if (mbad->mb_addr == addr) {
            if (!mbad->mb_handler) {
                return modbus_generror(MODBUS_GET_ERR(frame), MODBUS_EX_SDV);
            }
            wr =+ mbad->mb_handler(mbad, req, wr);
        }
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

/* Default hanlder for iregisters */
size_t modbus_iregister_handler(const struct __modbus_addr_desc *desc, void *req, size_t index)
{
    struct mbu_response *resp = MODBUS_REQ_TO_RESP((struct mbu_request *)req);

    resp->status[index]     = (*(uint16_t *)desc->mb_var & 0xff00) >> 8;
    resp->status[index + 1] = (*(uint16_t *)desc->mb_var & 0x00ff);

    return 2;
}
