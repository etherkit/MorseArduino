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

#if defined(__arm__)
#include <cstring>
#include <cstdint>
#else
#include <string.h>
#include <stdint.h>
#endif

#include "Morse.h"
#include "Arduino.h"
#include "morsechar.h"

//#define DEBUG

/*
 * Morse::Morse(uint8_t tx_pin, float init_wpm) : output_pin(tx_pin)
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
	tx = false;
	tx_enable = true;
	busy = false;
	cur_character = 0;

	setWPM(init_wpm);

	if (output_pin)
	{
		pinMode(output_pin, OUTPUT);
	}

	cur_state = State::IDLE;
	next_state = State::IDLE;
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
	noInterrupts();
	cur_timer++;
	interrupts();

#ifdef DEBUG
	if (cur_character != 0)
	{
		//Serial.println(cur_timer);
		//Serial.println(cur_character, HEX);
	}
#endif

	switch (cur_state)
	{
	case State::IDLE:
		// TX off
		//pa_enable = false;
		tx = false;

		if (tx_enable)
		{
			msg_delay_end = cur_timer + getMsgDelay(msg_delay);

			// If this is the first time thru the message loop, get the first character
			if ((cur_msg_p == msg_buffer) && (cur_character == '\0'))
			{
#if defined(__arm__)
				cur_character = morsechar[(*cur_msg_p) - MORSE_CHAR_START];
#else
				cur_character = pgm_read_byte(&morsechar[(*cur_msg_p) - MORSE_CHAR_START]);
#endif
#ifdef DEBUG
				Serial.println("Start of message");
#endif
				if (preamble_enable)
				{
					cur_state = State::PREAMBLE;
					cur_state_end = cur_state_end = cur_timer + (dit_length * MULT_WORDDELAY);
					return;
				}
			}

			// Get the current element in the current character
			if (cur_character != '\0')
			{
				if (cur_character == 0b10000000 || cur_character == 0b11111111) // End of character marker or SPACE
				{
					// Set next state based on whether EOC or SPACE
					if (cur_character == 0b10000000)
					{
#ifdef DEBUG
						Serial.println("End of character");
#endif
						cur_state_end = cur_timer + (dit_length * MULT_DAH);
						cur_state = State::DAHDELAY;

						// Grab next character, set state to inter-character delay
						cur_msg_p++;
						cur_char++;

						// If we read a NULL from the announce buffer, set cur_character to NULL,
						// otherwise set to correct morse character
						if ((*cur_msg_p) == '\0')
						{
							cur_character = '\0';
						}
						else
						{
#if defined(__arm__)
							cur_character = morsechar[(*cur_msg_p) - MORSE_CHAR_START];
#else
							cur_character = pgm_read_byte(&morsechar[(*cur_msg_p) - MORSE_CHAR_START]);
#endif
						}
					}
					else
					{
						cur_state_end = cur_timer + (dit_length * MULT_WORDDELAY);
						cur_state = State::WORDDELAY;
#ifdef DEBUG
						Serial.println("Word delay");
#endif
					}

					// Grab next character, set state to inter-character delay
					//cur_msg_p++;

					// If we read a NULL from the announce buffer, set cur_character to NULL,
					// otherwise set to correct morse character
					// if((*cur_msg_p) == '\0')
					// {
					//   cur_character = '\0';
					// }
					// else
					// {
					// 	#if defined(__arm__)
					// 	cur_character = morsechar[(*cur_msg_p) - MORSE_CHAR_START];
					// 	#else
					// 	cur_character = pgm_read_byte(&morsechar[(*cur_msg_p) - MORSE_CHAR_START]);
					// 	#endif
					// }
				}
				else
				{
					// Mask off MSb, set cur_element
					if ((cur_character & 0b10000000) == 0b10000000)
					{
						cur_state_end = cur_timer + (dit_length * MULT_DAH);
						cur_state = State::DAH;
#ifdef DEBUG
						Serial.print("DAH ");
						Serial.println(cur_character, BIN);
#endif
					}
					else
					{
						cur_state_end = cur_timer + dit_length;
						cur_state = State::DIT;
#ifdef DEBUG
						Serial.print("DIT ");
						Serial.println(cur_character, BIN);
#endif
					}

					// Shift left to get next element
					cur_character = cur_character << 1;
				}
			}
			else // End of message
			{
#ifdef DEBUG
				Serial.println("End of message");
#endif
				// Reload the message buffer and set buffer pointer back to beginning
				//strcpy(msg_buffer, tx_buffer);
				//cur_msg_p = msg_buffer;
				cur_character = '\0';

				if (msg_delay == 0)
				{
					// If a constantly repeating message, put a word delay at the end of message
					cur_state_end = cur_timer + (dit_length * MULT_WORDDELAY);
					cur_state = State::EOMDELAY;
				}
				else
				{
					// Otherwise, set the message delay time
					if (msg_delay_end < (cur_timer + (dit_length * MULT_WORDDELAY)))
					{
						cur_state_end = cur_timer + (dit_length * MULT_WORDDELAY);
					}
					else
					{
						cur_state_end = msg_delay_end;
					}

					cur_state = State::MSGDELAY;
				}
			}
		}
		break;

	case State::PREAMBLE:
		// Transmitter off
		tx = false;
		if (dfcw_mode)
		{
			// if(output_pin)
			// {
			// 	digitalWrite(output_pin, LOW);
			// }
			digitalWrite(output_pin, HIGH);
			digitalWrite(led_pin, HIGH);
		}
		else
		{
			if (output_pin)
			{
				digitalWrite(output_pin, LOW);
			}
			digitalWrite(led_pin, LOW);
		}

		// When done waiting, go back to IDLE state to start the message
		if (cur_timer > cur_state_end)
		{
			preamble_enable = false;
			cur_state = State::IDLE;
		}
		break;

	case State::DIT:
	case State::DAH:
		tx = true;
		if (dfcw_mode)
		{
			// if(output_pin)
			// {
			// 	digitalWrite(output_pin, LOW);
			// }
			digitalWrite(output_pin, HIGH);
			digitalWrite(led_pin, HIGH);
		}
		else
		{
			if (output_pin)
			{
				digitalWrite(output_pin, HIGH);
			}
			digitalWrite(led_pin, HIGH);
		}

		if (cur_timer > cur_state_end)
		{
			tx = false;
			if (dfcw_mode)
			{
				// if(output_pin)
				// {
				// 	digitalWrite(output_pin, LOW);
				// }
				digitalWrite(led_pin, HIGH);
			}
			else
			{
				if (output_pin)
				{
					digitalWrite(output_pin, LOW);
				}
				digitalWrite(led_pin, LOW);
			}

			cur_state_end = cur_timer + dit_length;
			cur_state = State::DITDELAY;
		}
		break;
	case State::DITDELAY:
	case State::DAHDELAY:
	case State::WORDDELAY:
	case State::MSGDELAY:
	case State::EOMDELAY:
		tx = false;
		if (dfcw_mode)
		{
			// if(output_pin)
			// {
			// 	digitalWrite(output_pin, LOW);
			// }
			digitalWrite(led_pin, HIGH);
		}
		else
		{
			if (output_pin)
			{
				digitalWrite(output_pin, LOW);
			}
			digitalWrite(led_pin, LOW);
		}

		if (cur_timer > cur_state_end)
		{
			if (cur_state == State::WORDDELAY)
			{
				// Grab next character
				cur_msg_p++;
				cur_char++;

				// If we read a NULL from the announce buffer, set cur_character to NULL,
				// otherwise set to correct morse character
				if ((*cur_msg_p) == '\0')
				{
					cur_character = '\0';
				}
				else
				{
// TODO
#if defined(__arm__)
					cur_character = morsechar[(*cur_msg_p) - MORSE_CHAR_START];
#else
					cur_character = pgm_read_byte(&morsechar[(*cur_msg_p) - MORSE_CHAR_START]);
#endif
				}
			}
			else if (cur_state == State::EOMDELAY)
			{
				// Clear the message buffer when message sending is complete
				memset(msg_buffer, 0, TX_BUFFER_SIZE + 1);
				busy = false;
				cur_char = 0;
				if (dfcw_mode)
				{
					digitalWrite(led_pin, LOW);
				}
				else
				{
					if (output_pin)
					{
						digitalWrite(output_pin, LOW);
					}
					digitalWrite(led_pin, LOW);
				}
			}

			cur_state = State::IDLE;
		}
		break;

	default:
		break;
	}
}

/*
 * Morse::setWPM(float new_wpm)
 *
 * Set the Morse code sending speed.
 *
 * new_wpm - Sending speed in words per minute.
 *
 */
