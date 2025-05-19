/*
 * flash_ops.h
 *
 *  Created on: 9 Jan 2025
 *      Author: CalvinJohnson
 */

#ifndef FLASH_OPS_H_
#define FLASH_OPS_H_

#include <stdint.h>

#define NOR_CMD_LUT_SEQ_IDX_READ_FAST_QUAD   0
#define NOR_CMD_LUT_SEQ_IDX_READSTATUSREG    1
#define NOR_CMD_LUT_SEQ_IDX_WRITEENABLE      3
#define NOR_CMD_LUT_SEQ_IDX_ERASESECTOR      5
#define NOR_CMD_LUT_SEQ_IDX_ERASEBLOCK       8
#define NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_QUAD 9
#define NOR_CMD_LUT_SEQ_IDX_ERASECHIP        11

void flash_init(void);

void flash_print_lut(void);

uint32_t flash_read_status(void);

void flash_read(uint32_t sourceAddr, uint8_t *buf, uint32_t len);

void flash_read_ipc(uint32_t sourceAddr, uint8_t *buf, uint32_t len);

#endif /* FLASH_OPS_H_ */
