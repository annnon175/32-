PREFIX ?= arm-none-eabi-
CC := $(PREFIX)gcc
OBJCOPY := $(PREFIX)objcopy
SIZE := $(PREFIX)size

CPUFLAGS := -mcpu=cortex-m3 -mthumb
DEFINES := -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER
INCLUDES := -IUser -ILibraries/CMSIS/Core/Include -ILibraries/CMSIS/Device/Include -ILibraries/STM32F10x_StdPeriph_Driver/inc
CFLAGS := $(CPUFLAGS) $(DEFINES) $(INCLUDES) -std=c11 -O0 -g3 -ffunction-sections -fdata-sections -Wall -Wextra -Werror
LDFLAGS := $(CPUFLAGS) -nostartfiles --specs=nano.specs --specs=nosys.specs -Tlinker/STM32F103C8T6_FLASH.ld -Wl,--gc-sections,-Map=build/exp2_spl.map

SOURCES := User/main.c \
           Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_gpio.c \
           Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_rcc.c

OBJECTS := $(patsubst %.c,build/%.o,$(SOURCES)) build/startup.o

.PHONY: all clean
all: build/exp2_spl.hex build/exp2_spl.bin

build/%.o: %.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

build/startup.o: startup/startup_stm32f103c8t6.S
	mkdir -p build
	$(CC) $(CPUFLAGS) -x assembler-with-cpp -c $< -o $@

build/exp2_spl.elf: $(OBJECTS)
	$(CC) $(LDFLAGS) $^ -o $@
	$(SIZE) $@

build/exp2_spl.hex: build/exp2_spl.elf
	$(OBJCOPY) -O ihex $< $@

build/exp2_spl.bin: build/exp2_spl.elf
	$(OBJCOPY) -O binary $< $@

clean:
	rm -rf build

