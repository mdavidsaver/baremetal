#ifndef FW_CFG_H
#define FW_CFG_H

#include <stdint.h>
#include <stddef.h>

#define CFG_ADDR 0xf0000510
#define FW_CFG_SIGNATURE        0x00
#define FW_CFG_ID               0x01
#define FW_CFG_RAM_SIZE         0x03
#define FW_CFG_KERNEL_ADDR      0x07
#define FW_CFG_KERNEL_SIZE      0x08

#define FW_CFG_CMDLINE_SIZE     0x14
#define FW_CFG_CMDLINE_DATA     0x15

#define FW_CFG_FILE_DIR         0x19

#define FW_CFG_FILE_FIRST       0x20

void fw_cfg_key(uint16_t key);
int fw_cfg_open(const char *filename, uint32_t *psize);
void fw_cfg_readmore(void *vbuf, size_t len);
uint8_t fw_cfg_readbyte(void);

/* key, reset, and read first len bytes (or padding) */
void fw_cfg_read(uint16_t key, void *vbuf, size_t len);


uint32_t fw_cfg_read32(uint16_t key);
uint64_t fw_cfg_read64(uint16_t key);

/* return zero if fw_cfg fount */
int fw_cfg_show(void);
/* return number of files */
unsigned fw_cfg_list_files(void);

typedef struct {              /* an individual file entry, 64 bytes total */
    uint32_t size;              /* size of referenced fw_cfg item, big-endian */
    uint16_t select;            /* selector key of fw_cfg item, big-endian */
    uint16_t reserved;
    char name[56];              /* fw_cfg item name, NUL-terminated ascii */
} FWCfgFile;


#endif /* FW_CFG_H */
