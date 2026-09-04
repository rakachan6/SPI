#include "W25Q128.h"
#include "W25Q128_Commands.h"
#include "stm32f4xx_hal.h"
#include <stdio.h>

extern SPI_HandleTypeDef hspi2;

W25Q128_Result_t W25Q128_Check_ID(void)
{
    uint32_t expected_id[3] = {0xEF, 0x40, 0x18};
    uint8_t reset_command[2] = {W25Q128_ENABLE_RESET_COMMAND, W25Q128_RESET_DEVICE_COMMAND};
    uint8_t rx_buffer[4];
    uint8_t tx_buffer[4] = {W25Q128_GET_ID_COMMAND, 0, 0, 0};
    HAL_StatusTypeDef status;

    HAL_Delay (100);
    CS_SELECT(W25Q128_CS_PORT, W25Q128_CS_PIN);
    status = HAL_SPI_Transmit(&hspi2, reset_command, sizeof(reset_command), 100);
    CS_DESELECT(W25Q128_CS_PORT, W25Q128_CS_PIN));
    HAL_Delay (100);

    if (status == HAL_ERROR)
        return W25Q128_ERROR;
    if (status == HAL_TIMEOUT)
        return W25Q128_TIMEOUT;

    CS_SELECT(W25Q128_CS_PORT, W25Q128_CS_PIN);
    status = HAL_SPI_TransmitReceive(&hspi2, tx_buffer, rx_buffer, 4, 100);
    CS_DESELECT(W25Q128_CS_PORT, W25Q128_CS_PIN);

    if (status == HAL_ERROR)
        return W25Q128_ERROR;
    if (status == HAL_TIMEOUT)
        return W25Q128_TIMEOUT;

    printf("ID: %02X %02X %02X\n", rx_buffer[1], rx_buffer[2], rx_buffer[3]);
    printf("Expected: %02X %02X %02X\n", 0xEF, 0x40, 0x18);

    for (int i = 0; i < 3; i++)
    {
        if (rx_buffer[i] != expected_id[i])
            return W25Q128_ERROR;
    }

    return W25Q128_OK;
}

W25Q128_Result_t W25Q128_Read(const uint32_t mem_addr, uint8_t* rx_buffer, const uint32_t len)
{
    if ((mem_addr > W25Q128_CHIP_SIZE) || ((mem_addr + len) > W25Q128_CHIP_SIZE))
        return W25Q128_OUT_OF_RANGE;

    HAL_StatusTypeDef status_tx;
    HAL_StatusTypeDef status_rx;
    uint8_t read_command[4] = {
        W25Q128_READ_DATA_COMMAND, ((mem_addr >> 16) & 0xFF),
        ((mem_addr >> 8) & 0xFF), (mem_addr & 0xFF)
    };

    CS_SELECT(W25Q128_CS_PORT, W25Q128_CS_PIN);
    status_tx = HAL_SPI_Transmit(&hspi2, read_command, sizeof(read_command), sizeof(read_command));
    status_rx = HAL_SPI_Receive(&hspi2, rx_buffer, len, len);
    CS_DESELECT(W25Q128_CS_PORT, W25Q128_CS_PIN);

    if ((status_tx == HAL_TIMEOUT) || (status_rx == HAL_TIMEOUT))
        return W25Q128_TIMEOUT;
    if ((status_tx == HAL_ERROR) || (status_rx == HAL_ERROR))
        return W25Q128_ERROR;

    return W25Q128_OK;
}

