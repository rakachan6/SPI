#include "SPI_Driver.h"

static SPI_Bus_Result_t SPI_Bus_Configurate(SPI_Bus_t *bus, const SPI_Device_t device)
{
    if (device == ST7789_Dev)
    {
        //Dev config
        bus->current_dev_config = ST7789_Dev;
        return SPI_Result_OK;
    }
    if (device == W25Q128_Dev)
    {
        //Dev config
        bus->current_dev_config = W25Q128_Dev;
        return SPI_Result_OK;
    }

    //Undefined device
    return SPI_Result_ERROR;
}

SPI_Bus_Result_t SPI_Bus_Capture(SPI_Bus_t *bus, const SPI_Device_t device)
{
    if (bus->bus_state != SPI_Bus_Idle)
        return SPI_Result_ERROR;
    if (bus->current_dev_config != device)
    {
        SPI_Bus_Result_t result;
        result = SPI_Bus_Configurate(bus, device);
        if (result != SPI_Result_OK)
            return result;
    }
    bus->bus_state = SPI_Bus_Busy;
    return SPI_Result_OK;
}

SPI_Bus_Result_t SPI_Bus_Free(SPI_Bus_t *bus)
{
    bus->bus_state = SPI_Bus_Idle;
    return SPI_Result_OK;
}

