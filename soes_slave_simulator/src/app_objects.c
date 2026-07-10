// src/app_objects.c (新建文件)
#include "esc_coe.h"
#include "ethercat.h"
#include "sdo.h"
#include <string.h>

// 声明你的 PDO 缓冲区
extern uint8_t output_buffer[8];
extern uint8_t input_buffer[8];

// 对象字典条目
const COE_Obj OD_6000_DigitalInputs = {
    .Index = 0x6000,
    .SubIndex = 0x00,
    .DataType = TYPE_UINT8,
    .ObjAccess = (COE_ObjAccess)(COE_READABLE), // 只读
    .pValue = (void*)input_buffer, // 指向你的输入数据区
    .pCallBack = NULL
};

const COE_Obj OD_7000_DigitalOutputs = {
    .Index = 0x7000,
    .SubIndex = 0x00,
    .DataType = TYPE_UINT8,
    .ObjAccess = (COE_ObjAccess)(COE_READABLE | COE_WRITEABLE), // 可读写
    .pValue = (void*)output_buffer, // 指向你的输出数据区
    .pCallBack = NULL
};

// 其他必要的对象，如设备名称 (0x1008)
const char device_name[] = "SOES_8DI8DO";
const COE_Obj OD_1008_DeviceName = {
    .Index = 0x1008,
    .SubIndex = 0x00,
    .DataType = TYPE_VISIBLE_STRING,
    .ObjAccess = COE_READABLE,
    .pValue = (void*)device_name,
    .pCallBack = NULL
};

int register_app_objects(void) {
    // 向SOES的对象字典表中注册这些对象
    // 具体的API函数需要查SOES文档，例如:
    // if (coe_register_object(&OD_6000_DigitalInputs) != 0) return -1;
    // if (coe_register_object(&OD_7000_DigitalOutputs) != 0) return -1;
    // if (coe_register_object(&OD_1008_DeviceName) != 0) return -1;
    return 0;
}