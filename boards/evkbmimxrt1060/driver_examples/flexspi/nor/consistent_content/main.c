/*
 * Copyright (c) 2013 - 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"

#include "fsl_flexram_allocate.h"

#include "flash_ops.h"
#include "flash_test.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/


/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static void FlexRAM_Allocation(void);

/*******************************************************************************
 * Code
 ******************************************************************************/
/*!
 * @brief Main function
 */
int main(void)
{
    /* Init board hardware. */
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();

    PRINTF("\r\n*** START ***\r\n");
    PRINTF("CCR: %08X\r\n", SCB->CCR);

    BOARD_ConfigMPU();

    FlexRAM_Allocation();

    flash_init();

        flash_test();

        while (1)
        {
        }

        return 0;
}

static void FlexRAM_Allocation(void)
{
    uint32_t flexram_alloc = IOMUXC_GPR->GPR17;

    uint32_t itcm_count    = 0;
    uint32_t dtcm_count    = 0;
    uint32_t ocram_count   = 0;

    for (uint8_t i = 0; i < FSL_FEATURE_FLEXRAM_INTERNAL_RAM_TOTAL_BANK_NUMBERS;
         i++)
    {
        uint32_t entry = flexram_alloc & 0x3;

        if (entry == kFLEXRAM_BankOCRAM)
        {
            ocram_count += 32;
        }
        else if (entry == kFLEXRAM_BankDTCM)
        {
            dtcm_count += 32;
        }
        else if (entry == kFLEXRAM_BankITCM)
        {
            itcm_count += 32;
        }

        flexram_alloc = flexram_alloc >> 2;
    }

    PRINTF(
        "\r\nFLEXRAM allocation -> DTC: %dKB - OCRAM: %dKB - ITCM: %dKB.\r\n",
        dtcm_count, ocram_count, itcm_count
    );
}
