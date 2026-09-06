#include "ST7789.h"

#include "ST7789_Commands.h"
#include "stm32f4xx_hal.h"

extern SPI_HandleTypeDef hspi2;

uint16_t current_row_amount = ST7789_MAX_ROW;
uint16_t current_column_amount = ST7789_MAX_COL;
uint8_t color_buffer[(ST7789_MAX_COL + 1) * 2];

volatile uint8_t color_fill_flag = 0;

static inline ST7789_Result_t ST7789_Convert_result(HAL_StatusTypeDef status)
{
    if (status == HAL_TIMEOUT)
        return ST7789_TIMEOUT;
    if (status == HAL_ERROR)
        return ST7789_ERROR;

    return ST7789_ERROR;
}

static inline void ST7789_Fill_Color_Continue(void)
{
    static uint16_t i = 1;
    if (i < current_row_amount)
    {
        i++;
        HAL_StatusTypeDef status;
        status = HAL_SPI_Transmit_DMA(&hspi2, color_buffer, (current_column_amount * 2));
        if (status != HAL_OK)
        {
            color_fill_flag = 0;
            i = 1;
            CS_DESELECT(ST7789_CS_PORT, ST7789_CS_PIN);
            //Error Handler
            return;
        }
    }
    else
    {
        color_fill_flag = 0;
        i = 1;
        CS_DESELECT(ST7789_CS_PORT, ST7789_CS_PIN);
        return;
    }
}

ST7789_Result_t ST7789_Init(void)
{
    ST7789_Result_t status;

    status = ST7789_Reset();
    if (status != ST7789_OK)
        return status;

    status = ST7789_Sleep_Out();
    if (status != ST7789_OK)
        return status;

    status = ST7789_Set_Pixel_Format_565();
    if (status != ST7789_OK)
        return status;

    status = ST7789_Display_On();
    if (status != ST7789_OK)
        return status;

    return ST7789_OK;
}

ST7789_Result_t ST7789_Reset(void)
{
    HAL_StatusTypeDef status;
    uint8_t command = ST7789_SW_RESET_COMMAND;

    CS_SELECT(ST7789_CS_PORT, ST7789_CS_PIN);
    ST7789_DC_COMMAND(ST7789_DC_PORT, ST7789_DC_PIN);
    status = HAL_SPI_Transmit(&hspi2, &command, sizeof(command), ST7789_MIN_TIMEOUT);
    CS_DESELECT(ST7789_CS_PORT, ST7789_CS_PIN);

    if (status != HAL_OK)
        return ST7789_Convert_result(status);

    HAL_Delay(120);

    return ST7789_OK;
}

ST7789_Result_t ST7789_Display_On(void)
{
    HAL_StatusTypeDef status;
    uint8_t command = ST7789_DISPLAY_ON_COMMAND;

    CS_SELECT(ST7789_CS_PORT, ST7789_CS_PIN);
    ST7789_DC_COMMAND(ST7789_DC_PORT, ST7789_DC_PIN);
    status = HAL_SPI_Transmit(&hspi2, &command, sizeof(command), ST7789_MIN_TIMEOUT);
    CS_DESELECT(ST7789_CS_PORT, ST7789_CS_PIN);

    if (status != HAL_OK)
        return ST7789_Convert_result(status);

    HAL_Delay(120);

    return ST7789_OK;
}

ST7789_Result_t ST7789_Sleep_In(void)
{
    HAL_StatusTypeDef status;
    uint8_t command = ST7789_SLEEP_IN_COMMAND;

    CS_SELECT(ST7789_CS_PORT, ST7789_CS_PIN);
    ST7789_DC_COMMAND(ST7789_DC_PORT, ST7789_DC_PIN);
    status = HAL_SPI_Transmit(&hspi2, &command, sizeof(command), ST7789_MIN_TIMEOUT);
    CS_DESELECT(ST7789_CS_PORT, ST7789_CS_PIN);

    if (status != HAL_OK)
        return ST7789_Convert_result(status);

    return ST7789_OK;
}
ST7789_Result_t ST7789_Sleep_Out(void)
{
    HAL_StatusTypeDef status;
    uint8_t command = ST7789_SLEEP_OUT_COMMAND;

    CS_SELECT(ST7789_CS_PORT, ST7789_CS_PIN);
    ST7789_DC_COMMAND(ST7789_DC_PORT, ST7789_DC_PIN);
    status = HAL_SPI_Transmit(&hspi2, &command, sizeof(command), ST7789_MIN_TIMEOUT);
    CS_DESELECT(ST7789_CS_PORT, ST7789_CS_PIN);

    if (status != HAL_OK)
        return ST7789_Convert_result(status);

    HAL_Delay(120);

    return ST7789_OK;
}

