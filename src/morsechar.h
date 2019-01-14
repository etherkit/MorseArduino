/*
 * morsechar.h
 *
 *  Created on: Nov 29, 2010
 *      Author: Jason Milldrum
 *     Company: Etherkit
 *
 *     Copyright (c) 2010 - 2018, Jason Milldrum
 *     All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without modification,
 *  are permitted provided that the following conditions are met:
 *
 *  - Redistributions of source code must retain the above copyright notice, this list
 *  of conditions and the following disclaimer.
 *
 *  - Redistributions in binary form must reproduce the above copyright notice, this list
 *  of conditions and the following disclaimer in the documentation and/or other
 *  materials provided with the distribution.
 *
 *  - Neither the name of Etherkit nor the names of its contributors may be
 *  used to endorse or promote products derived from this software without specific
 *  prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
 *  EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 *  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 *  SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 *  TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 *  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef MORSECHAR_H_
#define MORSECHAR_H_

// The morsechar array maps to the standard ASCII table starting at ASCII 32 (SPACE).
//
// A dit is represented by a "0" bit, while a dah is represented by a "1" bit.
//
// Characters are encoded with the most significant "bit" first so that the byte can be left-shifted
// to read out each element. Each character must be terminated in a "1" so that the reading algorithm
// knows to end when the byte == 0b10000000.

constexpr uint8_t MORSE_CHAR_START = 32;

#if defined(__arm__)
const uint8_t morsechar[] =
#else
const uint8_t morsechar[] PROGMEM =
#endif
{
  0b11111111,		// Special code for SPACE
  0b10000000,		// Not implemented
  0b10000000,		// Not implemented
  0b10000000,		// Not implemented
  0b10000000,		// Not implemented
  0b10000000,		// Not implemented
  0b10000000,		// Not implemented
  0b10000000,		// Not implemented
  0b10000000,		// Not implemented
  0b10000000,		// Not implemented
  0b10000000,		// Not implemented
  0b10000000,		// Not implemented
  0b10000000,		// Not implemented
  0b11100000,		// Minus sign (indicated by "M" in this implementation)
  0b10000000,		// Not implemented
  0b10010100,		// "/" Slash
  0b11111100,		// "0"
  0b01111100,		// "1"
  0b00111100,		// "2"
  0b00011100,		// "3"
  0b00001100,		// "4"
  0b00000100,		// "5"
  0b10000100,		// "6"
  0b11000100,		// "7"
  0b11100100,		// "8"
  0b11110100,		// "9"
  0b10000000,		// Not implemented
  0b10000000,		// Not implemented
  0b10000000,		// Not implemented
  0b10001100,		// "=" BT prosign/Equal sign
  0b10000000,		// Not implemented
  0b00110010,		// "?" Question mark
  0b10000000,		// Not implemented
  0b01100000,		// "A"
  0b10001000,		// "B"
  0b10101000,		// "C"
  0b10010000,		// "D"
  0b01000000,		// "E"
  0b00101000,		// "F"
  0b11010000,		// "G"
  0b00001000,		// "H"
  0b00100000,		// "I"
  0b01111000,		// "J"
  0b10110000,		// "K"
  0b01001000,		// "L"
  0b11100000,		// "M"
  0b10100000,		// "N"
  0b11110000,		// "O"
  0b01101000,		// "P"
  0b11011000,		// "Q"
  0b01010000,		// "R"
  0b00010000,		// "S"
  0b11000000,		// "T"
  0b00110000,		// "U"
  0b00011000,		// "V"
  0b01110000,		// "W"
  0b10011000,		// "X"
  0b10111000,		// "Y"
  0b11001000,		// "Z"
  0b10000000,		// Not implemented
  0b10000000,		// Not implemented
  0b10000000,		// Not implemented
  0b10000000,		// Not implemented
  0b10000000,		// Not implemented
  0b10000000,		// Not implemented
  0b01100000,		// "a"
  0b10001000,		// "b"
  0b10101000,		// "c"
  0b10010000,		// "d"
  0b01000000,		// "e"
  0b00101000,		// "f"
  0b11010000,		// "g"
  0b00001000,		// "h"
  0b00100000,		// "i"
  0b01111000,		// "j"
  0b10110000,		// "k"
  0b01001000,		// "l"
  0b11100000,		// "m"
  0b10100000,		// "n"
  0b11110000,		// "o"
  0b01101000,		// "p"
  0b11011000,		// "q"
  0b01010000,		// "r"
  0b00010000,		// "s"
  0b11000000,		// "t"
  0b00110000,		// "u"
  0b00011000,		// "v"
  0b01110000,		// "w"
  0b10011000,		// "x"
  0b10111000,		// "y"
  0b11001000    // "z"
};

#endif /* MORSECHAR_H_ */
