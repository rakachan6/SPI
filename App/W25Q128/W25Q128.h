#ifndef HAL_SPI_W25Q128_H
#define HAL_SPI_W25Q128_H

#include <stdint.h>
#include "SPI_Driver.h"

#define W25Q128_CS_PORT         (GPIOB)
#define W25Q128_CS_PIN          (GPIO_PIN_13)

#define W25Q128_PAGE_SIZE       (256)
#define W25Q128_SECTOR_SIZE     (4096)
#define W25Q128_BLOCK_SIZE      (65536)
#define W25Q128_CHIP_SIZE       (16777216)

#define W25Q128_MIN_TIMEOUT     (10)

typedef enum
{
    W25Q128_OK = 0,
    W25Q128_ERROR,
    W25Q128_TIMEOUT,
    W25Q128_OUT_OF_RANGE,
} W25Q128_Result_t;

W25Q128_Result_t W25Q128_Check_ID(SPI_Bus_t *bus);
W25Q128_Result_t W25Q128_Read(SPI_Bus_t *bus, uint32_t mem_addr, uint8_t* rx_buffer, uint32_t len);
W25Q128_Result_t W25Q128_Write_Page(SPI_Bus_t *bus, uint32_t mem_addr, const uint8_t* tx_buffer, uint16_t len);
W25Q128_Result_t W25Q128_Write(SPI_Bus_t *bus, uint32_t mem_addr, const uint8_t* tx_buffer, uint16_t len);

W25Q128_Result_t W25Q128_Erase_Sector(SPI_Bus_t *bus, uint32_t sector_addr);
W25Q128_Result_t W25Q128_Erase_Block_32kb(SPI_Bus_t *bus, uint32_t block_addr);
W25Q128_Result_t W25Q128_Erase_Block_64kb(SPI_Bus_t *bus, uint32_t block_addr);
W25Q128_Result_t W25Q128_Erase_Chip(SPI_Bus_t *bus);


#endif //HAL_SPI_W25Q128_H
