#include "Settings.h"


static bool getLow() {
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
      return Settings::digitOUT[shadeID / 2];
    }

    static byte Settings::getShadeInPinUp(byte shadeID) {
      return Settings::digitIN[shadeID / 2];
    }

    static byte Settings::getShadeOutPinDown(byte shadeID) {
      return Settings::digitOUT[(shadeID / 2) + 1];
    }

    static byte Settings::getShadeInPinDown(byte shadeID) {
      return Settings::digitIN[(shadeID / 2) + 1];
    }
