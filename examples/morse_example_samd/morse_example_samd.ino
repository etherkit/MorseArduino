/*
 * morse_example_samd.ino - Simple example of using the Etherkit Morse library
 *                          on a SAMD-based Arduino
 *
 * Copyright (C) 2018 Jason Milldrum <milldrum@gmail.com>
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

 #include <Morse.h>

 // Constant expressions
 constexpr uint16_t TIMER_PRESCALER_DIV = 1024;

 // Class declaration
 Morse morse(LED_BUILTIN, 15);

 void setup()
 {
   startTimer(1000); // 1 ms ISR
   delay(1000);
   morse.send("HELLO WORLD");
 }

 void loop()
 {
   // Let the ISR have all the fun
 }

 // Timer code derived from:
 // https://github.com/nebs/arduino-zero-timer-demo

 void setTimerFrequency(int frequencyHz)
 {
   int compareValue = (VARIANT_MCK / (TIMER_PRESCALER_DIV * frequencyHz)) - 1;
   TcCount16* TC = (TcCount16*) TC5;
   // Make sure the count is in a proportional position to where it was
   // to prevent any jitter or disconnect when changing the compare value.
   TC->COUNT.reg = map(TC->COUNT.reg, 0, TC->CC[0].reg, 0, compareValue);
   TC->CC[0].reg = compareValue;
   while (TC->STATUS.bit.SYNCBUSY == 1);
 }

 /*
 This is a slightly modified version of the timer setup found at:
 https://github.com/maxbader/arduino_tools
  */
 void startTimer(int frequencyHz)
 {
   REG_GCLK_CLKCTRL = (uint16_t) (GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_ID (GCM_TC4_TC5)) ;
   while ( GCLK->STATUS.bit.SYNCBUSY == 1 );

   TcCount16* TC = (TcCount16*) TC5;

   TC->CTRLA.reg &= ~TC_CTRLA_ENABLE;

   // Use the 16-bit timer
   TC->CTRLA.reg |= TC_CTRLA_MODE_COUNT16;
   while (TC->STATUS.bit.SYNCBUSY == 1);

   // Use match mode so that the timer counter resets when the count matches the compare register
   TC->CTRLA.reg |= TC_CTRLA_WAVEGEN_MFRQ;
   while (TC->STATUS.bit.SYNCBUSY == 1);

   // Set prescaler to 1024
   TC->CTRLA.reg |= TC_CTRLA_PRESCALER_DIV1024;
   while (TC->STATUS.bit.SYNCBUSY == 1);

   setTimerFrequency(frequencyHz);

   // Enable the compare interrupt
   TC->INTENSET.reg = 0;
   TC->INTENSET.bit.MC0 = 1;

   NVIC_EnableIRQ(TC5_IRQn);

   TC->CTRLA.reg |= TC_CTRLA_ENABLE;
   while (TC->STATUS.bit.SYNCBUSY == 1);
 }

 void TC5_Handler()
 {
   TcCount16* TC = (TcCount16*) TC5;

   if (TC->INTFLAG.bit.MC0 == 1)
   {
     TC->INTFLAG.bit.MC0 = 1;
     morse.update();
   }
 }
