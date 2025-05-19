/*
 * flash_ops.c
 *
 *  Created on: 9 Jan 2025
 *      Author: CalvinJohnson
 */

#include "flash_ops.h"

#include "fsl_flexspi.h"

#include "fsl_debug_console.h"

#define FLASH_BUSY_STATUS_POLARITY 1
#define FLASH_BUSY_STATUS_OFFSET   0

#define CUSTOM_LUT_LENGTH          64

static const uint32_t customLUT[CUSTOM_LUT_LENGTH] = {
    // Read
    // cppcheck-suppress misra-c2012-2.2
    [(4 * NOR_CMD_LUT_SEQ_IDX_READ_FAST_QUAD)] = FLEXSPI_LUT_SEQ(
        kFLEXSPI_Command_SDR,
        kFLEXSPI_1PAD,
        0xEB,
        kFLEXSPI_Command_RADDR_SDR,
        kFLEXSPI_4PAD,
        0x18
    ),

    // cppcheck-suppress misra-c2012-2.2
    [(4 * NOR_CMD_LUT_SEQ_IDX_READ_FAST_QUAD) + 1] = FLEXSPI_LUT_SEQ(
        kFLEXSPI_Command_DUMMY_SDR,
        kFLEXSPI_4PAD,
        0x06,
        kFLEXSPI_Command_READ_SDR,
        kFLEXSPI_4PAD,
        0x04
    ),

    /* Read status register */
    [(4 * NOR_CMD_LUT_SEQ_IDX_READSTATUSREG)] = FLEXSPI_LUT_SEQ(
        kFLEXSPI_Command_SDR,
        kFLEXSPI_1PAD,
        0x05,
        kFLEXSPI_Command_READ_SDR,
        kFLEXSPI_1PAD,
        0x04
    ),

    /* Write Enable */
    [(4 * NOR_CMD_LUT_SEQ_IDX_WRITEENABLE)] = FLEXSPI_LUT_SEQ(
        kFLEXSPI_Command_SDR,
        kFLEXSPI_1PAD,
        0x06,
        kFLEXSPI_Command_STOP,
        kFLEXSPI_1PAD,
        0
    ),

    /* Erase Sector*/
    [(4 * NOR_CMD_LUT_SEQ_IDX_ERASESECTOR)] = FLEXSPI_LUT_SEQ(
        kFLEXSPI_Command_SDR,
        kFLEXSPI_1PAD,
        0xD7,
        kFLEXSPI_Command_RADDR_SDR,
        kFLEXSPI_1PAD,
        0x18
    ),

    /* Page Program - quad mode or Octal mode */
    [(4 * NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_QUAD)] = FLEXSPI_LUT_SEQ(
        kFLEXSPI_Command_SDR,
        kFLEXSPI_1PAD,
        0x32,
        kFLEXSPI_Command_RADDR_SDR,
        kFLEXSPI_1PAD,
        0x18
    ),
    [(4 * NOR_CMD_LUT_SEQ_IDX_PAGEPROGRAM_QUAD) + 1] = FLEXSPI_LUT_SEQ(
        kFLEXSPI_Command_WRITE_SDR,
        kFLEXSPI_4PAD,
        0x01 /*0x04*/,
        kFLEXSPI_Command_STOP,
        kFLEXSPI_1PAD,
        0
    ),

    /* Erase whole chip */
    [(4 * NOR_CMD_LUT_SEQ_IDX_ERASECHIP)] = FLEXSPI_LUT_SEQ(
        kFLEXSPI_Command_SDR,
        kFLEXSPI_1PAD,
        0xC7,
        kFLEXSPI_Command_STOP,
        kFLEXSPI_1PAD,
        0
    ),

};

void flash_init(void)
{
    // static uint32_t customLUTTemp[CUSTOM_LUT_LENGTH];
    // memcpy(customLUTTemp, customLUT, CUSTOM_LUT_LENGTH * sizeof(uint32_t));

    FLEXSPI_UpdateLUT(FLEXSPI, 0, &customLUT[0], CUSTOM_LUT_LENGTH);

    FLEXSPI_SoftwareReset(FLEXSPI);
}

