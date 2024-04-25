#ifndef __PLATFORM_H
#define __PLATFORM_H

#define MEM_BELOW_1M_START  0ULL
#define MEM_BELOW_1M_SIZE   0xe0000ULL /* 896K */
#define MMIO_BELOW_1M_START (MEM_BELOW_1M_START + MEM_BELOW_1M_SIZE)
#define MMIO_BELOW_1M_SIZE (1ULL * 1024 * 1024 - MMIO_BELOW_1M_START)

#define MEM_BELOW_4G_START (1ULL * 1024 * 1024)
#define MEM_BELOW_4G_END 0xc0000000ULL
#define MMIO_BELOW_4G_START MEM_BELOW_4G_END
#define MMIO_BELOW_4G_SIZE (0x100000000ULL - MMIO_BELOW_4G_START)
#define MEM_ABOVE_4G_START 0x100000000ULL

#define DEVICE_SERIAL_PORT_MMIO_BEGIN MMIO_BELOW_1M_START
#define DEVICE_SERIAL_MMIO_DATA_OUT (DEVICE_SERIAL_PORT_MMIO_BEGIN + 1)
#define DEVICE_SERIAL_PORT_MMIO_END (DEVICE_SERIAL_PORT_MMIO_BEGIN + 16)

#endif
