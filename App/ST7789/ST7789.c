#include "ST7789.h"
#include "ST7789_Commands.h"
#include "SPI_Driver.h"
#include "stm32f4xx_hal.h"

static SPI_Bus_t * volatile color_fill_spi_bus;

static uint16_t current_row_amount = ST7789_MAX_ROW;
static uint16_t current_column_amount = ST7789_MAX_COL;
static uint8_t color_buffer[(ST7789_MAX_COL + 1) * 2];

static volatile uint8_t color_fill_flag = 0;

static inline void ST7789_DC_Command(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
}

static inline void ST7789_DC_Data(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
}

static ST7789_Result_t ST7789_Convert_result(HAL_StatusTypeDef status)
{
    if (status == HAL_TIMEOUT)
        return ST7789_TIMEOUT;
    if (status == HAL_ERROR)
        return ST7789_ERROR;

    return ST7789_ERROR;
}

static inline void ST7789_Fill_Color_Continue(SPI_Bus_t *bus)
{
    static uint16_t i = 1;
    if (i < current_row_amount)
    {
        i++;
        HAL_StatusTypeDef status;
        status = HAL_SPI_Transmit_DMA(bus->hspi, color_buffer, (current_column_amount * 2));
        if (status != HAL_OK)
        {
            color_fill_flag = 0;
            i = 1;
            SPI_CS_Deselect(ST7789_CS_PORT, ST7789_CS_PIN);
            SPI_Bus_Free(bus);
            //Error Handler
            return;
        }
    }
    else
    {
        color_fill_flag = 0;
        i = 1;
        uint32_t guard = ST7789_BUSY_WAIT_LIMIT;
        while ((__HAL_SPI_GET_FLAG(bus->hspi, SPI_FLAG_BSY)) &&
                guard)
            guard--;
        SPI_CS_Deselect(ST7789_CS_PORT, ST7789_CS_PIN);
        SPI_Bus_Free(bus);
        return;
    }
}

ST7789_Result_t ST7789_Init(SPI_Bus_t *bus)
{
    ST7789_Result_t status;

    status = ST7789_Reset(bus);
    if (status != ST7789_OK)
        return status;

    status = ST7789_Sleep_Out(bus);
    if (status != ST7789_OK)
        return status;

    status = ST7789_Set_Pixel_Format_565(bus);
    if (status != ST7789_OK)
        return status;

    status = ST7789_Display_On(bus);
    if (status != ST7789_OK)
        return status;

    return ST7789_OK;
}

ST7789_Result_t ST7789_Reset(SPI_Bus_t *bus)
{
    HAL_StatusTypeDef status;
    uint8_t command = ST7789_SW_RESET_COMMAND;
    SPI_Bus_Result_t SPI_res;
    SPI_res = SPI_Bus_Capture(bus, ST7789_Dev);
    if (SPI_res != SPI_Result_OK)
        return ST7789_ERROR;

    SPI_CS_Select(ST7789_CS_PORT, ST7789_CS_PIN);
    ST7789_DC_Command(ST7789_DC_PORT, ST7789_DC_PIN);
    status = HAL_SPI_Transmit(bus->hspi, &command, sizeof(command), ST7789_MIN_TIMEOUT);
    SPI_CS_Deselect(ST7789_CS_PORT, ST7789_CS_PIN);

    if (status != HAL_OK)
    {
        SPI_Bus_Free(bus);
        return ST7789_Convert_result(status);
    }

    HAL_Delay(120);

    SPI_Bus_Free(bus);
    return ST7789_OK;
}

ST7789_Result_t ST7789_Display_On(SPI_Bus_t *bus)
{
    HAL_StatusTypeDef status;
    uint8_t command = ST7789_DISPLAY_ON_COMMAND;

    SPI_Bus_Result_t SPI_res;
    SPI_res = SPI_Bus_Capture(bus, ST7789_Dev);
    if (SPI_res != SPI_Result_OK)
        return ST7789_ERROR;

    SPI_CS_Select(ST7789_CS_PORT, ST7789_CS_PIN);
    ST7789_DC_Command(ST7789_DC_PORT, ST7789_DC_PIN);
    status = HAL_SPI_Transmit(bus->hspi, &command, sizeof(command), ST7789_MIN_TIMEOUT);
    SPI_CS_Deselect(ST7789_CS_PORT, ST7789_CS_PIN);

    if (status != HAL_OK)
    {
        SPI_Bus_Free(bus);
        return ST7789_Convert_result(status);
    }

    HAL_Delay(120);

    SPI_Bus_Free(bus);
    return ST7789_OK;
}

