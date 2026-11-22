/*
 * Morse.cpp - Morse Code generation library for Arduino
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

#include "Morse.h"
#include "Arduino.h"
#include "morsechar.h"

#include <stdint.h>
#include <string.h>

static_assert(sizeof(morsechar) == MORSE_CHAR_END - MORSE_CHAR_START + 1,
  "morsechar table does not match MORSE_CHAR_START..MORSE_CHAR_END");

namespace
{

// Timing, in dit units. Standard spacing is 1 dit between elements, 3 dits
// between characters and 7 dits between words. Every element is followed by
// an element delay, so the character and word delays only add the remainder.
constexpr uint8_t UNITS_DIT = 1;
constexpr uint8_t UNITS_DAH = 3;
constexpr uint8_t UNITS_ELEMENT_DELAY = 1;
constexpr uint8_t UNITS_CHAR_DELAY = 3 - UNITS_ELEMENT_DELAY;
constexpr uint8_t UNITS_WORD_DELAY = 7 - UNITS_ELEMENT_DELAY - UNITS_CHAR_DELAY;
constexpr uint8_t UNITS_PREAMBLE = 7;

/*
 * Disables interrupts for the lifetime of the object, then restores the
 * previous interrupt state (so it is safe to use from inside an ISR).
 *
 * Used by the public methods that change state shared with update(), which
 * may be running from a timer interrupt.
 */
class InterruptGuard
{
public:
#if defined(__AVR__)
  InterruptGuard() : sreg(SREG) { cli(); }
  ~InterruptGuard() { __asm__ __volatile__("" ::: "memory"); SREG = sreg; }
private:
  uint8_t sreg;
#elif defined(__arm__) && defined(__ARM_ARCH_PROFILE) && (__ARM_ARCH_PROFILE == 'M')
  InterruptGuard()
  {
    __asm__ __volatile__("mrs %0, primask" : "=r"(primask));
    __asm__ __volatile__("cpsid i" ::: "memory");
  }
  ~InterruptGuard() { __asm__ __volatile__("msr primask, %0" :: "r"(primask) : "memory"); }
private:
  uint32_t primask;
#else
  InterruptGuard() { noInterrupts(); }
  ~InterruptGuard() { interrupts(); }
#endif
};

/*
 * Look up the Morse encoding of an ASCII character. Returns 0 for the end of
 * the string and MORSE_EOC for any character that cannot be sent.
 */
uint8_t lookupCharacter(char c)
{
  uint8_t ch = static_cast<uint8_t>(c);

  if (ch == '\0')
  {
    return 0;
  }

  if (ch >= 'a' && ch <= 'z')
  {
    ch -= 'a' - 'A';
  }

  if (ch < MORSE_CHAR_START || ch > MORSE_CHAR_END)
  {
    return MORSE_EOC;
  }

  return MORSE_READ_BYTE(&morsechar[ch - MORSE_CHAR_START]);
}

} // namespace

/*
 * Morse::Morse(uint8_t tx_pin, float init_wpm)
 *
 * Create an instance of the Morse class.
 *
 * tx_pin - Arduino pin used as the output by this library. Will not toggle an
 *          output pin if set to 0.
 * init_wpm - Sending speed in words per minute.
 *
 */
Morse::Morse(uint8_t tx_pin, float init_wpm) : output_pin(tx_pin), led_pin(tx_pin)
{
	setWPM(init_wpm);
	configurePins();
}

/*
 * Morse::update()
 *
 * State machine for the Morse library.
 *
 * This must be called by the client sketch every one millisecond in order
 * for the library to accurately send Morse code.
 *
 */
void Morse::update()
{
	cur_timer++;

	if (cur_state == State::IDLE)
	{
		if (busy && tx_enable)
		{
			nextElement();
		}
		return;
	}

	// Wrap-safe check for whether the current state has run its full length
	if (static_cast<int32_t>(cur_timer - cur_state_end) < 0)
	{
		return;
	}

	switch (cur_state)
	{
	case State::DIT:
	case State::DAH:
		startState(State::ELEMENTDELAY, UNITS_ELEMENT_DELAY);
		break;

	case State::PREAMBLE:
		preamble_enable = false;
		nextElement();
		break;

	case State::EOMDELAY:
		finishMessage();
		break;

	default:
		nextElement();
		break;
	}
}

/*
 * Morse::setWPM(float new_wpm)
 *
 * Set the Morse code sending speed. Values are limited to the range
 * MIN_WPM..MAX_WPM.
 *
 * new_wpm - Sending speed in words per minute.
 *
 */
void Morse::setWPM(float new_wpm)
{
	if (!(new_wpm >= MIN_WPM)) // Also catches NaN
	{
		new_wpm = MIN_WPM;
	}
	else if (new_wpm > MAX_WPM)
	{
		new_wpm = MAX_WPM;
	}

	// This is converted to WPM * 1000 due to need for fractional WPM for slow modes
	//
	// Dit length in milliseconds is 1200 ms / WPM
	uint32_t new_dit_length = 1200000UL / static_cast<uint32_t>(new_wpm * 1000 + 0.5f);

	InterruptGuard guard;
	wpm = new_wpm;
	dit_length = new_dit_length;
}