W25Q128_Result_t W25Q128_Write_Page(const uint32_t mem_addr, const uint8_t* tx_buffer, const uint16_t len)
{
    if ((mem_addr / W25Q128_PAGE_SIZE) != ((mem_addr + len - 1) / W25Q128_PAGE_SIZE))
        return W25Q128_OUT_OF_RANGE;

    uint32_t start_time;
    HAL_StatusTypeDef status;
    HAL_StatusTypeDef status_2;
    W25Q128_Result_t rx_status;
    uint8_t command = W25Q128_WRITE_ENABLE_COMMAND;
    uint8_t rx_buffer[W25Q128_PAGE_SIZE];
    uint8_t status_reg_val = W25Q128_STATUS_REG_BUSY_VALUE;
    uint8_t write_command[4] = {
        W25Q128_PAGE_PROGRAM_COMMAND,
        (mem_addr >> 16) & 0xFF,
        (mem_addr >> 8) & 0xFF,
        mem_addr & 0xFF
    };

    CS_SELECT(W25Q128_CS_PORT, W25Q128_CS_PIN);
    status = HAL_SPI_Transmit(&hspi2, &command, 1, 1);
    CS_DESELECT(W25Q128_CS_PORT, W25Q128_CS_PIN);

    if (status == HAL_TIMEOUT)
        return W25Q128_TIMEOUT;
    if (status == HAL_ERROR)
        return W25Q128_ERROR;


    CS_SELECT(W25Q128_CS_PORT, W25Q128_CS_PIN);
    status = HAL_SPI_Transmit(&hspi2, write_command, 4, W25Q128_MIN_TIMEOUT);
    status_2 = HAL_SPI_Transmit(&hspi2, tx_buffer, len, len);
    CS_DESELECT(W25Q128_CS_PORT, W25Q128_CS_PIN);

    if ((status == HAL_TIMEOUT) || (status_2 == HAL_TIMEOUT))
        return W25Q128_TIMEOUT;
    if ((status == HAL_ERROR) || (status_2 == HAL_ERROR))
        return W25Q128_ERROR;

    command = W25Q128_READ_STATUS_REG1_COMMAND;
    start_time = HAL_GetTick();
    while (status_reg_val & W25Q128_STATUS_REG_BUSY_VALUE)
    {
        CS_SELECT(W25Q128_CS_PORT, W25Q128_CS_PIN);
        status = HAL_SPI_Transmit(&hspi2, &command, 1, W25Q128_MIN_TIMEOUT);
        status_2 = HAL_SPI_Receive(&hspi2, &status_reg_val, 1, W25Q128_MIN_TIMEOUT);
        CS_DESELECT(W25Q128_CS_PORT, W25Q128_CS_PIN);
        if ((status == HAL_TIMEOUT) || (status_2 == HAL_TIMEOUT))
            return W25Q128_TIMEOUT;
        if ((status == HAL_ERROR) || (status_2 == HAL_ERROR))
            return W25Q128_ERROR;
        if (HAL_GetTick() - start_time > W25Q128_PROGRAM_PAGE_TIMEOUT)
            return W25Q128_TIMEOUT;
    }

    rx_status = W25Q128_Read(mem_addr, rx_buffer, len);
    if (rx_status == W25Q128_TIMEOUT)
        return W25Q128_TIMEOUT;
    if (rx_status == W25Q128_ERROR)
        return W25Q128_ERROR;

    for (uint16_t i = 0; i < len; i++)
    {
        if (tx_buffer[i] != rx_buffer[i])
            return W25Q128_ERROR;
    }

    return W25Q128_OK;
}

W25Q128_Result_t W25Q128_Write(uint32_t mem_addr, const uint8_t* tx_buffer, uint16_t len)
{
    if (mem_addr + len > W25Q128_CHIP_SIZE)
        return W25Q128_OUT_OF_RANGE;

    while (len > 0)
    {
        uint16_t bytes_left_in_page = W25Q128_PAGE_SIZE - (mem_addr % W25Q128_PAGE_SIZE);
        uint16_t current_len = (len < bytes_left_in_page) ? len : bytes_left_in_page;

        W25Q128_Result_t rx_status;
        rx_status = W25Q128_Write_Page(mem_addr, tx_buffer, current_len);

        if (rx_status == W25Q128_TIMEOUT)
            return W25Q128_TIMEOUT;
        if (rx_status == W25Q128_ERROR)
            return W25Q128_ERROR;

        len -= current_len;
        tx_buffer += current_len;
        mem_addr += current_len;
    }
    return W25Q128_OK;
}

W25Q128_Result_t W25Q128_Erase_Chip(void)
{
    uint32_t start_time;
    uint8_t status_reg_val = W25Q128_STATUS_REG_BUSY_VALUE;
    HAL_StatusTypeDef status;
    HAL_StatusTypeDef status_rx;

    uint8_t command = W25Q128_WRITE_ENABLE_COMMAND;
    CS_SELECT(W25Q128_CS_PORT, W25Q128_CS_PIN);
    status = HAL_SPI_Transmit(&hspi2, &command, 1, W25Q128_MIN_TIMEOUT);
    CS_DESELECT(W25Q128_CS_PORT, W25Q128_CS_PIN);

    if (status == HAL_TIMEOUT)
        return W25Q128_TIMEOUT;
    if (status == HAL_ERROR)
        return W25Q128_ERROR;

    command = W25Q128_CHIP_ERASE_COMMAND;
    CS_SELECT(W25Q128_CS_PORT, W25Q128_CS_PIN);
    status = HAL_SPI_Transmit(&hspi2, &command, 1, W25Q128_MIN_TIMEOUT);
    CS_DESELECT(W25Q128_CS_PORT, W25Q128_CS_PIN);

    if (status == HAL_TIMEOUT)
        return W25Q128_TIMEOUT;
    if (status == HAL_ERROR)
        return W25Q128_ERROR;

    command = W25Q128_READ_STATUS_REG1_COMMAND;
    start_time = HAL_GetTick();
    while (status_reg_val & W25Q128_STATUS_REG_BUSY_VALUE)
    {
        CS_SELECT(W25Q128_CS_PORT, W25Q128_CS_PIN);
        status = HAL_SPI_Transmit(&hspi2, &command, 1, W25Q128_MIN_TIMEOUT);
        status_rx = HAL_SPI_Receive(&hspi2, &status_reg_val, 1, W25Q128_MIN_TIMEOUT);
        CS_DESELECT(W25Q128_CS_PORT, W25Q128_CS_PIN);

        if ((status == HAL_TIMEOUT) || (status_rx == HAL_TIMEOUT))
            return W25Q128_TIMEOUT;
        if ((status == HAL_ERROR) || (status_rx == HAL_ERROR))
            return W25Q128_ERROR;

        if (HAL_GetTick() - start_time > W25Q128_ERASE_CHIP_TIMEOUT)
            return W25Q128_TIMEOUT;
    }

    return W25Q128_OK;
}

