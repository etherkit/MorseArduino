# Host-side regression tests for the Morse library.
#
#   make        build and run the tests on this machine (with ASan/UBSan)
#   make qemu   build the same tests for a Cortex-M0 (the SAMD21's ARMv6-M
#               instruction set) and run them under qemu-system-arm
#
# `make qemu` uses the arm-none-eabi toolchain installed by the Arduino SAMD
# core if it can find one, otherwise arm-none-eabi-g++ from PATH. Override with
# ARM_TOOLCHAIN=/path/to/toolchain/bin/

CXX ?= g++
CXXFLAGS ?= -std=gnu++11 -Wall -Wextra -g -fsanitize=address,undefined -fno-sanitize-recover=all
SRC_DIR := ../../src
LIB_SRC := $(SRC_DIR)/Morse.cpp $(SRC_DIR)/Morse.h $(SRC_DIR)/morsechar.h

ARM_TOOLCHAIN ?= $(lastword $(sort $(wildcard $(HOME)/.arduino15/packages/arduino/tools/arm-none-eabi-gcc/*/bin/)))
ARM_CC := $(ARM_TOOLCHAIN)arm-none-eabi-gcc
ARM_CXX := $(ARM_TOOLCHAIN)arm-none-eabi-g++
ARM_FLAGS := -mcpu=cortex-m0plus -mthumb -Os -g -Wall -Wextra
ARM_CXXFLAGS := $(ARM_FLAGS) -std=gnu++11 -fno-exceptions -fno-rtti
ARM_LDFLAGS := -nostartfiles --specs=rdimon.specs -T qemu/microbit.ld -Wl,--gc-sections
QEMU ?= qemu-system-arm
QEMU_FLAGS := -M microbit -display none -monitor none -serial null \
	-semihosting-config enable=on,target=native

.PHONY: test qemu clean
.DEFAULT_GOAL := test

test_morse: test_morse.cpp Arduino.h $(LIB_SRC)
	$(CXX) $(CXXFLAGS) -I. -I$(SRC_DIR) test_morse.cpp $(SRC_DIR)/Morse.cpp -o $@

test: test_morse
	./test_morse

test_morse_m0.elf: test_morse.cpp Arduino.h $(LIB_SRC) qemu/startup.c qemu/microbit.ld
	$(ARM_CC) $(ARM_FLAGS) -c qemu/startup.c -o startup_m0.o
	$(ARM_CXX) $(ARM_CXXFLAGS) -I. -I$(SRC_DIR) test_morse.cpp $(SRC_DIR)/Morse.cpp \
		startup_m0.o $(ARM_LDFLAGS) -o $@
	rm -f startup_m0.o

qemu: test_morse_m0.elf
	timeout 300 $(QEMU) $(QEMU_FLAGS) -kernel $<

clean:
	rm -f test_morse test_morse_m0.elf startup_m0.o
