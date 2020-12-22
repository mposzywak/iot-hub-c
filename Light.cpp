#include "Light.h"
#include "Settings.h"

byte Light::low = Settings::getLow();
byte Light::high = Settings::getHigh();



Light::Light() {

  //init();
}

void Light::init(byte lightID, byte type) {
  this->lightID = lightID;
  this->type = type;
  this->outPin = Platform.getLightOutPin(lightID);
  this->inPin = Platform.getLightInPin(lightID);
  Platform.setInPinMode(inPin);

  pinMode(outPin, OUTPUT);

  inPinState  = LOW;
  inPinPressed    = false;

  Light::low = Settings::getLow();
  Light::high = Settings::getHigh();

  Platform.setOutputPinValue(outPin, Light::low);

  outPinState = Light::low;

  buttonHold = { 0, 1000, true };

  timer = DEFAULT_TIMER;
  onTimer = { 0, timer * 1000, true };
  
  justToggled = false;
}

void Light::setType(byte type) {
  this->type = type;
  if (type == DIGITOUT_TIMER) {
    this->timer = DEFAULT_TIMER;
  }
}

void Light::setTimer(unsigned long timer) {
  this->timer = timer;
  this->onTimer.tTimeout = timer * 1000;
}

byte Light::getType() {
  return this->type;
}

byte Light::getDevID() {
  return this->lightID;
}

byte Light::isPressed() {
  if (timeCheck(&onTimer)) {
    Serial.println("Turning off light after timer expired.");
    setOFF();
  }
  
  inPinState = Settings::getInputPinValue(inPin);
  if (inPinState == HIGH) { /* Button pressed and held */
    if (!inPinPressed) { /* at the moment of pressing start counting time */
      timeRun(&buttonHold);
    }
    inPinPressed = true;
    delay(10); // this delay here was placed in order for the press button result to be predictable
    return PHY_NO_PRESS;
  } else {
    if (inPinPressed) { /* Button is released */
      /* EXECUTED ON BUTTON RELEASE - START */

      /* EXECUTED ON BUTTON RELEASE - END */
      if (timeCheck(&buttonHold)) {
        Serial.println("Held up above 2 sec");
        inPinPressed = false;
        return PHY_PRESS_MORE_THAN_2SEC;
      }
      inPinPressed = false;
      return PHY_MOMENTARY_PRESS;
    } else {
      return PHY_NO_PRESS;
    }
  }
}

void Light::setON() {
  Platform.setOutputPinValue(outPin, Light::high);
  outPinState = Light::high;
  justToggled = true;
  if (type == DIGITOUT_TIMER) {
      timeRun(&onTimer);
  }
}

void Light::setOFF() {
  Platform.setOutputPinValue(outPin, Light::low);
  outPinState = Light::low;
  justToggled = true;
}

void Light::toggle() {
  if (outPinState == Light::low) {
    outPinState = Light::high;
    Platform.setOutputPinValue(outPin, Light::high);
    if (type == DIGITOUT_TIMER) {
      timeRun(&onTimer);
    }
  } else {
    outPinState = Light::low;
    Platform.setOutputPinValue(outPin, Light::low);
  }
  justToggled = true;
}

bool Light::justTurnedON() {
  if (justToggled == true && outPinState == Light::high) {
    justToggled = false;
    return true;
  } else {
    return false;
  }
}

bool Light::justTurnedOFF() {
  if (justToggled == true && outPinState == Light::low) {
    justToggled = false;
    return true;
  } else {
    return false;
  }
}

bool Light::getStatus() {
  if (outPinState == Light::low) {
    return false;
  } else {
    return true;
  }
}

bool Light::timeCheck(struct t *t ) {
  if ((unsigned long)(millis() - t->tStart) > t->tTimeout && t->executed == false) {
    t->executed = true;
    return true;
  } else {
    return false;
  }
}

void Light::timeRun(struct t *t) {
  t->tStart = millis();
  t->executed = false;
}

void Light::reset() {
  Platform.setOutputPinValue(outPin, Light::low);
  outPinState = Light::low;
  justToggled = false;
}
