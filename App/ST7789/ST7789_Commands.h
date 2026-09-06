#ifndef HAL_SPI_ST7789_COMMANDS_H
#define HAL_SPI_ST7789_COMMANDS_H

#define ST7789_SW_RESET_COMMAND                 (0x01)
#define ST7789_SLEEP_IN_COMMAND                 (0x10)
#define ST7789_SLEEP_OUT_COMMAND                (0x11)
#define ST7789_NORMAL_DISP_ON_COMMAND           (0x13)
#define ST7789_INVERSION_ON_COMMAND             (0x20)
#define ST7789_INVERSION_OFF_COMMAND            (0x21)
#define ST7789_DISPLAY_OFF_COMMAND              (0x28)
#define ST7789_DISPLAY_ON_COMMAND               (0x29)
#define ST7789_COL_ADDR_SET_COMMAND             (0x2A)
#define ST7789_ROW_ADDR_SET_COMMAND             (0x2B)
#define ST7789_MEM_WRITE_COMMAND                (0x2C)
#define ST7788_MADCTL_COMMAND                   (0x36)
#define ST7789_COLMOD_COMMAND                   (0x3A)
#define ST7789_SET_PIXEL_FORMAT_565     (0x55)
#endif //HAL_SPI_ST7789_COMMANDS_H
