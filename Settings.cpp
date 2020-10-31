#include "Settings.h"

/* ----- Platform related defines - START ----- */

#if defined(CONTROLLINO_MEGA)

#include <Controllino.h>

#elif defined(CONTROLLINO_MAXI)

#include <Controllino.h>

#elif defined(ARDUINO_AVR_MEGA2560)
/* nothing needed here */

#else 
  #error "Unsupported board"
#endif

/* ----- Platform related defines - END ----- */

#if defined(CONTROLLINO_MEGA)
/* The input pin array */
    static byte Settings::digitIN[IN_PINS] =   { CONTROLLINO_A0, 
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
                                                 CONTROLLINO_IN1 };

    /* The output pin array */
    static byte Settings::digitOUT[OUT_PINS] = { CONTROLLINO_D0, 
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
                                                 CONTROLLINO_D20 };

    /* The input pin devID array */
    static byte Settings::digitINdevID[IN_PINS] = { 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30 };

    /* the output pin devID array */
    static byte Settings::digitOUTdevID[OUT_PINS] = { 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70 };

    /* the shadeID array */
    static byte Settings::shadesIDs[SHADES] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

#elif defined(CONTROLLINO_MAXI) 
    static byte Settings::digitIN[IN_PINS] =   { CONTROLLINO_A0, 
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
                                                 CONTROLLINO_IN1 };

    /* The output pin array */
    static byte Settings::digitOUT[OUT_PINS] = { CONTROLLINO_D0, 
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
                                                 CONTROLLINO_D11 };
                            
    /* The input pin devID array */
    static byte Settings::digitINdevID[IN_PINS] = { 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21 };

    /* the output pin devID array */
    static byte Settings::digitOUTdevID[OUT_PINS] = { 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61 };

    /* the shadeID array */
    static byte Settings::shadesIDs[SHADES] = { 1, 2, 3, 4, 5, 6 };
    
#elif defined(ARDUINO_AVR_MEGA2560)
    static byte Settings::digitIN[IN_PINS] =   { 2, 
                                                 3, 
                                                 5,
                                                 6,
                                                 7,
                                                 8,
                                                 9,
                                                 14,
                                                 15,
                                                 16,
                                                 17,
                                                 18,
                                                 19,
                                                 20,
                                                 21,
                                                 22,
                                                 23,
                                                 24,
                                                 25,
                                                 26,
                                                 27,
                                                 28,
                                                 29,
                                                 30,
                                                 31,
                                                 32,
                                                 33,
                                                 34 };

    /* The output pin array */
    static byte Settings::digitOUT[OUT_PINS] = { 38, 
                                                 39,
                                                 40,
                                                 41,
                                                 42,
                                                 43,
                                                 44,
                                                 45,
                                                 46,
                                                 47,
                                                 48,
                                                 49,
                                                 50,
                                                 51,
                                                 52,
                                                 53,
                                                 54,
                                                 55,
                                                 56,
                                                 57,
                                                 58,
                                                 59,
                                                 60,
                                                 61,
                                                 62,
                                                 63,
                                                 64,
                                                 65 };
                            
    /* The input pin devID array */
    static byte Settings::digitINdevID[IN_PINS] = { 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21 };

    /* the output pin devID array */
    static byte Settings::digitOUTdevID[OUT_PINS] = { 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61 };

    /* the shadeID array */
    static byte Settings::shadeIDs[SHADES] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 };
    
#else 
  #error "Unsupported board"
#endif

static bool Settings::getLow() {
  if (Settings::relaysNC)
    return LOW;
  else
    return HIGH;
}

static bool Settings::getHigh() {
  if (Settings::relaysNC)
    return HIGH;
  else
    return LOW;
}

static byte Settings::getShadeOutPinUp(byte shadeID) {
  return Settings::digitOUT[(shadeID - 1) * 2];
}

static byte Settings::getShadeInPinUp(byte shadeID) {
  return Settings::digitIN[(shadeID - 1) * 2];
}

static byte Settings::getShadeOutPinDown(byte shadeID) {
  return Settings::digitOUT[((shadeID - 1) * 2) + 1];
}

static byte Settings::getShadeInPinDown(byte shadeID) {
  return Settings::digitIN[((shadeID - 1) * 2) + 1];
}

static void Settings::setInPinMode(uint8_t pin) {
#if defined(CONTROLLINO_MEGA)
  pinMode(pin, INPUT);
#elif defined(CONTROLLINO_MAXI)
  pinMode(pin, INPUT);
#elif defined(ARDUINO_AVR_MEGA2560)
  pinMode(pin, INPUT_PULLUP);
#endif
}

static int Settings::getInputPinValue(uint8_t pin) {
#if defined(CONTROLLINO_MEGA)
  return digitalRead(pin);
#elif defined(CONTROLLINO_MAXI)
  return digitalRead(pin);
#elif defined(ARDUINO_AVR_MEGA2560)
  int pinValue;
  pinValue = digitalRead(pin);
  if (pinValue == LOW) {
    return HIGH;
  } else {
    return LOW;
  }
#endif
}

static void Settings::setOutputPinValue(uint8_t pin, uint8_t value) {
#if defined(CONTROLLINO_MEGA)
  digitalWrite(pin, value);
#elif defined(CONTROLLINO_MAXI)
  digitalWrite(pin, value);
#elif defined(ARDUINO_AVR_MEGA2560)
  if (value == HIGH) {
    digitalWrite(pin, LOW);
  } else {
    digitalWrite(pin, HIGH);
  }
#endif
}

static void Settings::initPlatform() {
#if defined(CONTROLLINO_MEGA)
  Controllino_RTC_init(0);
  //Controllino_SetTimeDate(7,4,11,19,11,00,45);
#elif defined(CONTROLLINO_MAXI) 
  Controllino_RTC_init(0);
#elif defined(ARDUINO_AVR_MEGA2560)
  /* nothing needed here */
#endif
}
