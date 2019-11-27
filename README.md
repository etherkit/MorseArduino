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

There two simple examples named **morse_example_avr.ino** and **morse_example_samd.ino** that are placed in your examples menu under the Etherkit Morse folder. Open one corresponding to the Arduino variant on which you will be running the example. If you are using an AVR-based Arduino (such as an Uno), you'll want to also install the SimpleTimer library found (here)[https://playground.arduino.cc/Code/SimpleTimer].

In order to use the Morse library in your sketch, you must first instantiate an object of class Morse:

    Morse morse(LED_BUILTIN, 15);

The class constructor takes two parameters: the desired Morse code output pin and sending speed.

Each example implements a method for calling the Morse class _update()_ method every one millisecond so that Morse code can be sent with the proper timing. See each sketch for details and note that these examples are only one possible way to implement the update function. As long as the _update()_ method is called reliably every one millisecond, the Morse library will function as intended.

Now all that one has to do to send Morse code in real-time is to use the _send()_ method with the desired message to send as a string literal or C-type string (null delimited character array).

    morse.send("HELLO WORLD");

Note that this library can accept lower or uppercase letters in the message buffer. The maximum message size is 100 characters.

Further Details
---------------
If you need to check to see if the Morse library is currently sending the contents of its message buffer, check the boolean _busy_ class member. Should you call the _send()_ method before the library is done sending a current message, the old buffer will be overwritten with the new message and sending of the new message will start immediately.

The sending speed can be changed on-the-fly by using the _setWPM()_ method. The parameter is typed using a _float_ so that fractional words per minute can be specified. Why would you want to do this? In case you need to send Morse code using a very long integration time such as with the QRSS operating mode. For example, a setting of 0.2 WPM sets a Morse code "dit" length of 6 seconds.

If you don't want to have the library directly control a digital I/O pin, you may have your sketch poll the boolean _tx_ member variable and act on it accordingly within their periodic 1 ms function. Set the output pin parameter in the constructor to 0.

When you are using this library for the DFCW mode (continuous wave Morse Code with frequency modulation), you can specify a preamble period to happen with the carrier on before the message begins. Set member variable _preamble_enable_ to true to enable this for a transmission. Keep in mind that this member variable acts like a one-shot, so you will need to set it every time you intend to use it.

Startup Conditions and Constraints
----------------------------------
The default output pin is defined as LED_BUILTIN while the default sending speed is 25 words per minute.

The maximum transmit buffer size is 100 characters.

Public Methods
--------------
### Morse() [class constructor]
```
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
Morse::Morse(uint8_t tx_pin, float init_wpm) : output_pin(tx_pin)
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
 * Set the Morse code sending speed.
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
 * Send the specified message in Morse code on the output pin.
 *
 * message - String literal or zero-delimited string to transmit.
 *
 */
void Morse::send(char * message)
```
### reset()
```
/*
 * Morse::reset()
 *
 * Halts any sending in progress, empties the message buffer, and resets the
 * Morse state machine.
 *
 */
```

Public Variables
----------------
    float wpm;
    bool tx;
    bool tx_enable;
    uint8_t output_pin;
    uint8_t led_pin;
    bool dfcw_mode = false;
    bool busy;
    bool preamble_enable;
    uint8_t cur_char = 0;

Valid Characters
----------------
The standard uppercase and lowercase letters 'A' through 'Z' and digits '0' through '9' are of course supported, along with a handful of punctuation and special characters. The following table indicates the special characters supported along with the corresponding Morse code. Other input characters are ignored.

| Input Character   | Morse Character |
|-------------------|-----------------|
| - (minus sign)    | ➖➖ (M)          |
| / (slash)         | ➖••➖•           |
| = (equal sign)    | ➖•••➖ (BT)      |
| ? (question mark) | ••➖➖••          |

Changelog
---------
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