ST7789_Result_t ST7789_Set_Pixel_Format_565(void)
{
    HAL_StatusTypeDef status;
    uint8_t command = ST7789_COLMOD_COMMAND;
    uint8_t data = ST7789_SET_PIXEL_FORMAT_565;

    CS_SELECT(ST7789_CS_PORT, ST7789_CS_PIN);
    ST7789_DC_COMMAND(ST7789_DC_PORT, ST7789_DC_PIN);
    status = HAL_SPI_Transmit(&hspi2, &command, sizeof(command), ST7789_MIN_TIMEOUT);
    if (status != HAL_OK)
    {
        CS_DESELECT(ST7789_CS_PORT, ST7789_CS_PIN);
        return ST7789_Convert_result(status);
    }

    ST7789_DC_DATA(ST7789_DC_PORT, ST7789_DC_PIN);
    status = HAL_SPI_Transmit(&hspi2, &data, sizeof(data), ST7789_MIN_TIMEOUT);
    CS_DESELECT(ST7789_CS_PORT, ST7789_CS_PIN);

    if (status != HAL_OK)
        return ST7789_Convert_result(status);

    return ST7789_OK;
}

ST7789_Result_t ST7789_Set_Columns(const uint16_t start_col, const uint16_t end_col)
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
    CS_SELECT(ST7789_CS_PORT, ST7789_CS_PIN);
    ST7789_DC_COMMAND(ST7789_DC_PORT, ST7789_DC_PIN);
    status_command = HAL_SPI_Transmit(&hspi2, &command, sizeof(command), ST7789_MIN_TIMEOUT);
    ST7789_DC_DATA(ST7789_DC_PORT, ST7789_DC_PIN);
    status_data = HAL_SPI_Transmit(&hspi2, data, sizeof(data), ST7789_MIN_TIMEOUT);
    CS_DESELECT(ST7789_CS_PORT, ST7789_CS_PIN);

    if ((status_command == HAL_TIMEOUT) || (status_data == HAL_TIMEOUT))
        return ST7789_TIMEOUT;
    if ((status_command == HAL_ERROR) || (status_data == HAL_ERROR))
        return ST7789_ERROR;

    current_column_amount = end_col - start_col + 1;
    return ST7789_OK;
}

ST7789_Result_t ST7789_Set_Rows(const uint16_t start_row, const uint16_t end_row)
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

    CS_SELECT(ST7789_CS_PORT, ST7789_CS_PIN);
    ST7789_DC_COMMAND(ST7789_DC_PORT, ST7789_DC_PIN);
    status_command = HAL_SPI_Transmit(&hspi2, &command, sizeof(command), ST7789_MIN_TIMEOUT);
    ST7789_DC_DATA(ST7789_DC_PORT, ST7789_DC_PIN);
    status_data = HAL_SPI_Transmit(&hspi2, data, sizeof(data), ST7789_MIN_TIMEOUT);
    CS_DESELECT(ST7789_CS_PORT, ST7789_CS_PIN);

    if ((status_command == HAL_TIMEOUT) || (status_data == HAL_TIMEOUT))
        return ST7789_TIMEOUT;
    if ((status_command == HAL_ERROR) || (status_data == HAL_ERROR))
        return ST7789_ERROR;

    current_row_amount = end_row - start_row + 1;
    return ST7789_OK;
}

ST7789_Result_t ST7789_Fill_Color(const uint16_t start_col, const uint16_t end_col,
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

    result = ST7789_Set_Rows(start_row, end_row);
    if (result != ST7789_OK)
        return result;
    result = ST7789_Set_Columns(start_col, end_col);
    if (result != ST7789_OK)
        return result;

    for (uint16_t j = 0; j < current_column_amount; j++)
    {
        color_buffer[j * 2] = (color >> 8) & 0xFF;
        color_buffer[j * 2 + 1] = (color & 0xFF);
    }

    CS_SELECT(ST7789_CS_PORT, ST7789_CS_PIN);
    ST7789_DC_COMMAND(ST7789_DC_PORT, ST7789_DC_PIN);
    status = HAL_SPI_Transmit(&hspi2, &command, sizeof(command), ST7789_MIN_TIMEOUT);
    if (status != HAL_OK)
    {
        CS_DESELECT(ST7789_CS_PORT, ST7789_CS_PIN);
        return ST7789_Convert_result(status);
    }
    ST7789_DC_DATA(ST7789_DC_PORT, ST7789_DC_PIN);
    color_fill_flag = 1;
    status = HAL_SPI_Transmit_DMA(&hspi2, color_buffer, (current_column_amount * 2));
    if (status != HAL_OK)
    {
        CS_DESELECT(ST7789_CS_PORT, ST7789_CS_PIN);
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
            ST7789_Fill_Color_Continue();
        }
    }
}