/*
 * Morse::send(const char * message)
 *
 * Send the specified message in Morse code on the output pin. Any message in
 * progress is abandoned. Messages longer than TX_BUFFER_SIZE characters are
 * truncated.
 *
 * message - String literal or zero-delimited string to transmit.
 *
 */
void Morse::send(const char *message)
{
	InterruptGuard guard;

	configurePins();

	strncpy(msg_buffer, message, TX_BUFFER_SIZE);
	msg_buffer[TX_BUFFER_SIZE] = '\0';
	cur_msg_p = msg_buffer;
	cur_char = 0;
	cur_character = 0;
	msg_started = false;
	cur_state = State::IDLE;
	tx = false;
	setOutputs(false);
	busy = true;
}

/*
 * Morse::reset()
 *
 * Halts any sending in progress, empties the message buffer, turns the
 * outputs off, and resets the Morse state machine.
 *
 */
void Morse::reset()
{
	InterruptGuard guard;
	finishMessage();
}

/*
 * Private class members
 */

/*
 * Morse::nextElement()
 *
 * Start whatever comes next in the message: an element, a delay, or the end
 * of the message.
 *
 */
void Morse::nextElement()
{
	if (!tx_enable)
	{
		// Pause here; update() resumes from IDLE when tx_enable is set again
		cur_state = State::IDLE;
		tx = false;
		return;
	}

	if (!msg_started)
	{
		msg_started = true;
		loadCharacter();

		if (preamble_enable && cur_character != 0)
		{
			startState(State::PREAMBLE, UNITS_PREAMBLE);
			return;
		}
	}

	if (cur_character == 0)
	{
		// End of message. This delay completes the word space after the last
		// character so that back-to-back messages are spaced correctly.
		startState(State::EOMDELAY, UNITS_WORD_DELAY);
	}
	else if (cur_character == MORSE_SPACE)
	{
		cur_msg_p++;
		loadCharacter();
		startState(State::WORDDELAY, UNITS_WORD_DELAY);
	}
	else if (cur_character == MORSE_EOC)
	{
		cur_msg_p++;
		loadCharacter();
		startState(State::CHARDELAY, UNITS_CHAR_DELAY);
	}
	else
	{
		bool dah = (cur_character & 0b10000000) != 0;
		cur_character = static_cast<uint8_t>(cur_character << 1);

		if (dah)
		{
			startState(State::DAH, UNITS_DAH);
		}
		else
		{
			startState(State::DIT, UNITS_DIT);
		}
	}
}

/*
 * Morse::loadCharacter()
 *
 * Load the character at cur_msg_p into cur_character, skipping over any
 * characters that cannot be sent.
 *
 */
void Morse::loadCharacter()
{
	cur_character = lookupCharacter(*cur_msg_p);

	while (cur_character == MORSE_EOC)
	{
		cur_msg_p++;
		cur_character = lookupCharacter(*cur_msg_p);
	}

	cur_char = static_cast<uint8_t>(cur_msg_p - msg_buffer);
}

/*
 * Morse::startState(State state, uint8_t units)
 *
 * Enter a timed state lasting the given number of dits and set the outputs
 * for it immediately.
 *
 */
void Morse::startState(State state, uint8_t units)
{
	bool key_down = (state == State::DIT || state == State::DAH);

	cur_state = state;
	cur_state_end = cur_timer + units * dit_length;
	tx = key_down;

	// In DFCW mode the carrier stays on for the whole message
	setOutputs(key_down || dfcw_mode);
}

/*
 * Morse::finishMessage()
 *
 * Clear the message buffer, turn the outputs off and return to IDLE.
 *
 */
void Morse::finishMessage()
{
	memset(msg_buffer, 0, TX_BUFFER_SIZE + 1);
	cur_msg_p = msg_buffer;
	cur_char = 0;
	cur_character = 0;
	msg_started = false;
	cur_state = State::IDLE;
	tx = false;
	setOutputs(false);
	busy = false;
}

/*
 * Morse::setOutputs(bool on)
 *
 * Drive the output and LED pins. A pin number of 0 means "not used".
 *
 */
void Morse::setOutputs(bool on)
{
	if (output_pin)
	{
		digitalWrite(output_pin, on ? HIGH : LOW);
	}

	if (led_pin && led_pin != output_pin)
	{
		digitalWrite(led_pin, on ? HIGH : LOW);
	}
}

/*
 * Morse::configurePins()
 *
 * Set the output and LED pins to OUTPUT mode. Called from the constructor
 * and again from send(), so that pins are configured after the core's own
 * initialization and so that a led_pin changed after construction works.
 *
 */
void Morse::configurePins()
{
	if (output_pin)
	{
		pinMode(output_pin, OUTPUT);
	}

	if (led_pin && led_pin != output_pin)
	{
		pinMode(led_pin, OUTPUT);
	}
}
