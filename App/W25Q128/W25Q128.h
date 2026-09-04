#ifndef HAL_SPI_W25Q128_H
#define HAL_SPI_W25Q128_H

#include <stdint.h>

#define CS_SELECT(PORT, PIN)    HAL_GPIO_WritePin((PORT), (PIN), GPIO_PIN_RESET)
#define CS_DESELECT(PORT, PIN)  HAL_GPIO_WritePin((PORT), (PIN), GPIO_PIN_SET)
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

W25Q128_Result_t W25Q128_Check_ID(void);
W25Q128_Result_t W25Q128_Read(uint32_t mem_addr, uint8_t* rx_buffer, uint32_t len);
W25Q128_Result_t W25Q128_Write_Page(uint32_t mem_addr, const uint8_t* tx_buffer, uint16_t len);
W25Q128_Result_t W25Q128_Write(uint32_t mem_addr, const uint8_t* tx_buffer, uint16_t len);

W25Q128_Result_t W25Q128_Erase_Sector(uint32_t sector_addr);
W25Q128_Result_t W25Q128_Erase_Block_32kb(uint32_t block_addr);
W25Q128_Result_t W25Q128_Erase_Block_64kb(uint32_t block_addr);
W25Q128_Result_t W25Q128_Erase_Chip(void);


#endif //HAL_SPI_W25Q128_H