void flash_print_lut(void)
{
    PRINTF(
        "FLEXSPI->FLSHCR2: %08X %08X %08X %08X\r\n", FLEXSPI->FLSHCR2[0],
        FLEXSPI->FLSHCR2[1], FLEXSPI->FLSHCR2[2], FLEXSPI->FLSHCR2[3]
    );
    PRINTF("FLEXSPI->LUTCR: %08X\r\n", FLEXSPI->LUTCR);
    PRINTF("LUT\r\n");
    for (uint8_t i = 0; i < 64 / 4; i++)
    {
        PRINTF(
            "%d - %08X %08X %08X %08X\r\n", i, FLEXSPI->LUT[i * 4 + 0],
            FLEXSPI->LUT[i * 4 + 1], FLEXSPI->LUT[i * 4 + 2],
            FLEXSPI->LUT[i * 4 + 3]
        );
    }

    PRINTF("CUSTOM LUT\r\n");
    for (uint8_t i = 0; i < 64 / 4; i++)
    {
        PRINTF(
            "%d - %08X %08X %08X %08X\r\n", i, customLUT[i * 4 + 0],
            customLUT[i * 4 + 1], customLUT[i * 4 + 2], customLUT[i * 4 + 3]
        );
    }
}

uint32_t flash_read_status(void)
{
    uint32_t readValue = 0;
    flexspi_transfer_t flashXfer;

    flashXfer.deviceAddress = 0;
    flashXfer.port          = kFLEXSPI_PortA1;
    flashXfer.cmdType       = kFLEXSPI_Read;
    flashXfer.SeqNumber     = 1;
    flashXfer.seqIndex      = NOR_CMD_LUT_SEQ_IDX_READSTATUSREG;
    flashXfer.data          = &readValue;
    flashXfer.dataSize      = 1;

    (void) FLEXSPI_TransferBlocking(FLEXSPI, &flashXfer);

    return readValue;
}

void flash_wait_bus_busy(FLEXSPI_Type *base)
{
    bool isBusy;
    uint32_t readValue;
    status_t status;
    flexspi_transfer_t flashXfer;

    flashXfer.deviceAddress = 0;
    flashXfer.port          = kFLEXSPI_PortA1;
    flashXfer.cmdType       = kFLEXSPI_Read;
    flashXfer.SeqNumber     = 1;
    flashXfer.seqIndex      = NOR_CMD_LUT_SEQ_IDX_READSTATUSREG;
    flashXfer.data          = &readValue;
    flashXfer.dataSize      = 1;

    do
    {
        status = FLEXSPI_TransferBlocking(base, &flashXfer);

        if (status != kStatus_Success)
        {
            break;
        }
        if (FLASH_BUSY_STATUS_POLARITY)
        {
            if ((readValue & (1U << FLASH_BUSY_STATUS_OFFSET)) != 0U)
            {
                isBusy = true;
            }
            else
            {
                isBusy = false;
            }
        }
        else
        {
            if ((readValue & (1U << FLASH_BUSY_STATUS_OFFSET)) != 0U)
            {
                isBusy = false;
            }
            else
            {
                isBusy = true;
            }
        }

    } while (isBusy);
}

void flash_read(uint32_t sourceAddr, uint8_t *buf, uint32_t len)
{
    uint32_t addr = sourceAddr | FlexSPI_AMBA_BASE;
    (void) memcpy(buf, (uint8_t *) addr, len);
}

void flash_read_ipc(uint32_t sourceAddr, uint8_t *buf, uint32_t len)
{
    flexspi_transfer_t flashXfer;

    flashXfer.deviceAddress = sourceAddr;
    flashXfer.port          = kFLEXSPI_PortA1;
    flashXfer.cmdType       = kFLEXSPI_Read;
    flashXfer.SeqNumber     = 1;
    flashXfer.seqIndex      = NOR_CMD_LUT_SEQ_IDX_READ_FAST_QUAD;

    flashXfer.data          = (uint32_t *) buf;
    flashXfer.dataSize      = len;

    // cppcheck-suppress misra-c2012-11.4
    (void) FLEXSPI_TransferBlocking(FLEXSPI, &flashXfer);

    flash_wait_bus_busy(FLEXSPI);
}
