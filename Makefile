# Build the project with avr-gcc (no Atmel Studio needed).
#
#   make            Build build/gpio_struct.elf and .hex
#   make size       Show flash/RAM usage
#   make flash      Flash to an Arduino Uno via avrdude (PORT=/dev/ttyACM0)
#   make clean      Remove build output
#
# Pass WERROR=1 to treat warnings as errors (used by CI).

TARGET  := gpio_struct
MCU     := atmega328p
F_CPU   := 16000000UL
BUILD   := build

CC      := avr-gcc
OBJCOPY := avr-objcopy
SIZE    := avr-size
AVRDUDE := avrdude
PORT    ?= /dev/ttyACM0

SRCS := main.c \
        source/driver/gpio.c \
        source/driver/timer.c

OBJS := $(SRCS:%.c=$(BUILD)/%.o)
DEPS := $(OBJS:.o=.d)

CFLAGS := -mmcu=$(MCU) -DF_CPU=$(F_CPU) -std=gnu11 -Os \
          -Wall -Wextra \
          -fshort-enums -fpack-struct -funsigned-char -funsigned-bitfields \
          -ffunction-sections -fdata-sections \
          -Iinclude -MMD -MP
ifeq ($(WERROR),1)
CFLAGS += -Werror
endif

LDFLAGS := -mmcu=$(MCU) -Wl,--gc-sections -Wl,-Map=$(BUILD)/$(TARGET).map

.PHONY: all size flash clean

all: $(BUILD)/$(TARGET).hex size

$(BUILD)/$(TARGET).elf: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

$(BUILD)/$(TARGET).hex: $(BUILD)/$(TARGET).elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

size: $(BUILD)/$(TARGET).elf
	$(SIZE) $<

flash: $(BUILD)/$(TARGET).hex
	$(AVRDUDE) -p $(MCU) -c arduino -P $(PORT) -b 115200 -U flash:w:$<:i

clean:
	rm -rf $(BUILD)

-include $(DEPS)
