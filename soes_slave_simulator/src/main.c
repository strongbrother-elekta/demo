#include <stdio.h>
#include <unistd.h>
#include "esc_hal.h"
#include "ecat_slv.h"
#include "app_process.h"

int main(void) {
    printf("SOES Slave Simulator Starting...\n");

    if (esc_hal_init() != 0) {
        printf("ESC HAL init failed\n");
        return -1;
    }

    /* Optionally start pcap helper to detect EtherCAT frames and bring link up.
     * Set PCAP_IFACE environment variable to the interface name (e.g. tap0 or eth0).
     * Using async version to avoid blocking the main application.
     */
    //const char *pcap_iface = getenv("PCAP_IFACE");
    const char *pcap_iface = "enp0s3"; // For testing, hardcode to eth0. Change as needed.
    if (pcap_iface != NULL) {
        printf("[DEBUG] PCAP_IFACE env value=%s\n", pcap_iface);
        if (pcap_iface[0] != '\0') {
            if (esc_hal_start_pcap_async(pcap_iface) == 0) {
                printf("Started pcap_emulator on interface %s (async mode)\n", pcap_iface);
            } else {
                printf("Failed to start pcap_emulator on interface %s\n", pcap_iface);
            }
        } else {
            printf("PCAP_IFACE is set but empty, skipping pcap_emulator\n");
        }
    }

    app_process_init();

    esc_cfg_t config = {
        .use_interrupt = 0,
        .watchdog_cnt = 1000,
        .skip_default_initialization = 0,
	  //.use_interrupt = 0,
      //.watchdog_cnt = 150,
      //.set_defaults_hook = NULL,
      //.pre_state_change_hook = NULL,
      //.post_state_change_hook = NULL,
      //.application_hook = NULL,
      //.safeoutput_override = NULL,
      //.pre_object_download_hook = NULL,
      //.post_object_download_hook = NULL,
      //.rxpdo_override = NULL,
      //.txpdo_override = NULL,
      //.esc_hw_interrupt_enable = NULL,
      //.esc_hw_interrupt_disable = NULL,
      //.esc_hw_eep_handler = NULL,
      //.esc_check_dc_handler = NULL,
    };
    

    ecat_slv_init(&config);
    printf("Slave is ready, waiting for master...\n");

    while (1) {
        ecat_slv();
        app_process_cyclic();
        usleep(10000);
    }

    return 0;
}