W25Q128_Result_t W25Q128_Erase_Block_32kb(uint32_t block_addr)
{
    if (block_addr >= W25Q128_CHIP_SIZE)
        return W25Q128_OUT_OF_RANGE;

    uint32_t start_time;
    uint8_t command[4] = {W25Q128_WRITE_ENABLE_COMMAND, 0};
    uint8_t status_reg_val = W25Q128_STATUS_REG_BUSY_VALUE;
    HAL_StatusTypeDef status;
    HAL_StatusTypeDef status_rx;

    CS_SELECT(W25Q128_CS_PORT, W25Q128_CS_PIN);
    status = HAL_SPI_Transmit(&hspi2, command, 1, W25Q128_MIN_TIMEOUT);
    CS_DESELECT(W25Q128_CS_PORT, W25Q128_CS_PIN);
    if (status == HAL_TIMEOUT)
        return W25Q128_TIMEOUT;
    if (status == HAL_ERROR)
        return W25Q128_ERROR;

    command[0] = W25Q128_BLOCK_ERASE_32KB_COMMAND;
    command[1] = (block_addr >> 16) & 0xFF;
    command[2] = (block_addr >> 8) & 0xFF;
    command[3] = block_addr & 0xFF;

    CS_SELECT(W25Q128_CS_PORT, W25Q128_CS_PIN);
    status = HAL_SPI_Transmit(&hspi2, command, 4, W25Q128_MIN_TIMEOUT);
    CS_DESELECT(W25Q128_CS_PORT, W25Q128_CS_PIN);
    if (status == HAL_TIMEOUT)
        return W25Q128_TIMEOUT;
    if (status == HAL_ERROR)
        return W25Q128_ERROR;

    command[0] = W25Q128_READ_STATUS_REG1_COMMAND;
    start_time = HAL_GetTick();

    while (status_reg_val & W25Q128_STATUS_REG_BUSY_VALUE)
    {
        CS_SELECT(W25Q128_CS_PORT, W25Q128_CS_PIN);
        status = HAL_SPI_Transmit(&hspi2, command, 1, W25Q128_MIN_TIMEOUT);
        status_rx = HAL_SPI_Receive(&hspi2, &status_reg_val, 1, W25Q128_MIN_TIMEOUT);
        CS_DESELECT(W25Q128_CS_PORT, W25Q128_CS_PIN);
        if ((status == HAL_TIMEOUT) || (status_rx == HAL_TIMEOUT))
            return W25Q128_TIMEOUT;
        if ((status == HAL_ERROR || (status_rx == HAL_ERROR)))
            return W25Q128_ERROR;

        if (HAL_GetTick() - start_time > W25Q128_ERASE_BLOCK_32KB_TIMEOUT)
            return W25Q128_TIMEOUT;
    }
    return W25Q128_OK;
}
W25Q128_Result_t W25Q128_Erase_Block_64kb(uint32_t block_addr)
{
    if (block_addr >= W25Q128_CHIP_SIZE)
        return W25Q128_OUT_OF_RANGE;

    uint32_t start_time;
    uint8_t command[4] = {W25Q128_WRITE_ENABLE_COMMAND, 0};
    uint8_t status_reg_val = W25Q128_STATUS_REG_BUSY_VALUE;
    HAL_StatusTypeDef status;
    HAL_StatusTypeDef status_rx;

    CS_SELECT(W25Q128_CS_PORT, W25Q128_CS_PIN);
    status = HAL_SPI_Transmit(&hspi2, command, 1, W25Q128_MIN_TIMEOUT);
    CS_DESELECT(W25Q128_CS_PORT, W25Q128_CS_PIN);
    if (status == HAL_TIMEOUT)
        return W25Q128_TIMEOUT;
    if (status == HAL_ERROR)
        return W25Q128_ERROR;

    command[0] = W25Q128_BLOCK_ERASE_64KB_COMMAND;
    command[1] = (block_addr >> 16) & 0xFF;
    command[2] = (block_addr >> 8) & 0xFF;
    command[3] = block_addr & 0xFF;

    CS_SELECT(W25Q128_CS_PORT, W25Q128_CS_PIN);
    status = HAL_SPI_Transmit(&hspi2, command, 4, W25Q128_MIN_TIMEOUT);
    CS_DESELECT(W25Q128_CS_PORT, W25Q128_CS_PIN);
    if (status == HAL_TIMEOUT)
        return W25Q128_TIMEOUT;
    if (status == HAL_ERROR)
        return W25Q128_ERROR;

    command[0] = W25Q128_READ_STATUS_REG1_COMMAND;
    start_time = HAL_GetTick();
    while (status_reg_val & W25Q128_STATUS_REG_BUSY_VALUE)
    {
        CS_SELECT(W25Q128_CS_PORT, W25Q128_CS_PIN);
        status = HAL_SPI_Transmit(&hspi2, command, 1, W25Q128_MIN_TIMEOUT);
        status_rx = HAL_SPI_Receive(&hspi2, &status_reg_val, 1, W25Q128_MIN_TIMEOUT);
        CS_DESELECT(W25Q128_CS_PORT, W25Q128_CS_PIN);
        if ((status == HAL_TIMEOUT) || (status_rx == HAL_TIMEOUT))
            return W25Q128_TIMEOUT;
        if ((status == HAL_ERROR || (status_rx == HAL_ERROR)))
            return W25Q128_ERROR;

        if (HAL_GetTick() - start_time > W25Q128_ERASE_BLOCK_64KB_TIMEOUT)
            return W25Q128_TIMEOUT;
    }
    return W25Q128_OK;
}
W25Q128_Result_t W25Q128_Erase_Sector(uint32_t sector_addr)
{
    if (sector_addr >= W25Q128_CHIP_SIZE)
        return W25Q128_OUT_OF_RANGE;

    uint32_t start_time;
    uint8_t command[4] = {W25Q128_WRITE_ENABLE_COMMAND, 0};
    uint8_t status_reg_val = W25Q128_STATUS_REG_BUSY_VALUE;
    HAL_StatusTypeDef status;
    HAL_StatusTypeDef status_rx;

    CS_SELECT(W25Q128_CS_PORT, W25Q128_CS_PIN);
    status = HAL_SPI_Transmit(&hspi2, command, 1, W25Q128_MIN_TIMEOUT);
    CS_DESELECT(W25Q128_CS_PORT, W25Q128_CS_PIN);
    if (status == HAL_TIMEOUT)
        return W25Q128_TIMEOUT;
    if (status == HAL_ERROR)
        return W25Q128_ERROR;

    command[0] = W25Q128_SECTOR_ERASE_4KB_COMMAND;
    command[1] = (sector_addr >> 16) & 0xFF;
    command[2] = (sector_addr >> 8) & 0xFF;
    command[3] = sector_addr & 0xFF;

    CS_SELECT(W25Q128_CS_PORT, W25Q128_CS_PIN);
    status = HAL_SPI_Transmit(&hspi2, command, 4, W25Q128_MIN_TIMEOUT);
    CS_DESELECT(W25Q128_CS_PORT, W25Q128_CS_PIN);
    if (status == HAL_TIMEOUT)
        return W25Q128_TIMEOUT;
    if (status == HAL_ERROR)
        return W25Q128_ERROR;

    command[0] = W25Q128_READ_STATUS_REG1_COMMAND;
    start_time = HAL_GetTick();

    while (status_reg_val & W25Q128_STATUS_REG_BUSY_VALUE)
    {
        CS_SELECT(W25Q128_CS_PORT, W25Q128_CS_PIN);
        status = HAL_SPI_Transmit(&hspi2, command, 1, W25Q128_MIN_TIMEOUT);
        status_rx = HAL_SPI_Receive(&hspi2, &status_reg_val, 1, W25Q128_MIN_TIMEOUT);
        CS_DESELECT(W25Q128_CS_PORT, W25Q128_CS_PIN);
        if ((status == HAL_TIMEOUT) || (status_rx == HAL_TIMEOUT))
            return W25Q128_TIMEOUT;
        if ((status == HAL_ERROR || (status_rx == HAL_ERROR)))
            return W25Q128_ERROR;

        if (HAL_GetTick() - start_time > W25Q128_ERASE_SECTOR_TIMEOUT)
            return W25Q128_TIMEOUT;
    }
    return W25Q128_OK;
}