PREFIX ?= arm-none-eabi-
CC := $(PREFIX)gcc
OBJCOPY := $(PREFIX)objcopy
SIZE := $(PREFIX)size

CPUFLAGS := -mcpu=cortex-m3 -mthumb
CFLAGS := $(CPUFLAGS) -std=c11 -Os -ffreestanding -ffunction-sections -fdata-sections -Wall -Wextra -Werror
LDFLAGS := $(CPUFLAGS) -nostartfiles --specs=nano.specs --specs=nosys.specs -Tlinker/STM32F103C8T6_FLASH.ld -Wl,--gc-sections

.PHONY: all three four clean
all: three four

three:
	$(MAKE) VARIANT=three-led ONBOARD=0 firmware

four:
	$(MAKE) VARIANT=four-led ONBOARD=1 firmware

firmware:
	mkdir -p build/$(VARIANT)
	$(CC) $(CFLAGS) -DINCLUDE_ONBOARD_LED=$(ONBOARD) -c src/main.c -o build/$(VARIANT)/main.o
	$(CC) $(CPUFLAGS) -x assembler-with-cpp -c startup/startup_stm32f103c8t6.S -o build/$(VARIANT)/startup.o
	$(CC) $(LDFLAGS) -Wl,-Map=build/$(VARIANT)/bluepill-led-$(VARIANT).map build/$(VARIANT)/startup.o build/$(VARIANT)/main.o -o build/$(VARIANT)/bluepill-led-$(VARIANT).elf
	$(OBJCOPY) -O ihex build/$(VARIANT)/bluepill-led-$(VARIANT).elf build/$(VARIANT)/bluepill-led-$(VARIANT).hex
	$(OBJCOPY) -O binary build/$(VARIANT)/bluepill-led-$(VARIANT).elf build/$(VARIANT)/bluepill-led-$(VARIANT).bin
	$(SIZE) build/$(VARIANT)/bluepill-led-$(VARIANT).elf

clean:
	rm -rf build

