#ifndef MODBUS_UDP_H
#define MODBUS_UDP_H

#include <stdint.h>
#include <stddef.h>
#include "../arch/stm32/util.h"

struct modbus_frame {
    unsigned trans_id   : 16;
    unsigned prot_id    : 16;
    unsigned len        : 16;
    unsigned unit_id    : 8;
    unsigned fn_code    : 8; /* Not a part of the MBAP header */
    uint8_t data[0];
};

#define MODBUS_FRAME_LEN(wr) \
    (sizeof(struct modbus_frame) - 3 * sizeof(uint16_t) + (wr))

#define MODBUS_UDP_DG_LEN(wr) \
    (sizeof(struct modbus_frame) + wr)

struct modbus_error {
    unsigned fn_code    : 8;
    unsigned exception  : 8;
};

#define MODBUS_GET_ERR(frame)  ((struct modbus_error *)((uint8_t *)(frame) + 7))

#define MODBUS_EX_ILLFN 0x01    /*!< Illegal function */
#define MODBUS_EX_ILLDA 0x02    /*!< Illegeal data adrres */
#define MODBUS_EX_ILLDV 0x03    /*!< Illegal data value */
#define MODBUS_EX_SDV   0x04    /*!< Server device failure */
#define MODBUS_EX_ACK   0x05    /*!< Acknowledge */
#define MODBUS_EX_SDB   0x06    /*!< Service device busy */
#define MODBUS_EX_MPE   0x08    /*!< Memory parity error */
#define MODBUS_EX_GPU   0x0A    /*!< Gateway path unavailable */
#define MODBUS_EX_GWFR  0x0B    /*!< Gateway target device failed to respond */

struct __modbus_addr_desc {
    size_t mb_addr;
    void *mb_var;
    size_t mb_acc;
    size_t (*mb_handler)(const struct __modbus_addr_desc *desc,
            struct modbus_frame *req, size_t index, int wr);
};

#define MODBUS_NA 0x0
#define MODBUS_RD 0x1
#define MODBUS_WR 0x2

#define __EXPAND_MBADDR_NAME(var) __modbus_addr_ ## var
#define __MBADDR_SECTION  ".modbus.app_mem."

# define DECLARE_MODBUS_ADDR(var)                                       \
    extern const struct __modbus_addr_desc __EXPAND_MBADDR_NAME(var)    \
        __attribute__((section(__MBADDR_SECTION)))

#define DEFINE_MODBUS_ADDR(addr, var, access, handler)                  \
    DECLARE_MODBUS_ADDR(var);                                           \
    const struct __modbus_addr_desc __EXPAND_MBADDR_NAME(var)           \
        __attribute__((section(__MBADDR_SECTION))) = {                  \
        .mb_addr = (addr), .mb_var = &(var), .mb_acc = access,          \
        .mb_handler = (handler)                                         \
    }

#define DEFINE_MODBUS_REGISTER(addr, var, access)                      \
    DEFINE_MODBUS_ADDR((addr), var, (access), &modbus_reg_handler)

# ifdef __cplusplus
extern "C" {
# endif
typedef size_t (*modbus_fn_t)(struct modbus_frame *frame);

size_t modbus_generror(struct modbus_error *errmsg, int8_t exception);
size_t modbus_reg_handler(const struct __modbus_addr_desc *desc,
        struct modbus_frame *req, size_t index, int wr);

size_t modbus_read_iregisters(struct modbus_frame *frame);

size_t modbus_write_register(struct modbus_frame *frame);

size_t modbus_read_devid(struct modbus_frame *frame);

# ifdef __cplusplus
}
# endif

#endif /* MODBUS_UDP_H */