void Morse::setWPM(float new_wpm)
{
	wpm = new_wpm;
	// This is converted to WPM * 1000 due to need for fractional WPM for slow modes
	//
	// Dit length in milliseconds is 1200 ms / WPM
	dit_length = (1200000UL / (uint32_t)(new_wpm * 1000));
}

/*
 * Morse::send(const char * message)
 *
 * Send the specified message in Morse code on the output pin.
 *
 * message - String literal or zero-delimited string to transmit.
 *
 */
void Morse::send(const char *message)
{
	strcpy(msg_buffer, message);
	cur_msg_p = msg_buffer;
	cur_char = 0;
	cur_state = State::IDLE;
	cur_character = 0;
	busy = true;
}

/*
 * Morse::reset()
 *
 * Halts any sending in progress, empties the message buffer, and resets the
 * Morse state machine.
 *
 */
void Morse::reset()
{
	// strcpy((char *)msg_buffer, "");
	memset(msg_buffer, 0, TX_BUFFER_SIZE + 1);
	cur_msg_p = msg_buffer;
	cur_char = 0;
	cur_state = State::IDLE;
	cur_character = 0;
	busy = false;
	digitalWrite(led_pin, LOW);
}

/*
 * Private class members
 */

/*
 * uint32_t Morse::getMsgDelay(uint8_t delay_minutes)
 *
 * Convert a number of minutes to clock ticks (in milliseconds)
 * in order to have the library delay for longer periods.
 *
 * delay_minutes - Number of minutes to convert to clock ticks.
 *
 */
uint32_t Morse::getMsgDelay(uint8_t delay_minutes)
{
	// The single function parameter is the message delay time in minutes
	if (delay_minutes > MAX_MSG_DELAY)
	{
		delay_minutes = MAX_MSG_DELAY;
	}

	// Number of clock ticks is the number of minutes * 60000 ticks/per min
	return delay_minutes * 60000UL;
}
