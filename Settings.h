
#include <Arduino.h>

#ifndef SETTINGS_H_
#define SETTINGS_H_

#if defined(CONTROLLINO_MEGA)
#include <Controllino.h>
#define IN_PINS  21
#define OUT_PINS 21
#define SHADES   10
#elif defined(CONTROLLINO_MAXI)
#include <Controllino.h>
#define IN_PINS  12
#define OUT_PINS 12
#define SHADES   6
#elif defined(ARDUINO_AVR_MEGA2560)
#define IN_PINS  28
#define OUT_PINS 28
#define SHADES   14
#endif

class Settings {
  private:

    /* The input pin array */
    static byte digitIN[IN_PINS];

    /* The output pin array */
    static byte digitOUT[OUT_PINS];

  public:

    /* The input pin devID array */
    static byte digitINdevID[IN_PINS];

    /* the output pin devID array */
    static byte digitOUTdevID[OUT_PINS];

    /* the shadeID array */
    static byte shadeIDs[SHADES];


    /* variable sets how much time the shade motor should be running (in seconds) */
    static const byte runTimer = 10;

    /* variable controlling if the relays are set to NC or NO
     *  NC - Normally Closed - the digitOUT is by default in LOW state, LightON -> HIGH state (true)
     *  NO - Normally Open   - the digitOUT is by default in HIGH state, LightON -> LOW state (false)
     */
    static const bool relaysNC = true;
    static bool getLow();
    static bool getHigh();

    /* Get the Shade PINs based on the shade ID */
    static byte getShadeOutPinUp(byte shadeID);
    static byte getShadeInPinUp(byte shadeID);
    static byte getShadeOutPinDown(byte shadeID);
    static byte getShadeInPinDown(byte shadeID);

    /* set input pin mode (platform independent) */
    static void Settings::setInPinMode(uint8_t pin);

    /* get input pin value (platform independent) */
    static int Settings::getInputPinValue(uint8_t pin);

    /* set output pin value (platform independent) */
    static void Settings::setOutputPinValue(uint8_t pin, uint8_t value);

    /* initialize the platform */
    static void Settings::initPlatform();
};

#endif
