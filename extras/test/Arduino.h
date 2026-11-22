/*
 * Minimal Arduino.h stub for running the Morse library on a host machine.
 * Records every pin change with the update() tick it happened on.
 */

#ifndef MORSE_TEST_ARDUINO_H_
#define MORSE_TEST_ARDUINO_H_

#include <stdint.h>
#include <vector>

#define LED_BUILTIN 13
#define INPUT 0
#define OUTPUT 1
#define LOW 0
#define HIGH 1

struct PinEvent
{
  uint32_t tick;
  uint8_t pin;
  uint8_t value;
};

namespace sim
{
extern uint32_t now;
extern uint8_t pin_value[256];
extern uint8_t pin_mode[256];
extern std::vector<PinEvent> events;
}

inline void pinMode(uint8_t pin, uint8_t mode) { sim::pin_mode[pin] = mode; }

inline void digitalWrite(uint8_t pin, uint8_t value)
{
  if (sim::pin_value[pin] != value)
  {
    sim::events.push_back(PinEvent{sim::now, pin, value});
  }
  sim::pin_value[pin] = value;
}

inline void noInterrupts() {}
inline void interrupts() {}

#endif // MORSE_TEST_ARDUINO_H_
