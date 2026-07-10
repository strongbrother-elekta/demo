#include "esc_coe.h"
#include "esc.h"
#include "app_process.h"
#include <string.h>

// ========== 全局 PDO 缓冲区（供其他模块使用） ==========
uint8_t output_buffer[8] = {0};
uint8_t input_buffer[8] = {0};

// ========== 对象字典条目定义 ==========
// 对象 0x1000：设备类型（常量）
static const uint32_t device_type_val = 0x00013889;
static const _objd obj_1000[] = {
    { 0x00, DTYPE_UNSIGNED32, 32, ATYPE_RO, "Device Type", device_type_val, NULL }
};

// 对象 0x1008：设备名称（常量字符串）
static const char device_name[] = "SOES_8DI8DO";
static const _objd obj_1008[] = {
    { 0x00, DTYPE_VISIBLE_STRING, 8*sizeof(device_name), ATYPE_RO, "Device Name", 0, (void*)device_name }
};

// 对象 0x6000：数字输入（只读，指向 input_buffer）
static const _objd obj_6000[] = {
    { 0x00, DTYPE_UNSIGNED8, 8, ATYPE_Rpre | ATYPE_Rsafe | ATYPE_Rop, "Inputs", 0, input_buffer }
};

// 对象 0x7000：数字输出（读写，指向 output_buffer）
static const _objd obj_7000[] = {
    { 0x00, DTYPE_UNSIGNED8, 8, ATYPE_RW, "Outputs", 0, output_buffer }
};

// 对象 0x1C12：RxPDO 分配
static uint8_t rx_pdo_count = 1;          // 子索引0的值
static uint16_t rx_pdo_list[2] = { 0x1600, 0x0000 };
static const _objd obj_1C12[] = {
    { 0x00, DTYPE_UNSIGNED8,   8, ATYPE_RW, "Number of RxPDO",  1, &rx_pdo_count },
    { 0x01, DTYPE_UNSIGNED16, 16, ATYPE_RW, "RxPDO 0x1600",   0, &rx_pdo_list[0] },
};

// 对象 0x1C13：TxPDO 分配
static uint8_t tx_pdo_count = 1;
static uint16_t tx_pdo_list[2] = { 0x1A00, 0x0000 };
static const _objd obj_1C13[] = {
    { 0x00, DTYPE_UNSIGNED8,   8, ATYPE_RW, "Number of TxPDO",  1, &tx_pdo_count },
    { 0x01, DTYPE_UNSIGNED16, 16, ATYPE_RW, "TxPDO 0x1A00",   0, &tx_pdo_list[0] },
};

// 对象 0x1600：RxPDO 映射
static uint8_t rx_map_count = 1;
static uint32_t rx_map_list[2] = { (0x7000 << 16) | (0x00 << 8) | 8, 0 };
static const _objd obj_1600[] = {
    { 0x00, DTYPE_UNSIGNED8,   8, ATYPE_RW, "Number of mapped objects", 1, &rx_map_count },
    { 0x01, DTYPE_UNSIGNED32, 32, ATYPE_RW, "Mapped 0x7000:0", 0, &rx_map_list[0] },
};

// 对象 0x1A00：TxPDO 映射
static uint8_t tx_map_count = 1;
static uint32_t tx_map_list[2] = { (0x6000 << 16) | (0x00 << 8) | 8, 0 };
static const _objd obj_1A00[] = {
    { 0x00, DTYPE_UNSIGNED8,   8, ATYPE_RW, "Number of mapped objects", 1, &tx_map_count },
    { 0x01, DTYPE_UNSIGNED32, 32, ATYPE_RW, "Mapped 0x6000:0", 0, &tx_map_list[0] },
};

// ========== 对象列表（非 static，对外可见） ==========
const _objectlist SDOobjects[] = {
    { 0x1000, OTYPE_VAR,   1, 0, "Device type",        obj_1000 },
    { 0x1008, OTYPE_VAR,   1, 0, "Device name",        obj_1008 },
    { 0x6000, OTYPE_VAR,   1, 0, "Digital Inputs",     obj_6000 },
    { 0x7000, OTYPE_VAR,   1, 0, "Digital Outputs",    obj_7000 },
    { 0x1C12, OTYPE_RECORD,2, 0, "RxPDO Assign",       obj_1C12 },
    { 0x1C13, OTYPE_RECORD,2, 0, "TxPDO Assign",       obj_1C13 },
    { 0x1600, OTYPE_RECORD,2, 0, "RxPDO Mapping",      obj_1600 },
    { 0x1A00, OTYPE_RECORD,2, 0, "TxPDO Mapping",      obj_1A00 },
    { 0xFFFF, 0,           0, 0, NULL,                 NULL }
};

// ========== SOES 强制要求的回调函数 ==========
void cb_get_inputs(void) {
    app_process_get_inputs(input_buffer);
}

void cb_set_outputs(void) {
    app_process_set_outputs(output_buffer);
}