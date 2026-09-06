#ifndef HAL_SPI_ST7789_H
#define HAL_SPI_ST7789_H
#include <stdint.h>

#define CS_SELECT(PORT, PIN)            HAL_GPIO_WritePin((PORT), (PIN), GPIO_PIN_RESET)
#define CS_DESELECT(PORT, PIN)          HAL_GPIO_WritePin((PORT), (PIN), GPIO_PIN_SET)
#define ST7789_DC_COMMAND(PORT, PIN)    HAL_GPIO_WritePin((PORT), (PIN), GPIO_PIN_RESET)
#define ST7789_DC_DATA(PORT, PIN)       HAL_GPIO_WritePin((PORT), (PIN), GPIO_PIN_SET)

#define ST7789_CS_PORT                  (GPIOB)
#define ST7789_CS_PIN                   (GPIO_PIN_12)
#define ST7789_DC_PORT                  (GPIOB)
#define ST7789_DC_PIN                   (GPIO_PIN_11)
#define ST7789_MIN_TIMEOUT              (10)
#define ST7789_MAX_COL                  (239)
#define ST7789_MAX_ROW                  (319)

typedef enum
{
    ST7789_OK,
    ST7789_ERROR,
    ST7789_TIMEOUT,
} ST7789_Result_t;

ST7789_Result_t ST7789_Init(void);
ST7789_Result_t ST7789_Reset(void);
ST7789_Result_t ST7789_Display_On(void);
ST7789_Result_t ST7789_Sleep_In(void);
ST7789_Result_t ST7789_Sleep_Out(void);
ST7789_Result_t ST7789_Set_Pixel_Format_565(void);
ST7789_Result_t ST7789_Set_Columns(uint16_t start_col, uint16_t end_col);
ST7789_Result_t ST7789_Set_Rows(uint16_t start_row, uint16_t end_row);
ST7789_Result_t ST7789_Fill_Color(uint16_t start_col, uint16_t end_col,
                                    uint16_t start_row, uint16_t end_row,
                                    uint16_t color);


#endif //HAL_SPI_ST7789_H