ST7789_Result_t ST7789_Sleep_In(SPI_Bus_t *bus)
{
    HAL_StatusTypeDef status;
    uint8_t command = ST7789_SLEEP_IN_COMMAND;

    SPI_Bus_Result_t SPI_res;
    SPI_res = SPI_Bus_Capture(bus, ST7789_Dev);
    if (SPI_res != SPI_Result_OK)
        return ST7789_ERROR;

    SPI_CS_Select(ST7789_CS_PORT, ST7789_CS_PIN);
    ST7789_DC_Command(ST7789_DC_PORT, ST7789_DC_PIN);
    status = HAL_SPI_Transmit(bus->hspi, &command, sizeof(command), ST7789_MIN_TIMEOUT);
    SPI_CS_Deselect(ST7789_CS_PORT, ST7789_CS_PIN);

    if (status != HAL_OK)
    {
        SPI_Bus_Free(bus);
        return ST7789_Convert_result(status);
    }

    SPI_Bus_Free(bus);
    return ST7789_OK;
}
ST7789_Result_t ST7789_Sleep_Out(SPI_Bus_t *bus)
{
    HAL_StatusTypeDef status;
    uint8_t command = ST7789_SLEEP_OUT_COMMAND;

    SPI_Bus_Result_t SPI_res;
    SPI_res = SPI_Bus_Capture(bus, ST7789_Dev);
    if (SPI_res != SPI_Result_OK)
        return ST7789_ERROR;

    SPI_CS_Select(ST7789_CS_PORT, ST7789_CS_PIN);
    ST7789_DC_Command(ST7789_DC_PORT, ST7789_DC_PIN);
    status = HAL_SPI_Transmit(bus->hspi, &command, sizeof(command), ST7789_MIN_TIMEOUT);
    SPI_CS_Deselect(ST7789_CS_PORT, ST7789_CS_PIN);

    if (status != HAL_OK)
    {
        SPI_Bus_Free(bus);
        return ST7789_Convert_result(status);
    }

    HAL_Delay(120);

    SPI_Bus_Free(bus);
    return ST7789_OK;
}

ST7789_Result_t ST7789_Set_Pixel_Format_565(SPI_Bus_t *bus)
{
    HAL_StatusTypeDef status;
    uint8_t command = ST7789_COLMOD_COMMAND;
    uint8_t data = ST7789_SET_PIXEL_FORMAT_565;

    SPI_Bus_Result_t SPI_res;
    SPI_res = SPI_Bus_Capture(bus, ST7789_Dev);
    if (SPI_res != SPI_Result_OK)
        return ST7789_ERROR;

    SPI_CS_Select(ST7789_CS_PORT, ST7789_CS_PIN);
    ST7789_DC_Command(ST7789_DC_PORT, ST7789_DC_PIN);
    status = HAL_SPI_Transmit(bus->hspi, &command, sizeof(command), ST7789_MIN_TIMEOUT);
    if (status != HAL_OK)
    {
        SPI_CS_Deselect(ST7789_CS_PORT, ST7789_CS_PIN);
        SPI_Bus_Free(bus);
        return ST7789_Convert_result(status);
    }

    ST7789_DC_Data(ST7789_DC_PORT, ST7789_DC_PIN);
    status = HAL_SPI_Transmit(bus->hspi, &data, sizeof(data), ST7789_MIN_TIMEOUT);
    SPI_CS_Deselect(ST7789_CS_PORT, ST7789_CS_PIN);

    SPI_Bus_Free(bus);
    if (status != HAL_OK)
        return ST7789_Convert_result(status);

    return ST7789_OK;
}

ST7789_Result_t ST7789_Set_Columns(SPI_Bus_t *bus, const uint16_t start_col, const uint16_t end_col)
{
    HAL_StatusTypeDef status_command;
    HAL_StatusTypeDef status_data;
    uint8_t command = ST7789_COL_ADDR_SET_COMMAND;
    uint8_t data[4] = {
        (start_col >> 8) & 0xFF,
        (start_col) & 0xFF,
        (end_col >> 8) & 0xFF,
        (end_col) & 0xFF
    };

    SPI_Bus_Result_t SPI_res;
    SPI_res = SPI_Bus_Capture(bus, ST7789_Dev);
    if (SPI_res != SPI_Result_OK)
        return ST7789_ERROR;

    SPI_CS_Select(ST7789_CS_PORT, ST7789_CS_PIN);
    ST7789_DC_Command(ST7789_DC_PORT, ST7789_DC_PIN);
    status_command = HAL_SPI_Transmit(bus->hspi, &command, sizeof(command), ST7789_MIN_TIMEOUT);
    ST7789_DC_Data(ST7789_DC_PORT, ST7789_DC_PIN);
    status_data = HAL_SPI_Transmit(bus->hspi, data, sizeof(data), ST7789_MIN_TIMEOUT);
    SPI_CS_Deselect(ST7789_CS_PORT, ST7789_CS_PIN);

    SPI_Bus_Free(bus);
    if ((status_command == HAL_TIMEOUT) || (status_data == HAL_TIMEOUT))
        return ST7789_TIMEOUT;
    if ((status_command == HAL_ERROR) || (status_data == HAL_ERROR))
        return ST7789_ERROR;

    current_column_amount = end_col - start_col + 1;


    return ST7789_OK;
}

