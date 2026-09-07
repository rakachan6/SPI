#include "W25Q128.h"
#include "W25Q128_Commands.h"
#include "stm32f4xx_hal.h"
#include <stdio.h>

#include "SPI_Driver.h"

static W25Q128_Result_t W25Q128_Convert_Status(HAL_StatusTypeDef status)
{
    if (status == HAL_TIMEOUT)
        return W25Q128_TIMEOUT;
    if (status == HAL_ERROR)
        return W25Q128_ERROR;

    return  W25Q128_ERROR;
}

W25Q128_Result_t W25Q128_Check_ID(SPI_Bus_t *bus)
{
    SPI_Bus_Result_t SPI_result;
    SPI_result = SPI_Bus_Capture(bus, W25Q128_Dev);
    if (SPI_result != SPI_Result_OK)
        return W25Q128_ERROR;
    uint32_t expected_id[3] = {0xEF, 0x40, 0x18};
    uint8_t reset_command[2] = {W25Q128_ENABLE_RESET_COMMAND, W25Q128_RESET_DEVICE_COMMAND};
    uint8_t rx_buffer[4];
    uint8_t tx_buffer[4] = {W25Q128_GET_ID_COMMAND, 0, 0, 0};
    HAL_StatusTypeDef status;

    HAL_Delay (100);
    SPI_CS_Select(W25Q128_CS_PORT, W25Q128_CS_PIN);
    status = HAL_SPI_Transmit(bus->hspi, reset_command, sizeof(reset_command), 100);
    SPI_CS_Deselect(W25Q128_CS_PORT, W25Q128_CS_PIN);
    HAL_Delay (100);

    if (status != HAL_OK)
    {
        SPI_Bus_Free(bus);
        return W25Q128_Convert_Status(status);
    }

    SPI_CS_Select(W25Q128_CS_PORT, W25Q128_CS_PIN);
    status = HAL_SPI_TransmitReceive(bus->hspi, tx_buffer, rx_buffer, sizeof(rx_buffer), 100);
    SPI_CS_Deselect(W25Q128_CS_PORT, W25Q128_CS_PIN);

    SPI_Bus_Free(bus);
    if (status != HAL_OK)
        return W25Q128_Convert_Status(status);

    printf("ID: %02X %02X %02X\n", rx_buffer[1], rx_buffer[2], rx_buffer[3]);
    printf("Expected: %02X %02X %02X\n", 0xEF, 0x40, 0x18);

    for (int i = 0; i < 3; i++)
    {
        if (rx_buffer[i + 1] != expected_id[i])
            return W25Q128_ERROR;
    }

    return W25Q128_OK;
}

W25Q128_Result_t W25Q128_Read(SPI_Bus_t *bus, const uint32_t mem_addr, uint8_t* rx_buffer, const uint32_t len)
{
    SPI_Bus_Result_t SPI_result;
    SPI_result = SPI_Bus_Capture(bus, W25Q128_Dev);
    if (SPI_result != SPI_Result_OK)
        return W25Q128_ERROR;

    if ((mem_addr > W25Q128_CHIP_SIZE) || ((mem_addr + len) > W25Q128_CHIP_SIZE))
    {
        SPI_Bus_Free(bus);
        return W25Q128_OUT_OF_RANGE;
    }

    HAL_StatusTypeDef status_tx;
    HAL_StatusTypeDef status_rx;
    uint8_t read_command[4] = {
        W25Q128_READ_DATA_COMMAND, ((mem_addr >> 16) & 0xFF),
        ((mem_addr >> 8) & 0xFF), (mem_addr & 0xFF)
    };

    SPI_CS_Select(W25Q128_CS_PORT, W25Q128_CS_PIN);
    status_tx = HAL_SPI_Transmit(bus->hspi, read_command, sizeof(read_command), W25Q128_MIN_TIMEOUT);
    status_rx = HAL_SPI_Receive(bus->hspi, rx_buffer, len, len);
    SPI_CS_Deselect(W25Q128_CS_PORT, W25Q128_CS_PIN);

    SPI_Bus_Free(bus);
    if (status_tx != HAL_OK)
        return W25Q128_Convert_Status(status_tx);
    if (status_rx != HAL_OK)
        return W25Q128_Convert_Status(status_rx);

    return W25Q128_OK;
}

