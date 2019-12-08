#include <Controllino.h>
#include <Arduino.h>

#ifndef SETTINGS_H_
#define SETTINGS_H_

#if defined(CONTROLLINO_MEGA)
#define IN_PINS  21
#define OUT_PINS 21
#define SHADES   10
#elif defined(CONTROLLINO_MAXI) 
#define IN_PINS  12
#define OUT_PINS 12
#define SHADES   6
#endif

class Settings {
  private:

    /* The input pin array */
    static byte digitIN[IN_PINS];

    /* The output pin array */
    static byte digitOUT[OUT_PINS];

    /* The input pin devID array */
    static byte digitINdevID[IN_PINS];

    /* the output pin devID array */
    static byte digitOUTdevID[OUT_PINS];

    /* the shadeID array */
    //static byte shades[SHADES];


  public:

    /* variable sets how much time the shade motor should be running (in seconds) */
    static const byte runTimer = 10;

    /* variable controlling if the relays are set to NC or NO
     *  NC - Normally Closed - the digitOUT is by default in LOW state, LightON -> HIGH state (true)
     *  NO - Normally Open   - the digitOUT is by default in HIGH state, LightON -> LOW state (false)
     */
    static const bool relaysNC = true;

    static bool getLow();

    static bool getHigh();

    static byte getShadeOutPinUp(byte shadeID);

    static byte getShadeInPinUp(byte shadeID);

    static byte getShadeOutPinDown(byte shadeID);

    static byte getShadeInPinDown(byte shadeID);
};

#endif