ST7789_Result_t ST7789_Set_Rows(SPI_Bus_t *bus, const uint16_t start_row, const uint16_t end_row)
{
    HAL_StatusTypeDef status_command;
    HAL_StatusTypeDef status_data;
    uint8_t command = ST7789_ROW_ADDR_SET_COMMAND;
    uint8_t data[4] = {
        (start_row >> 8) & 0xFF,
        (start_row & 0xFF),
        (end_row >> 8) & 0xFF,
        (end_row & 0xFF)
    };

    SPI_Bus_Result_t SPI_res;
    SPI_res = SPI_Bus_Capture(bus, ST7789_Dev);
    if (SPI_res != SPI_Result_OK)
        return ST7789_ERROR;

    SPI_CS_Select(ST7789_CS_PORT, ST7789_CS_PIN);
    ST7789_DC_Command(ST7789_DC_PORT, ST7789_DC_PIN);
    status_command = HAL_SPI_Transmit(bus->hspi, &command, sizeof(command), ST7789_MIN_TIMEOUT);
    ST7789_DC_Data(ST7789_DC_PORT, ST7789_DC_PIN);
    status_data = HAL_SPI_Transmit(bus->hspi, data, sizeof(data), ST7789_MIN_TIMEOUT);
    SPI_CS_Deselect(ST7789_CS_PORT, ST7789_CS_PIN);

    SPI_Bus_Free(bus);
    if ((status_command == HAL_TIMEOUT) || (status_data == HAL_TIMEOUT))
        return ST7789_TIMEOUT;
    if ((status_command == HAL_ERROR) || (status_data == HAL_ERROR))
        return ST7789_ERROR;

    current_row_amount = end_row - start_row + 1;

    return ST7789_OK;
}

ST7789_Result_t ST7789_Fill_Color(SPI_Bus_t *bus,
                                    const uint16_t start_col, const uint16_t end_col,
                                    const uint16_t start_row, const uint16_t end_row,
                                    const uint16_t color)
{
    if ((start_col > end_col) || (start_row > end_row) ||
        (start_col > ST7789_MAX_COL) || (start_row > ST7789_MAX_ROW) ||
        (end_row > ST7789_MAX_ROW) || (end_col > ST7789_MAX_COL))
        return ST7789_ERROR;
    if (color_fill_flag)
        return ST7789_ERROR;

    HAL_StatusTypeDef status;
    ST7789_Result_t result;
    uint8_t command = ST7789_MEM_WRITE_COMMAND;

    result = ST7789_Set_Rows(bus, start_row, end_row);
    if (result != ST7789_OK)
        return result;
    result = ST7789_Set_Columns(bus, start_col, end_col);
    if (result != ST7789_OK)
        return result;

    for (uint16_t j = 0; j < current_column_amount; j++)
    {
        color_buffer[j * 2] = (color >> 8) & 0xFF;
        color_buffer[j * 2 + 1] = (color & 0xFF);
    }

    SPI_Bus_Result_t SPI_res;
    SPI_res = SPI_Bus_Capture(bus, ST7789_Dev);
    if (SPI_res != SPI_Result_OK)
        return ST7789_ERROR;

    SPI_CS_Select(ST7789_CS_PORT, ST7789_CS_PIN);
    ST7789_DC_Command(ST7789_DC_PORT, ST7789_DC_PIN);
    status = HAL_SPI_Transmit(bus->hspi, &command, sizeof(command), ST7789_MIN_TIMEOUT);
    if (status != HAL_OK)
    {
        SPI_Bus_Free(bus);
        SPI_CS_Deselect(ST7789_CS_PORT, ST7789_CS_PIN);
        return ST7789_Convert_result(status);
    }
    ST7789_DC_Data(ST7789_DC_PORT, ST7789_DC_PIN);
    color_fill_flag = 1;
    color_fill_spi_bus = bus;
    status = HAL_SPI_Transmit_DMA(bus->hspi, color_buffer, (current_column_amount * 2));
    if (status != HAL_OK)
    {
        color_fill_flag = 0;
        SPI_Bus_Free(bus);
        SPI_CS_Deselect(ST7789_CS_PORT, ST7789_CS_PIN);
        return ST7789_Convert_result(status);
    }
    return ST7789_OK;
}


void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance == SPI2)
    {
        if (color_fill_flag)
        {
            ST7789_Fill_Color_Continue(color_fill_spi_bus);
        }
    }
}