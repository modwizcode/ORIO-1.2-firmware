//**************************************************************************
//
//  Open Robot I/O
//
//    Copyright (C) 2019 John Winans
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <https://www.gnu.org/licenses/>.
//
//**************************************************************************

#include <stdio.h>
#include "board.h"
#include "peripherals.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "LPC54606.h"

#include "fsl_power.h"
#include "fsl_gpio.h"
#include "fsl_spi.h"

#include "hexdump.h"


#if 0
/**
 * @param data Buffer of data to calculate.
 * @param dataSize number of bytes in 'data'
 ******************************************************************/
static uint32_t crc16(const void *data, size_t dataSize)
{
	CRC_1_PERIPHERAL->SEED = 0xffff;
	CRC_WriteData(CRC_1_PERIPHERAL, (const uint8_t*)data, dataSize);
	return CRC_1_PERIPHERAL->SUM;
}
#else

/**
 * Calculate a CRC16 of an array of 16-bit words.
 * The data array is byte-swapped while read.
 *
 * @param data Buffer of data to calculate.
 * @param dataSize number of 16-bit words in 'data'
 ******************************************************************/
static uint32_t crc16_16le(const void *data, size_t dataSize)
{
	CRC_1_PERIPHERAL->SEED = 0xffff;
	uint16_t *p = (uint16_t*)data;
	for (int i=0; i<dataSize; ++i)
		*((__O uint16_t *)&(CRC_1_PERIPHERAL->WR_DATA)) = (p[i]>>8)|(p[i]<<8);
	return CRC_1_PERIPHERAL->SUM;
}
#endif


/**
 * Write the given uint16 to the SPI and also to the CRC generator.
 ******************************************************************/
static void spiTX16(uint16_t i)
{
	SPI_WriteData(SPI_1_PERIPHERAL, i, 0);
#if 0
	*((volatile uint16_t *)&(CRC_1_PERIPHERAL->WR_DATA)) = i;
#else
	*((volatile uint8_t *)&(CRC_1_PERIPHERAL->WR_DATA)) = (uint8_t)(i>>8&0x0ff);
	*((volatile uint8_t *)&(CRC_1_PERIPHERAL->WR_DATA)) = (uint8_t)(i&0x0ff);
#endif
}

/**
 *
 ******************************************************************/
void spitest()
{
	SPI_Type *base = SPI_1_PERIPHERAL;
	printf("SPI message loop entered\n");

	// prime the FIFO for the next transfer
    base->FIFOCFG |= SPI_FIFOCFG_EMPTYTX_MASK | SPI_FIFOCFG_EMPTYRX_MASK;
    base->FIFOSTAT |= SPI_FIFOSTAT_TXERR_MASK | SPI_FIFOSTAT_RXERR_MASK;

    // Write this so that the FIFO doesn't start empty after a reset
    // This doesn't actually seem to matter that much but the docs say to make sure a byte is written before any reads need to occur
    // and we don't want to trust the initialization code with that
    // The fifo (despite being "empty" as in the pointers are reset) still contains old data, so if something sensitive was in there you'd have to manually
    // empty the fifo, then fill it, then empty it, before the next transfer...
    SPI_WriteData(base, 0x12FF, 0);

    // This test code doesn't use CRC
    //CRC_1_PERIPHERAL->SEED = 0xffff;
    int rx_counter = 0;
    int tx_counter = 0;
    uint16_t buffer[256];
	while(1)
	{
		// This is set after a slave select input is deasserted, we use this as a signal that the transfered packet is complete
		// and we can reset to start transfering the next byte
		if (base->STAT & SPI_STAT_SSD_MASK) {
			base->STAT |= SPI_STAT_SSD_MASK;
#if 0
			for (int i = 0; i < rx_counter; i++) {
				printf("%x ", buffer[i]);
			}
			printf("\n");
			printf("%d = %d\n", tx_counter, rx_counter);
#endif
			tx_counter = 0;
			rx_counter =  0;
			memset(buffer, 0xFF, sizeof(buffer));
		}
		if (base->FIFOSTAT & SPI_FIFOSTAT_RXNOTEMPTY_MASK) {
			buffer[rx_counter++ & 0xFF] = (SPI_ReadData(base) & SPI_FIFORD_RXDATA_MASK) >> SPI_FIFORD_RXDATA_SHIFT;
		}
		if (base->FIFOSTAT & SPI_FIFOSTAT_TXNOTFULL_MASK) {
			if (tx_counter < rx_counter) {
				SPI_WriteData(base, buffer[tx_counter++], 0);
			}
		}
	}
}
