#include "modbus_udp.h"

struct mbu_request {
    unsigned reg_addr   : 16;
    unsigned reg_val    : 16;
};

struct mbu_response {
    unsigned reg_addr   : 16;
    unsigned reg_val    : 16;
};

#define MODBUS_GET_REQ(frame)   ((struct mbu_request *)((uint8_t *)(frame) + 8))
#define MODBUS_GET_RESP(frame)  ((struct mbu_response *)((uint8_t *)(frame) + 8))
#define MODBUS_REQ_TO_RESP(req) ((struct mbu_response *)(req))

extern const struct __modbus_addr_desc __attribute__((section(__MBADDR_SECTION))) __modbus_app_mem[];
extern const struct __modbus_addr_desc __attribute__((section(__MBADDR_SECTION))) __modbus_app_mem_end[];

static struct mbu_request *ntohreq(struct modbus_frame *frame)
{
    struct mbu_request *req = MODBUS_GET_REQ(frame);

    req->reg_addr = ntohs(req->reg_addr);
    req->reg_val = ntohs(req->reg_val);

    return req;
}

static void htonresp(struct modbus_frame *frame)
{
    struct mbu_response *resp = MODBUS_GET_RESP(frame);

    resp->reg_addr = htons(resp->reg_addr);
    resp->reg_val = htons(resp->reg_val);
}

size_t modbus_write_register(struct modbus_frame *frame)
{
    struct mbu_request *req = ntohreq(frame);
    size_t addr = req->reg_addr;

    for (size_t i = 0; i < (__modbus_app_mem_end - __modbus_app_mem); i++) {
        const struct __modbus_addr_desc *mbad = &__modbus_app_mem[i];
        if (mbad->mb_addr == addr) {
            if (!mbad->mb_handler)
                return modbus_generror(MODBUS_GET_ERR(frame), MODBUS_EX_SDV);
            if (!(mbad->mb_acc & MODBUS_WR))
                return modbus_generror(MODBUS_GET_ERR(frame), MODBUS_EX_ILLDA);
            mbad->mb_handler(mbad, frame, 2, 1);
            break;
        }
    }

    htonresp(frame);
    return 4;
}
