#include "modbus_udp.h"

union devid_base {
    struct dbase_struct {
        char * vendor_name;
        char * product_code;
        char * major_minor_version;
        char * vendor_url;
        char * product_name;
        char * model_name;
        char * user_app_name;
    } by_name;
    char ** by_id;
};

static union devid_base devid_base = {
    .by_name = {
        .vendor_name = "ELL-i",
        .product_code = "",
        .major_minor_version = "1.0",
        .vendor_url = "http://ell-i.org/",
    }
};
#define DEVID_BASE_SIZE (sizeof(devid_base) / sizeof(char *))

struct mbu_request {
    unsigned mei_type       : 8;
    unsigned rd_devid_code  : 8;
    unsigned obj_id         : 8;
};

struct mbu_response {
    unsigned mei_type       : 8;
    unsigned rd_devid_code  : 8;
    unsigned conf_level     : 8;
    unsigned more_follows   : 8;
    unsigned next_obj_id    : 8;
    unsigned nobjs          : 8;
    uint8_t list[];
};

#define MODBUS_GET_REQ(frame)   ((struct mbu_request *)((uint8_t *)(frame) + 8))
#define MODBUS_GET_RESP(frame)  ((struct mbu_response *)((uint8_t *)(frame) + 8))
#define MODBUS_REQ_TO_RESP(req) ((struct mbu_response *)(req))

static size_t ua_strcpy(char * dst, char * src)
{
    size_t n = 0;

    while (*src != '\0') {
        *dst = *src;
        n++;

        src++;
        dst++;
    }
    *dst = '\0';

    return n;
}

static size_t read_obj(uint8_t * p, size_t oid)
{
    char * src;

    if (oid > DEVID_BASE_SIZE)
        return 0;
    src = devid_base.by_id[oid];

    if (src == 0)
        return 0;

    p[0] = oid;
    p[1] = ua_strcpy((char *)(p + 2), src);

    return p[1];
}

static size_t read_mobjs(struct mbu_request *req, size_t first_oid, size_t max_oid)
{
    struct mbu_response *resp = MODBUS_REQ_TO_RESP(req);
    size_t i = first_oid;
    int obj_bytes = 0;

    /* Note: conf_level == rd_devid_code */

    /* XXX Long response should be split into multiple messages */
    resp->more_follows = 0x00;
    resp->next_obj_id = 0x00;
    resp->nobjs = 0;

    for (; i <= max_oid; i++) {
        obj_bytes += read_obj(&(resp->list[obj_bytes]), i);
        resp->nobjs++;
    }

    if (resp->nobjs == 0)
        return 0;
    return obj_bytes + 6;
}

static size_t read_specific(struct mbu_request *req)
{
    struct mbu_response *resp = MODBUS_REQ_TO_RESP(req);

    return 0;
}

size_t modbus_read_devid(struct modbus_frame *frame)
{
    struct mbu_request *req = MODBUS_GET_REQ(frame);
    struct modbus_error *errmsg = MODBUS_GET_ERR(frame);
    size_t oid = req->obj_id;
    int wr = 0;

    switch (req->rd_devid_code) {
        case 1: /* basic */
        wr = read_mobjs(req, (oid == 0 || oid > DEVID_BASE_SIZE - 1) ? 0 : oid, 2);
        break;
    case 2: /* reqular */
        wr = read_mobjs(req, (oid == 0 || oid > DEVID_BASE_SIZE - 1) ? 3 : oid, 6);
        break;
    case 3: /* extended */
        //return read_extended(mbu_request *req);
        wr = modbus_generror(errmsg, MODBUS_EX_ILLDV);
        break;
    case 4: /* specific */
        wr = read_specific(req);
        break;
    default:
        return modbus_generror(errmsg, MODBUS_EX_ILLDV);
    }

    if (wr == 0)
        return modbus_generror(errmsg, MODBUS_EX_ILLDA);
    return wr;
}
