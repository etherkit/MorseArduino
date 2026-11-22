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

#include "Arduino.h"
#include <stdint.h>

class Morse
{
public:
  // Constants
#ifdef LED_BUILTIN
  static constexpr uint8_t DEFAULT_OUTPUT_PIN = LED_BUILTIN;
#else
  static constexpr uint8_t DEFAULT_OUTPUT_PIN = 0;
#endif
  static constexpr float DEFAULT_WPM = 25;
  static constexpr float MIN_WPM = 0.01;   // 120 second dit (QRSS120)
  static constexpr float MAX_WPM = 1200;   // 1 ms dit (one update() tick)
  static constexpr uint8_t TX_BUFFER_SIZE = 100;

  Morse(uint8_t tx_pin = DEFAULT_OUTPUT_PIN, float init_wpm = DEFAULT_WPM);
  void update();
  void setWPM(float);
  void send(const char *);
  void reset();

  float wpm;
  volatile bool tx = false;
  bool tx_enable = true;
  uint8_t output_pin;
  uint8_t led_pin;
  bool dfcw_mode = false;
  volatile bool busy = false;
  bool preamble_enable = false;
  volatile uint8_t cur_char = 0;

private:
  enum class State {IDLE, PREAMBLE, DIT, DAH, ELEMENTDELAY, CHARDELAY,
    WORDDELAY, EOMDELAY};

  void nextElement();
  void loadCharacter();
  void startState(State, uint8_t);
  void finishMessage();
  void setOutputs(bool);
  void configurePins();

  uint32_t dit_length = 0;
  uint32_t cur_timer = 0;
  uint32_t cur_state_end = 0;
  State cur_state = State::IDLE;
  bool msg_started = false;
  const char * cur_msg_p = msg_buffer;
  uint8_t cur_character = 0;
  char msg_buffer[TX_BUFFER_SIZE + 1] = {};
};

#endif // ETHERKIT_MORSE_H_
