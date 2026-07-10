#include "app_process.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

static uint8_t virtual_inputs = 0x00;

void app_process_init(void) {
    srand((unsigned int)time(NULL));
    printf("[APP] Application initialized.\n");
}

void app_process_cyclic(void) {
    static int counter = 0;
    if (++counter >= 500) {
        virtual_inputs = (virtual_inputs == 0x00) ? 0xFF : 0x00;
        counter = 0;
        printf("[APP] Virtual Inputs changed to: 0x%02X\n", virtual_inputs);
    }
}

void app_process_get_inputs(uint8_t* input) {
    input[0] = virtual_inputs;
}

void app_process_set_outputs(const uint8_t* output) {
    printf("[APP] Received Outputs: 0x%02X\n", output[0]);
}