# Host-side regression tests for the Morse library.
#
#   make        build and run the tests (with AddressSanitizer/UBSan)

CXX ?= g++
CXXFLAGS ?= -std=gnu++11 -Wall -Wextra -g -fsanitize=address,undefined -fno-sanitize-recover=all
SRC_DIR := ../../src

test_morse: test_morse.cpp Arduino.h $(SRC_DIR)/Morse.cpp $(SRC_DIR)/Morse.h $(SRC_DIR)/morsechar.h
	$(CXX) $(CXXFLAGS) -I. -I$(SRC_DIR) test_morse.cpp $(SRC_DIR)/Morse.cpp -o $@

.PHONY: test clean
.DEFAULT_GOAL := test

test: test_morse
	./test_morse

clean:
	rm -f test_morse
