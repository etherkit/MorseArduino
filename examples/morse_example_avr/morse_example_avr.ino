/*
 * morse_example_avr.ino - Simple example of using the Etherkit Morse library
 *                         on an AVR-based Arduino
 *
 * Copyright (C) 2018 Jason Milldrum <milldrum@gmail.com>
 *
 * Uses the SimpleTimer library which you can find here:
 * https://playground.arduino.cc/Code/SimpleTimer
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

 #include <SimpleTimer.h>
 #include <Morse.h>

 // Class declaration
 Morse morse(LED_BUILTIN, 15);
 SimpleTimer timer;

 // a function to be executed periodically
 void repeatMe()
 {
   morse.update();
 }

 void setup()
 {
   timer.setInterval(1, repeatMe);

   delay(1000);
   morse.send("HELLO WORLD");
 }

 void loop() {
   timer.run();
 }
