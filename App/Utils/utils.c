#include "utils.h"

#include <stdint.h>
#include "stm32f4xx_hal.h"

int _write(int file, char *ptr, int len)
{
    for (int i = 0; i < len; i++)
    {
        ITM_SendChar((uint32_t)ptr[i]);
    }
    return len;
}
