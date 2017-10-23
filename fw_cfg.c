
#include "mmio.h"
#include "common.h"
#include "fw_cfg.h"


void fw_cfg_key(uint16_t key)
{
    out16x(CFG_ADDR, 0, key);
}

uint8_t fw_cfg_readbyte(void)
{
    return in8x(CFG_ADDR, 2);
}

void fw_cfg_readmore(void *vbuf, size_t len)
{
    char *buf = vbuf;
    while(len--) {
        *buf++ = in8x(CFG_ADDR, 2);
    }
}

void fw_cfg_read(uint16_t key, void *vbuf, size_t len)
{
    fw_cfg_key(key);
    fw_cfg_readmore(vbuf, len);
}

uint32_t fw_cfg_read32(uint16_t key)
{
    uint32_t val;
    fw_cfg_read(key, &val, sizeof(val));
    return __builtin_bswap32(val);
}

uint64_t fw_cfg_read64(uint16_t key)
{
    uint64_t val;
    fw_cfg_read(key, &val, sizeof(val));
    return __builtin_bswap64(val);
}

int fw_cfg_show(void)
{
    char cbuf[16];
    fw_cfg_read(FW_CFG_SIGNATURE, cbuf, 16);
    printk("FW ID: \"%s\"\n", cbuf);

    return strcmp(cbuf, "QEMU")!=0;
}

unsigned fw_cfg_list_files(void)
{
    unsigned count, i;

    count = __builtin_bswap32(fw_cfg_read32(FW_CFG_FILE_DIR));
    
    printk("Found %x files\n", count);

    for(i=0; i<count; i++) {
        FWCfgFile info;
        fw_cfg_readmore(&info, sizeof(info));
        if(info.select==0)
            break; /* should not be necessary if count is correct */
        printk("%x %x %s\n", info.select, (unsigned)info.size, info.name);
    }

    return count;
}

int fw_cfg_open(const char *filename, uint32_t *psize)
{
    unsigned count, i;

    count = __builtin_bswap32(fw_cfg_read32(FW_CFG_FILE_DIR));

    for(i=0; i<count; i++) {
        FWCfgFile info;
        fw_cfg_readmore(&info, sizeof(info));
        if(info.select==0)
            break; /* should not be necessary if count is correct */
        if(strcmp(info.name, filename)==0) {
            fw_cfg_key(info.select);
            if(psize)
                *psize = info.size;
            return 0;
        }
    }
    return -1;
}
