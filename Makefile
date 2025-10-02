# Commands to upload code:
# $ avr-gcc -Os -DF_CPU=16000000UL -mmcu=atmega328p -c -o main.o main.c
# $ avr-gcc -mmcu=atmega328p main.o -o main
# $ avr-objcopy -O ihex -R .eeprom main main.hex
# $ avrdude -F -V -c arduino -p ATMEGA328P -P /dev/cu.usbmodem1421 -b 115200 -U flash:w:main.hex

# AI generated Makefile

CC=avr-gcc
OBJCOPY=avr-objcopy

CFLAGS=-Os -DF_CPU=16000000UL -mmcu=atmega328p -Wall -Werror -pedantic
PORT=/dev/cu.usbmodem2101

# Collect all source files
SRCS := $(wildcard *.c)
OBJS := $(patsubst %.c, build/%.o, $(SRCS))

# Default target
all: build/main.hex

# Hex depends on ELF
build/main.hex: build/main.elf
	$(OBJCOPY) -O ihex -R .eeprom $< $@

# Link all objects
build/main.elf: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Compile rule for every .c -> .o
build/%.o: %.c $(wildcard *.h)
	@mkdir -p build
	$(CC) $(CFLAGS) -c $< -o $@

# Upload target
upload: build/main.hex
	avrdude -F -V -c arduino -p ATMEGA328P -P $(PORT) -b 115200 -U flash:w:$<

# Clean
clean:
	-rm -rf build