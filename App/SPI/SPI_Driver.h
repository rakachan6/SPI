#ifndef HAL_SPI_SPI_DRIVER_H
#define HAL_SPI_SPI_DRIVER_H
#include <stdint.h>

#include "stm32f4xx_hal.h"


typedef enum
{
    SPI_Result_OK,
    SPI_Result_ERROR,
} SPI_Bus_Result_t;

typedef enum
{
    SPI_Bus_Idle,
    SPI_Bus_Busy
} SPI_Bus_State_t;

typedef enum
{
    ST7789_Dev,
    W25Q128_Dev,
} SPI_Device_t;

typedef struct
{
    SPI_HandleTypeDef *hspi;
    SPI_Device_t current_dev_config;
    SPI_Bus_State_t bus_state;
} SPI_Bus_t;

static inline void SPI_CS_Select(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_RESET);
}
static inline void SPI_CS_Deselect(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    HAL_GPIO_WritePin(GPIOx, GPIO_Pin, GPIO_PIN_SET);
}

SPI_Bus_Result_t SPI_Bus_Capture(SPI_Bus_t *bus, SPI_Device_t device);
SPI_Bus_Result_t SPI_Bus_Free(SPI_Bus_t *bus);




#endif //HAL_SPI_SPI_DRIVER_H
