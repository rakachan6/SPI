# STM32 SPI Driver Learning Project.

## Functionality
Implemented display and flash memory control over SPI2.

## Hardware 
 - MCU - STM32F411CEU6 (black pill).
 - Display - ST7789.
 - Flash memory - W25Q128.

## Wiring
|MCU pin | Function  | Connect to                                   |
|--------|-----------|----------------------------------------------|
|PB3     |SWO        |Print to host console                         |
|PB10    |SPI CLK    |Clock pin of the display and flash memory     |
|PB14    |SPI MISO   |MISO pin of the display and flash memory      |
|PB15    |SPI MOSI   |MOSI pin of the display and flash memory      |
|PB13    |W25Q128 CS |CS pin of the flash memory                    |
|PB12    |ST7789 CS  |CS pin of the display                         |
|PA8     |ST7789 DC  |DC pin of the display                         |
|GND     |-          |GND pin of the display and flash memory       |
|VCC     |-          |VCC pin of the display and flash memory       |

## Building
Requirements:
- `arm-none-eabi-gcc` toolchain
- CMake ≥ 3.22
- Ninja
```sh
# Configure (only needed once per preset, or after CMakePresets.json changes)
cmake --preset Release
cmake --build build/Release
```
The resulting binary is at build/Release/HAL_SPI.elf.

## Flashing
Using an ST-Link probe and STM32CubeProgrammer:
```shell
STM32_Programmer_CLI -c port=SWD -w build/Release/HAL_SPI.elf -v -rst
```