W25Q128_Result_t W25Q128_Write_Page(SPI_Bus_t *bus, const uint32_t mem_addr, const uint8_t* tx_buffer, const uint16_t len)
{
    SPI_Bus_Result_t SPI_result;
    SPI_result = SPI_Bus_Capture(bus, W25Q128_Dev);
    if (SPI_result != SPI_Result_OK)
        return W25Q128_ERROR;

    if ((mem_addr / W25Q128_PAGE_SIZE) != ((mem_addr + len - 1) / W25Q128_PAGE_SIZE))
    {
        SPI_Bus_Free(bus);
        return W25Q128_OUT_OF_RANGE;
    }

    uint32_t start_time;
    HAL_StatusTypeDef status;
    HAL_StatusTypeDef status_2;
    W25Q128_Result_t status_rx;
    uint8_t command = W25Q128_WRITE_ENABLE_COMMAND;
    uint8_t rx_buffer[W25Q128_PAGE_SIZE];
    uint8_t status_reg_val = W25Q128_STATUS_REG_BUSY_VALUE;
    uint8_t write_command[4] = {
        W25Q128_PAGE_PROGRAM_COMMAND,
        (mem_addr >> 16) & 0xFF,
        (mem_addr >> 8) & 0xFF,
        mem_addr & 0xFF
    };

    SPI_CS_Select(W25Q128_CS_PORT, W25Q128_CS_PIN);
    status = HAL_SPI_Transmit(bus->hspi, &command, 1, W25Q128_MIN_TIMEOUT);
    SPI_CS_Deselect(W25Q128_CS_PORT, W25Q128_CS_PIN);

    if (status != HAL_OK)
    {
        SPI_Bus_Free(bus);
        return W25Q128_Convert_Status(status);
    }


    SPI_CS_Select(W25Q128_CS_PORT, W25Q128_CS_PIN);
    status = HAL_SPI_Transmit(bus->hspi, write_command, 4, W25Q128_MIN_TIMEOUT);
    status_2 = HAL_SPI_Transmit(bus->hspi, tx_buffer, len, len);
    SPI_CS_Deselect(W25Q128_CS_PORT, W25Q128_CS_PIN);

    if (status != HAL_OK)
    {
        SPI_Bus_Free(bus);
        return W25Q128_Convert_Status(status);
    }
    if (status_2 != HAL_OK)
    {
        SPI_Bus_Free(bus);
        return W25Q128_Convert_Status(status_2);
    }


    command = W25Q128_READ_STATUS_REG1_COMMAND;
    start_time = HAL_GetTick();
    while (status_reg_val & W25Q128_STATUS_REG_BUSY_VALUE)
    {
        SPI_CS_Select(W25Q128_CS_PORT, W25Q128_CS_PIN);
        status = HAL_SPI_Transmit(bus->hspi, &command, 1, W25Q128_MIN_TIMEOUT);
        status_2 = HAL_SPI_Receive(bus->hspi, &status_reg_val, 1, W25Q128_MIN_TIMEOUT);
        SPI_CS_Deselect(W25Q128_CS_PORT, W25Q128_CS_PIN);

        if (status != HAL_OK)
        {
            SPI_Bus_Free(bus);
            return W25Q128_Convert_Status(status);
        }
        if (status_2 != HAL_OK)
        {
            SPI_Bus_Free(bus);
            return W25Q128_Convert_Status(status_2);
        }

        if (HAL_GetTick() - start_time > W25Q128_PROGRAM_PAGE_TIMEOUT)
        {
            SPI_Bus_Free(bus);
            return W25Q128_TIMEOUT;
        }
    }
    SPI_Bus_Free(bus);
    status_rx = W25Q128_Read(bus, mem_addr, rx_buffer, len);
    if (status_rx != W25Q128_OK)
        return status_rx;

    for (uint16_t i = 0; i < len; i++)
    {
        if (tx_buffer[i] != rx_buffer[i])
            return W25Q128_ERROR;
    }

    return W25Q128_OK;
}

