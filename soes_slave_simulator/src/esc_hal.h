#ifndef ESC_HAL_H
#define ESC_HAL_H

#include <stdint.h>
#include <stddef.h>

#define ESC_DPRAM_SIZE 0x2000

int  esc_hal_init(void);
void esc_hal_read(uint16_t address, void* data, size_t size);
void esc_hal_write(uint16_t address, const void* data, size_t size);
/* Set simulated link state: 0 = down, non-zero = up */
void esc_hal_set_link(int up);

/* Optional: start a packet capture thread that watches for EtherCAT frames
 * and toggles link state. Implemented in pcap_emulator.c when available. */
int esc_hal_start_pcap(const char *iface);

/* Async version: starts pcap listening in a background thread and returns immediately.
 * Useful to avoid blocking the main application. */
int esc_hal_start_pcap_async(const char *iface);

#endif