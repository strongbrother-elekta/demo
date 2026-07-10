#ifndef APP_PROCESS_H
#define APP_PROCESS_H

#include <stdint.h>

void app_process_init(void);
void app_process_cyclic(void);
void app_process_get_inputs(uint8_t* input);
void app_process_set_outputs(const uint8_t* output);

#endif