W25Q128_Result_t W25Q128_Write(SPI_Bus_t *bus, uint32_t mem_addr, const uint8_t* tx_buffer, uint16_t len)
{
    if (mem_addr + len > W25Q128_CHIP_SIZE)
        return W25Q128_OUT_OF_RANGE;

    while (len > 0)
    {
        uint16_t bytes_left_in_page = W25Q128_PAGE_SIZE - (mem_addr % W25Q128_PAGE_SIZE);
        uint16_t current_len = (len < bytes_left_in_page) ? len : bytes_left_in_page;

        W25Q128_Result_t status_rx;
        status_rx = W25Q128_Write_Page(bus, mem_addr, tx_buffer, current_len);

        if (status_rx != W25Q128_OK)
            return status_rx;

        len -= current_len;
        tx_buffer += current_len;
        mem_addr += current_len;
    }
    return W25Q128_OK;
}

W25Q128_Result_t W25Q128_Erase_Chip(SPI_Bus_t *bus)
{
    SPI_Bus_Result_t SPI_result;
    SPI_result = SPI_Bus_Capture(bus, W25Q128_Dev);
    if (SPI_result != SPI_Result_OK)
        return W25Q128_ERROR;

    uint32_t start_time;
    uint8_t status_reg_val = W25Q128_STATUS_REG_BUSY_VALUE;
    HAL_StatusTypeDef status;
    HAL_StatusTypeDef status_rx;

    uint8_t command = W25Q128_WRITE_ENABLE_COMMAND;
    SPI_CS_Select(W25Q128_CS_PORT, W25Q128_CS_PIN);
    status = HAL_SPI_Transmit(bus->hspi, &command, 1, W25Q128_MIN_TIMEOUT);
    SPI_CS_Deselect(W25Q128_CS_PORT, W25Q128_CS_PIN);

    if (status != HAL_OK)
    {
        SPI_Bus_Free(bus);
        return W25Q128_Convert_Status(status);
    }

    command = W25Q128_CHIP_ERASE_COMMAND;
    SPI_CS_Select(W25Q128_CS_PORT, W25Q128_CS_PIN);
    status = HAL_SPI_Transmit(bus->hspi, &command, 1, W25Q128_MIN_TIMEOUT);
    SPI_CS_Deselect(W25Q128_CS_PORT, W25Q128_CS_PIN);

    if (status != HAL_OK)
    {
        SPI_Bus_Free(bus);
        return W25Q128_Convert_Status(status);
    }

    command = W25Q128_READ_STATUS_REG1_COMMAND;
    start_time = HAL_GetTick();
    while (status_reg_val & W25Q128_STATUS_REG_BUSY_VALUE)
    {
        SPI_CS_Select(W25Q128_CS_PORT, W25Q128_CS_PIN);
        status = HAL_SPI_Transmit(bus->hspi, &command, 1, W25Q128_MIN_TIMEOUT);
        status_rx = HAL_SPI_Receive(bus->hspi, &status_reg_val, 1, W25Q128_MIN_TIMEOUT);
        SPI_CS_Deselect(W25Q128_CS_PORT, W25Q128_CS_PIN);

        if (status != HAL_OK)
        {
            SPI_Bus_Free(bus);
            return W25Q128_Convert_Status(status);
        }
        if (status_rx != HAL_OK)
        {
            SPI_Bus_Free(bus);
            return W25Q128_Convert_Status(status_rx);
        }

        if (HAL_GetTick() - start_time > W25Q128_ERASE_CHIP_TIMEOUT)
        {
            SPI_Bus_Free(bus);
            return W25Q128_TIMEOUT;
        }
    }

    SPI_Bus_Free(bus);
    return W25Q128_OK;
}

