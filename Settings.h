#include <Controllino.h>
#include <Arduino.h>

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

#if defined(CONTROLLINO_MEGA)
/* The input pin array */
    static byte digitIN[IN_PINS] =   { CONTROLLINO_A0, 
                                       CONTROLLINO_A1, 
                                       CONTROLLINO_A2,
                                       CONTROLLINO_A3,
                                       CONTROLLINO_A4,
                                       CONTROLLINO_A5,
                                       CONTROLLINO_A6,
                                       CONTROLLINO_A7,
                                       CONTROLLINO_A8,
                                       CONTROLLINO_A9,
                                       CONTROLLINO_A10,
                                       CONTROLLINO_A11,
                                       CONTROLLINO_A12,
                                       CONTROLLINO_A13,
                                       CONTROLLINO_A14,
                                       CONTROLLINO_A15,
                                       CONTROLLINO_I16,
                                       CONTROLLINO_I17,
                                       CONTROLLINO_I18,
                                       CONTROLLINO_IN0,
                                       CONTROLLINO_IN1};

    /* The output pin array */
    static byte digitOUT[OUT_PINS] = { CONTROLLINO_D0, 
                            CONTROLLINO_D1,
                            CONTROLLINO_D2,
                            CONTROLLINO_D3,
                            CONTROLLINO_D4,
                            CONTROLLINO_D5,
                            CONTROLLINO_D6,
                            CONTROLLINO_D7,
                            CONTROLLINO_D8,
                            CONTROLLINO_D9,
                            CONTROLLINO_D10,
                            CONTROLLINO_D11,
                            CONTROLLINO_D12,
                            CONTROLLINO_D13,
                            CONTROLLINO_D14,
                            CONTROLLINO_D15,
                            CONTROLLINO_D16,
                            CONTROLLINO_D17,
                            CONTROLLINO_D18,
                            CONTROLLINO_D19,
                            CONTROLLINO_D20};

    /* The input pin devID array */
    static byte digitINdevID[IN_PINS] = { 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30 };

    /* the output pin devID array */
    static byte digitOUTdevID[OUT_PINS] = { 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70 };

    /* the shadeID array */
    //static byte shades[SHADES] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

#elif defined(CONTROLLINO_MAXI) 
    static byte digitIN[IN_PINS] =   { CONTROLLINO_A0, 
                            CONTROLLINO_A1, 
                            CONTROLLINO_A2,
                            CONTROLLINO_A3,
                            CONTROLLINO_A4,
                            CONTROLLINO_A5,
                            CONTROLLINO_A6,
                            CONTROLLINO_A7,
                            CONTROLLINO_A8,
                            CONTROLLINO_A9,
                            CONTROLLINO_IN0,
                            CONTROLLINO_IN1};

    /* The output pin array */
    static byte digitOUT[OUT_PINS] = { CONTROLLINO_D0, 
                            CONTROLLINO_D1,
                            CONTROLLINO_D2,
                            CONTROLLINO_D3,
                            CONTROLLINO_D4,
                            CONTROLLINO_D5,
                            CONTROLLINO_D6,
                            CONTROLLINO_D7,
                            CONTROLLINO_D8,
                            CONTROLLINO_D9,
                            CONTROLLINO_D10,
                            CONTROLLINO_D11};
                            
    /* The input pin devID array */
    static byte digitINdevID[IN_PINS] = { 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21 };

    /* the output pin devID array */
    static byte digitOUTdevID[OUT_PINS] = { 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61 };

    /* the shadeID array */
    //static byte shades[SHADES] = { 1, 2, 3, 4, 5, 6};

#endif
  public:

    /* variable sets how much time the shade motor should be running (in seconds) */
    static const byte runTimer = 10;

    /* variable controlling if the relays are set to NC or NO
     *  NC - Normally Closed - the digitOUT is by default in LOW state, LightON -> HIGH state (true)
     *  NO - Normally Open   - the digitOUT is by default in HIGH state, LightON -> LOW state (false)
     */
    static const bool relaysNC = false;

    static bool getLow();

    static bool getHigh();

    static byte getShadeOutPinUp(byte shadeID);

    static byte getShadeInPinUp(byte shadeID);

    static byte getShadeOutPinDown(byte shadeID);

    static byte getShadeInPinDown(byte shadeID);
};
