#ifndef HAL_SPI_ST7789_H
#define HAL_SPI_ST7789_H
#include <stdint.h>
#include "SPI_Driver.h"

#define ST7789_CS_PORT                  (GPIOB)
#define ST7789_CS_PIN                   (GPIO_PIN_12)
#define ST7789_DC_PORT                  (GPIOA)
#define ST7789_DC_PIN                   (GPIO_PIN_8)
#define ST7789_MIN_TIMEOUT              (10)
#define ST7789_MAX_COL                  (319)
#define ST7789_MAX_ROW                  (239)
#define ST7789_BUSY_WAIT_LIMIT          (10000)

typedef enum
{
    ST7789_OK,
    ST7789_ERROR,
    ST7789_TIMEOUT,
} ST7789_Result_t;

ST7789_Result_t ST7789_Init(SPI_Bus_t *bus);
ST7789_Result_t ST7789_Reset(SPI_Bus_t *bus);
ST7789_Result_t ST7789_Display_On(SPI_Bus_t *bus);
ST7789_Result_t ST7789_Sleep_In(SPI_Bus_t *bus);
ST7789_Result_t ST7789_Sleep_Out(SPI_Bus_t *bus);
ST7789_Result_t ST7789_Set_Pixel_Format_565(SPI_Bus_t *bus);
ST7789_Result_t ST7789_Set_Columns(SPI_Bus_t *bus, uint16_t start_col, uint16_t end_col);
ST7789_Result_t ST7789_Set_Rows(SPI_Bus_t *bus, uint16_t start_row, uint16_t end_row);
ST7789_Result_t ST7789_Fill_Color(SPI_Bus_t *bus,
                                    uint16_t start_col, uint16_t end_col,
                                    uint16_t start_row, uint16_t end_row,
                                    uint16_t color);


#endif //HAL_SPI_ST7789_H
