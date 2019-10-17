/*
 * Morse.h - Morse Code generation library for Arduino
 *
 * Copyright (C) 2017 Jason Milldrum <milldrum@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ETHERKIT_MORSE_H_
#define ETHERKIT_MORSE_H_

#if defined(__arm__)
#include <cstdint>
#else
#include <stdint.h>
#endif

#include "Arduino.h"

// Constants
constexpr uint8_t DEFAULT_OUTPUT_PIN = LED_BUILTIN;
constexpr uint8_t DEFAULT_LED_PIN = LED_BUILTIN;
constexpr float DEFAULT_WPM = 25;
constexpr uint8_t TX_BUFFER_SIZE = 100;
constexpr uint8_t MULT_DAH = 3; // DAH is 3x a DIT
constexpr uint8_t MULT_WORDDELAY = 7; // Space between words is 7 dits
constexpr uint8_t MAX_MSG_DELAY = 30;	// Maximum message delay time in minutes

// Enumerations
enum class State {IDLE, DIT, DAH, DITDELAY, DAHDELAY, WORDDELAY, MSGDELAY,
  EOMDELAY, PREAMBLE};

class Morse
{
public:
  Morse(uint8_t tx_pin = DEFAULT_OUTPUT_PIN, float init_wpm = DEFAULT_WPM);
  void update();
  void setWPM(float);
  void send(const char *);
  void reset();

  float wpm;
  bool tx = false;
  bool tx_enable = true;
  uint8_t output_pin;
  uint8_t led_pin;
  bool dfcw_mode = false;
  bool busy = true;
  bool preamble_enable = false;
  uint8_t cur_char = 0;

private:
  uint32_t getMsgDelay(uint8_t);
  uint32_t dit_delay;
  uint32_t dit_length;
  uint8_t msg_delay = 0;
  uint32_t msg_delay_end;
  char * cur_msg_p = nullptr;
  uint8_t cur_character = 0;

  State cur_state, next_state;
  volatile uint32_t cur_timer, cur_state_end;
  char msg_buffer[TX_BUFFER_SIZE + 1];
};

#endif // ETHERKIT_MORSE_H_