W25Q128_Result_t W25Q128_Erase_Block_32kb(SPI_Bus_t *bus, uint32_t block_addr)
{
    SPI_Bus_Result_t SPI_result;
    SPI_result = SPI_Bus_Capture(bus, W25Q128_Dev);
    if (SPI_result != SPI_Result_OK)
        return W25Q128_ERROR;

    if (block_addr >= W25Q128_CHIP_SIZE)
    {
        SPI_Bus_Free(bus);
        return W25Q128_OUT_OF_RANGE;
    }

    uint32_t start_time;
    uint8_t command[4] = {W25Q128_WRITE_ENABLE_COMMAND, 0};
    uint8_t status_reg_val = W25Q128_STATUS_REG_BUSY_VALUE;
    HAL_StatusTypeDef status;
    HAL_StatusTypeDef status_rx;

    SPI_CS_Select(W25Q128_CS_PORT, W25Q128_CS_PIN);
    status = HAL_SPI_Transmit(bus->hspi, command, 1, W25Q128_MIN_TIMEOUT);
    SPI_CS_Deselect(W25Q128_CS_PORT, W25Q128_CS_PIN);
    if (status != HAL_OK)
    {
        SPI_Bus_Free(bus);
        return W25Q128_Convert_Status(status);
    }

    command[0] = W25Q128_BLOCK_ERASE_32KB_COMMAND;
    command[1] = (block_addr >> 16) & 0xFF;
    command[2] = (block_addr >> 8) & 0xFF;
    command[3] = block_addr & 0xFF;

    SPI_CS_Select(W25Q128_CS_PORT, W25Q128_CS_PIN);
    status = HAL_SPI_Transmit(bus->hspi, command, 4, W25Q128_MIN_TIMEOUT);
    SPI_CS_Deselect(W25Q128_CS_PORT, W25Q128_CS_PIN);
    if (status != HAL_OK)
    {
        SPI_Bus_Free(bus);
        return W25Q128_Convert_Status(status);
    }

    command[0] = W25Q128_READ_STATUS_REG1_COMMAND;
    start_time = HAL_GetTick();

    while (status_reg_val & W25Q128_STATUS_REG_BUSY_VALUE)
    {
        SPI_CS_Select(W25Q128_CS_PORT, W25Q128_CS_PIN);
        status = HAL_SPI_Transmit(bus->hspi, command, 1, W25Q128_MIN_TIMEOUT);
        status_rx = HAL_SPI_Receive(bus->hspi, &status_reg_val, 1, W25Q128_MIN_TIMEOUT);
        SPI_CS_Deselect(W25Q128_CS_PORT, W25Q128_CS_PIN);

        if (status != HAL_OK)
        {
            SPI_Bus_Free(bus);
            return W25Q128_Convert_Status(status);
        }
        if (status_rx != HAL_OK)
        {
            SPI_Bus_Free(bus);
            return W25Q128_Convert_Status(status_rx);
        }

        if (HAL_GetTick() - start_time > W25Q128_ERASE_BLOCK_32KB_TIMEOUT)
        {
            SPI_Bus_Free(bus);
            return W25Q128_TIMEOUT;
        }
    }

    SPI_Bus_Free(bus);
    return W25Q128_OK;
}
W25Q128_Result_t W25Q128_Erase_Block_64kb(SPI_Bus_t *bus, uint32_t block_addr)
{
    SPI_Bus_Result_t SPI_result;
    SPI_result = SPI_Bus_Capture(bus, W25Q128_Dev);
    if (SPI_result != SPI_Result_OK)
        return W25Q128_ERROR;

    if (block_addr >= W25Q128_CHIP_SIZE)
    {
        SPI_Bus_Free(bus);
        return W25Q128_OUT_OF_RANGE;
    }

    uint32_t start_time;
    uint8_t command[4] = {W25Q128_WRITE_ENABLE_COMMAND, 0};
    uint8_t status_reg_val = W25Q128_STATUS_REG_BUSY_VALUE;
    HAL_StatusTypeDef status;
    HAL_StatusTypeDef status_rx;

    SPI_CS_Select(W25Q128_CS_PORT, W25Q128_CS_PIN);
    status = HAL_SPI_Transmit(bus->hspi, command, 1, W25Q128_MIN_TIMEOUT);
    SPI_CS_Deselect(W25Q128_CS_PORT, W25Q128_CS_PIN);
    if (status != HAL_OK)
    {
        SPI_Bus_Free(bus);
        return W25Q128_Convert_Status(status);
    }

    command[0] = W25Q128_BLOCK_ERASE_64KB_COMMAND;
    command[1] = (block_addr >> 16) & 0xFF;
    command[2] = (block_addr >> 8) & 0xFF;
    command[3] = block_addr & 0xFF;

    SPI_CS_Select(W25Q128_CS_PORT, W25Q128_CS_PIN);
    status = HAL_SPI_Transmit(bus->hspi, command, 4, W25Q128_MIN_TIMEOUT);
    SPI_CS_Deselect(W25Q128_CS_PORT, W25Q128_CS_PIN);
    if (status != HAL_OK)
    {
        SPI_Bus_Free(bus);
        return W25Q128_Convert_Status(status);
    }

    command[0] = W25Q128_READ_STATUS_REG1_COMMAND;
    start_time = HAL_GetTick();
    while (status_reg_val & W25Q128_STATUS_REG_BUSY_VALUE)
    {
        SPI_CS_Select(W25Q128_CS_PORT, W25Q128_CS_PIN);
        status = HAL_SPI_Transmit(bus->hspi, command, 1, W25Q128_MIN_TIMEOUT);
        status_rx = HAL_SPI_Receive(bus->hspi, &status_reg_val, 1, W25Q128_MIN_TIMEOUT);
        SPI_CS_Deselect(W25Q128_CS_PORT, W25Q128_CS_PIN);
        if (status != HAL_OK)
        {
            SPI_Bus_Free(bus);
            return W25Q128_Convert_Status(status);
        }
        if (status_rx != HAL_OK)
        {
            SPI_Bus_Free(bus);
            return W25Q128_Convert_Status(status_rx);
        }

        if (HAL_GetTick() - start_time > W25Q128_ERASE_BLOCK_64KB_TIMEOUT)
        {
            SPI_Bus_Free(bus);
            return W25Q128_TIMEOUT;
        }
    }

    SPI_Bus_Free(bus);
    return W25Q128_OK;
}
W25Q128_Result_t W25Q128_Erase_Sector(SPI_Bus_t *bus, uint32_t sector_addr)
{
    SPI_Bus_Result_t SPI_result;
    SPI_result = SPI_Bus_Capture(bus, W25Q128_Dev);
    if (SPI_result != SPI_Result_OK)
        return W25Q128_ERROR;

    if (sector_addr >= W25Q128_CHIP_SIZE)
    {
        SPI_Bus_Free(bus);
        return W25Q128_OUT_OF_RANGE;
    }

    uint32_t start_time;
    uint8_t command[4] = {W25Q128_WRITE_ENABLE_COMMAND, 0};
    uint8_t status_reg_val = W25Q128_STATUS_REG_BUSY_VALUE;
    HAL_StatusTypeDef status;
    HAL_StatusTypeDef status_rx;

    SPI_CS_Select(W25Q128_CS_PORT, W25Q128_CS_PIN);
    status = HAL_SPI_Transmit(bus->hspi, command, 1, W25Q128_MIN_TIMEOUT);
    SPI_CS_Deselect(W25Q128_CS_PORT, W25Q128_CS_PIN);
    if (status != HAL_OK)
    {
        SPI_Bus_Free(bus);
        return W25Q128_Convert_Status(status);
    }

    command[0] = W25Q128_SECTOR_ERASE_4KB_COMMAND;
    command[1] = (sector_addr >> 16) & 0xFF;
    command[2] = (sector_addr >> 8) & 0xFF;
    command[3] = sector_addr & 0xFF;

    SPI_CS_Select(W25Q128_CS_PORT, W25Q128_CS_PIN);
    status = HAL_SPI_Transmit(bus->hspi, command, 4, W25Q128_MIN_TIMEOUT);
    SPI_CS_Deselect(W25Q128_CS_PORT, W25Q128_CS_PIN);
    if (status != HAL_OK)
    {
        SPI_Bus_Free(bus);
        return W25Q128_Convert_Status(status);
    }

    command[0] = W25Q128_READ_STATUS_REG1_COMMAND;
    start_time = HAL_GetTick();

    while (status_reg_val & W25Q128_STATUS_REG_BUSY_VALUE)
    {
        SPI_CS_Select(W25Q128_CS_PORT, W25Q128_CS_PIN);
        status = HAL_SPI_Transmit(bus->hspi, command, 1, W25Q128_MIN_TIMEOUT);
        status_rx = HAL_SPI_Receive(bus->hspi, &status_reg_val, 1, W25Q128_MIN_TIMEOUT);
        SPI_CS_Deselect(W25Q128_CS_PORT, W25Q128_CS_PIN);
        if (status != HAL_OK)
        {
            SPI_Bus_Free(bus);
            return W25Q128_Convert_Status(status);
        }
        if (status_rx != HAL_OK)
        {
            SPI_Bus_Free(bus);
            return W25Q128_Convert_Status(status_rx);
        }

        if (HAL_GetTick() - start_time > W25Q128_ERASE_SECTOR_TIMEOUT)
        {
            SPI_Bus_Free(bus);
            return W25Q128_TIMEOUT;
        }
    }

    SPI_Bus_Free(bus);
    return W25Q128_OK;
}