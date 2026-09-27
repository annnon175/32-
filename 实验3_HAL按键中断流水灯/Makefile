PREFIX ?= arm-none-eabi-
CC := $(PREFIX)gcc
OBJCOPY := $(PREFIX)objcopy
SIZE := $(PREFIX)size

CPUFLAGS := -mcpu=cortex-m3 -mthumb
DEFINES := -DSTM32F103xB -DUSE_HAL_DRIVER
INCLUDES := -ICore/Inc -IDrivers/CMSIS/Core/Include -IDrivers/CMSIS/Device/ST/STM32F1xx/Include -IDrivers/STM32F1xx_HAL_Driver/Inc
CFLAGS := $(CPUFLAGS) $(DEFINES) $(INCLUDES) -std=c11 -Os -g3 -ffunction-sections -fdata-sections -Wall -Wextra -Werror
LDFLAGS := $(CPUFLAGS) -nostartfiles --specs=nano.specs --specs=nosys.specs -Tlinker/STM32F103C8T6_FLASH.ld -Wl,--gc-sections,-Map=build/exp3_hal_exti.map

SOURCES := Core/Src/main.c Core/Src/stm32f1xx_it.c Core/Src/system_stm32f1xx.c \
           Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal.c \
           Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_cortex.c \
           Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_gpio.c \
           Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc.c \
           Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc_ex.c \
           Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_flash.c \
           Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_flash_ex.c \
           Drivers/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_pwr.c

OBJECTS := $(patsubst %.c,build/%.o,$(SOURCES)) build/startup.o

.PHONY: all clean
all: build/exp3_hal_exti.hex build/exp3_hal_exti.bin

build/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

build/startup.o: startup/startup_stm32f103xb.s
	mkdir -p build
	$(CC) $(CPUFLAGS) -x assembler-with-cpp -c $< -o $@

build/exp3_hal_exti.elf: $(OBJECTS)
	$(CC) $(LDFLAGS) $^ -o $@
	$(SIZE) $@

build/exp3_hal_exti.hex: build/exp3_hal_exti.elf
	$(OBJCOPY) -O ihex $< $@

build/exp3_hal_exti.bin: build/exp3_hal_exti.elf
	$(OBJCOPY) -O binary $< $@

clean:
	rm -rf build

