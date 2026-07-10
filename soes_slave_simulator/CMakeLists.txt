cmake_minimum_required(VERSION 3.10)
project(soes_slave_simulator)

set(CMAKE_C_STANDARD 99)

set(CMAKE_BUILD_TYPE Debug)
# 设置SOES根目录（根据您的实际路径修改）
set(SOES_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/3rd/SOES/soes)

# Disable EOE globally before building SOES
add_definitions(-DUSE_EOE=0)

# Build SOES as part of this project
add_subdirectory(3rd/SOES/soes)

# 包含路径
include_directories(
    src
    ${SOES_ROOT_DIR}
    ${SOES_ROOT_DIR}/include/sys/gcc
	/usr/include
)

# 编译宏：使SM2/SM3地址与您的XML一致
add_definitions(-DSM2_sma=0x1200 -DSM3_sma=0x1300)
add_definitions(-DMAX_RXPDO_SIZE=8 -DMAX_TXPDO_SIZE=8)
add_definitions(-DESC_DEBUG=1)


# 可执行文件
add_executable(soes_slave_simulator
    src/main.c
    src/esc_hal.c
    src/app_process.c
    src/sdo_objects.c
    src/pcap_emulator.c
)

target_link_directories(soes_slave_simulator PRIVATE ${SOES_ROOT_DIR})
# 链接SOES库（已通过add_subdirectory编译）
target_link_libraries(soes_slave_simulator PRIVATE soes pthread pcap)
