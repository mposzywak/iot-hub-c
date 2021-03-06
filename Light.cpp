#include "Light.h"
#include "Settings.h"

byte Light::low = Settings::getLow();
byte Light::high = Settings::getHigh();

static byte Light::centralControlEnabled;

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

  timer = DIGITOUT_DEFAULT_TIMER;
  onTimer = { 0, timer * 1000, true };

  justToggled = false;
  inputType = DIGITOUT_SWITCH_PRESS_RELEASE;
  buttonPressHold = { 0, 100, true };
}

void Light::setType(byte type) {
  this->type = type;
  if (type == DIGITOUT_TIMER) {
    this->timer = DIGITOUT_DEFAULT_TIMER;
  }
}

void Light::setTimer(unsigned long timer) {
  this->timer = timer;
  this->onTimer.tTimeout = timer;
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
  if (inputType == DIGITOUT_SWITCH_PRESS_RELEASE) {
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
          if (this->lightID == Platform.getLastLightDevID() && getCentralCtrl() == DIGITOUT_CENTRAL_CTRL_ENABLE) {
            Serial.println("Central ON pressed and hel");
            inPinPressed = false;
            return PHY_CENTRAL_CTRL_PRESS_MORE_THAN_2SEC;
          }
          Serial.println("Held up above 2 sec");
          inPinPressed = false;
          return PHY_PRESS_MORE_THAN_2SEC;
        }
        if (this->lightID == Platform.getLastLightDevID() && getCentralCtrl() == DIGITOUT_CENTRAL_CTRL_ENABLE) {
          Serial.println("Central ON mementary pressed");
          inPinPressed = false;
          return PHY_CENTRAL_CTRL_MOMENTARY_PRESS;
        }
        inPinPressed = false;
        return PHY_MOMENTARY_PRESS;
      } else {
        return PHY_NO_PRESS;
      }
    }
  } else { /* inputType == DIGITOUT_SWITCH_PRESS_HOLD */
    delay(10); // this delay here was placed in order for the press button result to be predictable
    if (inPinState != Settings::getInputPinValue(inPin) && inPinPressed == false) { /* indication that state of the input changed */
      inPinState = Settings::getInputPinValue(inPin);
      inPinPressed = true;
      
      Serial.println("Press button -----");
      timeRun(&buttonPressHold);
      delay(10); // this delay here was placed in order for the press button result to be predictable
      return PHY_NO_PRESS;
    }
    if (timeCheck(&buttonPressHold)) {
      Serial.println("Toggle output");
      inPinPressed = false;
      delay(10); // this delay here was placed in order for the press button result to be predictable
      return PHY_MOMENTARY_PRESS;
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

static void Light::enableCentralCtrl() {
  centralControlEnabled = DIGITOUT_CENTRAL_CTRL_ENABLE;
}

static void Light::disableCentralCtrl() {
  centralControlEnabled = DIGITOUT_CENTRAL_CTRL_DISABLE;
}

static byte Light::getCentralCtrl() {
  return centralControlEnabled;
}

void Light::setInputTypeHold() {
  inputType = DIGITOUT_SWITCH_PRESS_HOLD;
}

void Light::setInputTypeRelease() {
  inputType = DIGITOUT_SWITCH_PRESS_RELEASE;
}

byte Light::getInputType() {
  return inputType;
}
