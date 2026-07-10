#include "esc_hal.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include "esc.h"          // 包含 esc_cfg_t

static uint8_t dpram[ESC_DPRAM_SIZE];

void esc_hal_read(uint16_t address, void* data, size_t size) {
    if (address + size > ESC_DPRAM_SIZE) 
		return;
    memcpy(data, &dpram[address], size);
}

void esc_hal_write(uint16_t address, const void* data, size_t size) {
    if (address + size > ESC_DPRAM_SIZE) 
		return;
    memcpy(&dpram[address], data, size);
}

// SOES 底层接口
void ESC_read(uint16_t address, void* buf, uint16_t len) {
    esc_hal_read(address, buf, len);
}

void ESC_write(uint16_t address, void* buf, uint16_t len) {
    esc_hal_write(address, buf, len);
}

void ESC_init(const esc_cfg_t* cfg) {
    (void)cfg;
    // 填充必要的 ESC 寄存器（供主站识别）
    uint32_t esc_type = 0x00001389;
    ESC_write(0x0000, &esc_type, 4);
    uint32_t product_code = 0x00000001;
    ESC_write(0x0008, &product_code, 4);
    uint32_t revision_no = 0x00010000;
    ESC_write(0x000C, &revision_no, 4);
    uint16_t alias = 0x0000;
    ESC_write(0x0012, &alias, 2);
    /* Set DLSTATUS link-up bit so stack initialization can continue
     * (emulator: simulate link up). */
    uint16_t dlstatus = 0x0001;
    ESC_write(ESCREG_DLSTATUS, &dlstatus, 2);
    printf("[ESC HAL] ESC init done.\n");
}

void ESC_reset(void) {
    memset(dpram, 0, ESC_DPRAM_SIZE);
    printf("[ESC HAL] ESC reset.\n");
}

void esc_hal_set_link(int up)
{
    uint16_t dlstatus = up ? 0x0001 : 0x0000;
    /* write DLSTATUS (address defined in esc.h) */
    esc_hal_write(ESCREG_DLSTATUS, &dlstatus, 2);
    printf("[ESC HAL] link %s\n", up ? "up" : "down");
}


// EEPROM 仿真（简单占位）
static const uint8_t eeprom_data[256] = {0};
int8_t EEP_read(uint32_t addr, uint8_t* data, uint16_t size) {
    if (addr + size > sizeof(eeprom_data)) return -1;
    memcpy(data, &eeprom_data[addr], size);
    return 0;
}

int8_t EEP_write(uint32_t addr, uint8_t* data, uint16_t size) {
    (void)addr; (void)data; (void)size;
    return 0;
}

int esc_hal_init(void) {
    ESC_reset();
    return 0;
}

