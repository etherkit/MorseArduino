Morse Arduino
=============
This library sends Morse Code via toggling a specified output pin and/or toggling a boolean member variable which indicates the key state of the output.

**Please feel free to use the Issues feature of GitHub if you run into problems or have suggestions for important features to implement. This is the best way to get in touch.**

Thanks For Your Support!
------------------------
If you would like to support my library development efforts, please consider on-going support via my [SubscribeStar](https://www.subscribestar.com/nt7s) page, sending a one-time [PayPal donation](https://paypal.me/NT7S) or purchasing something from the [Etherkit Store](https://www.etherkit.com). Thank you!

Library Installation
---------------------
The best way to install the library is via the Arduino Library Manager, which is available if you are using Arduino IDE version 1.6.2 or greater. To install it this way, simply go to the menu Sketch > Include Library > Manage Libraries..., and then in the search box at the upper-right, type "Etherkit Morse". Click on the entry in the list below, then click on the provided "Install" button. By installing the library this way, you will always have notifications of future library updates, and can easily switch between library versions.

If you need to or would like to install the library in the old way, then you can download a copy of the library in a ZIP file. Download a ZIP file of the library from the GitHub repository by using the "Download ZIP" button at the right of the main repository page. Extract the ZIP file, then rename the unzipped folder as "Etherkit_Morse". Finally, open the Arduino IDE, select menu Sketch > Import Library... > Add Library..., and select the renamed folder that you just downloaded. Restart the IDE and you should have access to the new library.

Hardware Requirements and Setup
-------------------------------
This library has been written for the Arduino platform, to be processor agnostic. It has been successfully tested on the Arduino Uno, an Uno clone, and an Empyrean (Arduino Zero derivative). In order to be platform independent, it requires that the end user call on the class _update()_ member function one time per millisecond. The details of the implementation are left up to the user, but it is best done using a timer interrupt service routine.

Example
-------
First, install the Morse library into your instance of the Arduino IDE as described above.

There two simple examples named **morse_example_avr.ino** and **morse_example_samd.ino** that are placed in your examples menu under the Etherkit Morse folder. Open one corresponding to the Arduino variant on which you will be running the example. If you are using an AVR-based Arduino (such as an Uno), you'll want to also install the SimpleTimer library found [here](https://playground.arduino.cc/Code/SimpleTimer).

In order to use the Morse library in your sketch, you must first instantiate an object of class Morse:

    Morse morse(LED_BUILTIN, 15);

The class constructor takes two parameters: the desired Morse code output pin and sending speed.

Each example implements a method for calling the Morse class _update()_ method every one millisecond so that Morse code can be sent with the proper timing. See each sketch for details and note that these examples are only one possible way to implement the update function. As long as the _update()_ method is called reliably every one millisecond, the Morse library will function as intended.

Now all that one has to do to send Morse code in real-time is to use the _send()_ method with the desired message to send as a string literal or C-type string (null delimited character array).

    morse.send("HELLO WORLD");

Note that this library can accept lower or uppercase letters in the message buffer. The maximum message size is 100 characters; longer messages are truncated.

Further Details
---------------
If you need to check to see if the Morse library is currently sending the contents of its message buffer, check the boolean _busy_ class member. Should you call the _send()_ method before the library is done sending a current message, the old buffer will be overwritten with the new message and sending of the new message will start immediately.

The _send()_, _setWPM()_, and _reset()_ methods briefly disable interrupts while they update the library's internal state, so it is safe to call them from your main sketch while _update()_ runs from a timer interrupt.

You can pause sending at any time by setting the _tx_enable_ member to false. The library finishes the current element and then holds the key up until _tx_enable_ is set back to true.

The sending speed can be changed on-the-fly by using the _setWPM()_ method. The parameter is typed using a _float_ so that fractional words per minute can be specified. Why would you want to do this? In case you need to send Morse code using a very long integration time such as with the QRSS operating mode. For example, a setting of 0.2 WPM sets a Morse code "dit" length of 6 seconds. The speed is limited to the range 0.01 WPM (120 second dit) to 1200 WPM (1 ms dit).

If you don't want to have the library directly control a digital I/O pin, you may have your sketch poll the boolean _tx_ member variable and act on it accordingly within their periodic 1 ms function. Set the output pin parameter in the constructor to 0. The library will then not touch any pin.

By default the _led_pin_ member is the same as the output pin. You can set it to a different pin to have an LED follow the keying; the pin is configured as an output the next time you call _send()_.

When you are using this library for the DFCW mode (continuous wave Morse Code with frequency modulation), you can specify a preamble period to happen with the carrier on before the message begins. Set member variable _preamble_enable_ to true to enable this for a transmission. Keep in mind that this member variable acts like a one-shot, so you will need to set it every time you intend to use it. In DFCW mode the output pin is held high for the whole message, including the preamble, and goes low when the message ends or _reset()_ is called. Use the _tx_ member to tell dits and dahs apart.

Startup Conditions and Constraints
----------------------------------
The default output pin is defined as LED_BUILTIN while the default sending speed is 25 words per minute.

The maximum transmit buffer size is 100 characters.

Public Methods
--------------
### Morse() [class constructor]
```
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
Morse::Morse(uint8_t tx_pin = LED_BUILTIN, float init_wpm = 25)
```
### update()
```
/*
 * void Morse::update()
 *
 * State machine for the Morse library.
 *
 * This must be called by the client sketch every one millisecond in order
 * for the library to accurately send Morse code.
 *
 */
void Morse::update()
```
### setWPM()
```
/*
 * void Morse::setWPM(float new_wpm)
 *
 * Set the Morse code sending speed. Values are limited to the range
 * MIN_WPM..MAX_WPM.
 *
 * new_wpm - Sending speed in words per minute.
 *
 */
void Morse::setWPM(float new_wpm)
```
### send()
```
/*
 * void Morse::send(const char * message)
 *
 * Send the specified message in Morse code on the output pin. Any message in
 * progress is abandoned. Messages longer than TX_BUFFER_SIZE characters are
 * truncated.
 *
 * message - String literal or zero-delimited string to transmit.
 *
 */
void Morse::send(const char * message)
```
### reset()
```
/*
 * Morse::reset()
 *
 * Halts any sending in progress, empties the message buffer, turns the
 * outputs off, and resets the Morse state machine.
 *
 */
void Morse::reset()
```

Public Variables
----------------
    float wpm;                     // Read-only; use setWPM() to change
    volatile bool tx;              // True while the key is down
    bool tx_enable = true;         // Set false to pause sending
    uint8_t output_pin;
    uint8_t led_pin;
    bool dfcw_mode = false;
    volatile bool busy;            // True while a message is being sent
    bool preamble_enable = false;
    volatile uint8_t cur_char;     // Position in the message being sent

Public Constants
----------------
    Morse::DEFAULT_OUTPUT_PIN      // LED_BUILTIN, or 0 if the board has none
    Morse::DEFAULT_WPM             // 25
    Morse::MIN_WPM                 // 0.01
    Morse::MAX_WPM                 // 1200
    Morse::TX_BUFFER_SIZE          // 100

Valid Characters
----------------
The standard uppercase and lowercase letters 'A' through 'Z' and digits '0' through '9' are of course supported, along with a handful of punctuation and special characters. The following table indicates the special characters supported along with the corresponding Morse code. Other input characters (including control characters such as tabs and newlines) are skipped.

| Input Character   | Morse Character |
|-------------------|-----------------|
| - (minus sign)    | ➖➖ (M)          |
| / (slash)         | ➖••➖•           |
| = (equal sign)    | ➖•••➖ (BT)      |
| ? (question mark) | ••➖➖••          |

Testing
-------
The _extras/test_ folder holds host-side regression tests that build the library against a stub _Arduino.h_ and check the keying timing tick by tick. On a Linux or macOS machine with g++ or clang, run:

    cd extras/test
    make

Changelog
---------
* v1.2.0

    * Fix character and word spacing, which were 4 and 11 dits instead of the standard 3 and 7. Messages now send at the set WPM.
    * Fix element timing: gaps between elements were about 3 ms too long.
    * Fix buffer overflow when sending a message longer than 100 characters; long messages are now truncated.
    * Fix garbage being sent for unsupported characters, empty messages, and after reset(). Unsupported characters are now skipped.
    * Fix the state machine hanging when the 1 ms tick counter wraps after 49.7 days.
    * Fix pin 0 being written when the output pin is set to 0, and configure led_pin as an output.
    * reset() now turns the output pin off, and the DFCW carrier now turns off at the end of a message.
    * send(), setWPM(), and reset() are now safe to call while update() runs from an interrupt; tx, busy, and cur_char are volatile.
    * setWPM() limits the speed to 0.01 to 1200 WPM instead of dividing by zero.
    * The library constants and state enumeration are now members of the Morse class (e.g. Morse::TX_BUFFER_SIZE) instead of global names.
    * Add host-side regression tests in extras/test.

* v1.1.2

    * Fix bug in sending first character, correct keywords.txt

* v1.1.1

    * Minor bug and documentation fixes.

* v1.1.0

    * Add DFCW mode, LED pin, current character position, and reset.

* v1.0.1

    * Add support for preamble.

* v1.0.0

    * Initial release
