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

  //timer = DIGITOUT_DEFAULT_TIMER;
  onTimer = { 0, DIGITOUT_DEFAULT_TIMER * 1000, true };

  justToggled = false;
  /* set the default inputType for the classic and timer based light */
  if (type == DIGITOUT_ONOFF || type == DIGITOUT_TIMER) {
    inputType = DIGITOUT_SWITCH_PRESS_RELEASE;
  }
  /* automatically set the inputType for the DIGITOUT_COUNTER as it is the only allowed inputType
   *  it will not work with any onther inputType */
  if (type == DIGITOUT_COUNTER) {
    this->inputType = DIGITOUT_SWITCH_COUNTER_UP;
  }
  buttonPressHold = { 0, 100, true };
}

void Light::setType(byte type) {
  this->type = type;
  if (type == DIGITOUT_COUNTER) {
    this->inputType = DIGITOUT_SWITCH_COUNTER_UP;
    onTimer = { 0, DIGITOUT_SWITCH_COUNTER_DEF_TIMER, true };
    timeRun(&onTimer);
  }
}

void Light::setTimer(unsigned long timer) {
  //this->timer = timer;
  this->onTimer.tTimeout = timer;
}

unsigned long Light::getTimer() {
  //return this->timer;
  return this->onTimer.tTimeout;
}

byte Light::getType() {
  return this->type;
}

byte Light::getDevID() {
  return this->lightID;
}

void Light::setCtrlONEnabled() {
  this->ctrlON = DIGITOUT_CTRLON_ON;
}

void Light::setCtrlONDisabled() {
  this->ctrlON = DIGITOUT_CTRLON_OFF;
}

byte Light::getCtrlON() {
  return this->ctrlON;
}

byte Light::isPressed() {
  if (timeCheck(&onTimer)) {
    if (type == DIGITOUT_TIMER) {
      Serial.println(F("Turning off light after timer expired."));
      setOFF();
    } else if (type == DIGITOUT_COUNTER) {
      timeRun(&onTimer);
      Serial.println(F("reporting counter"));
      /* we just indicate the moment to report the counter, reset shall happen upon reporting */
      return PHY_COUNTER_TIME_TRIGGER;
    }
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
    } else { /* inPinState == LOW */
      if (inPinPressed) { /* Button is released */
        /* EXECUTED ON BUTTON RELEASE - START */
        Serial.println(F("Press detected."));
        /* EXECUTED ON BUTTON RELEASE - END */
        if (timeCheck(&buttonHold)) {
          if (this->lightID == Platform.getLastLightDevID() && getCentralCtrl() == DIGITOUT_CENTRAL_CTRL_ENABLE) {
            Serial.println(F("Central ON pressed and hel"));
            inPinPressed = false;
            return PHY_CENTRAL_CTRL_PRESS_MORE_THAN_2SEC;
          }
          Serial.println(F("Held up above 2 sec"));
          inPinPressed = false;
          return PHY_PRESS_MORE_THAN_2SEC;
        }
        if (this->lightID == Platform.getLastLightDevID() && getCentralCtrl() == DIGITOUT_CENTRAL_CTRL_ENABLE) {
          Serial.println(F("Central ON mementary pressed"));
          inPinPressed = false;
          return PHY_CENTRAL_CTRL_MOMENTARY_PRESS;
        }
        inPinPressed = false;
        delay(10);
        return PHY_MOMENTARY_PRESS;
      } else {
        return PHY_NO_PRESS;
      }
    }
  } else if (inputType == DIGITOUT_SWITCH_PRESS_HOLD || inputType == DIGITOUT_SWITCH_HEAT_OVERRIDE_OFF) {
    delay(10); // this delay here was placed in order for the press button result to be predictable
    if (inPinState != Settings::getInputPinValue(inPin) && inPinPressed == false) { /* indication that state of the input changed */
      inPinState = Settings::getInputPinValue(inPin);
      inPinPressed = true;
      timeRun(&buttonPressHold);
      delay(10); // this delay here was placed in order for the press button result to be predictable
      return PHY_NO_PRESS;
    }
    if (timeCheck(&buttonPressHold)) {
      inPinPressed = false;
      delay(10); // this delay here was placed in order for the press button result to be predictable
      Serial.println(lightID);
      return PHY_MOMENTARY_PRESS;
    }
    return PHY_NO_PRESS; /* function must always return a value even if situation "shouldn't" happen. Otherwise the return value is unpredictable */
  } else if (inputType == DIGITOUT_SWITCH_COUNTER_UP) {
    inPinState = Settings::getInputPinValue(inPin);
    if (inPinState == HIGH) { /* Button pressed and held */
      inPinPressed = true;
      delay(10); // this delay here was placed in order for the press button result to be predictable
      return PHY_NO_PRESS;
    } else { /* inPinState == LOW */
      if (inPinPressed) { /* Button is released */
        /* EXECUTED ON BUTTON RELEASE - START */
        if (counter == PLATFORM_MAX_U_LONG) {
          return PHY_COUNTER_TIME_TRIGGER;
        } else {
          counter++;
        }
        Serial.print(F("new Counter value: "));
        Serial.println(counter);
        /* EXECUTED ON BUTTON RELEASE - END */
        inPinPressed = false;
        delay(10);
        return PHY_NO_PRESS;
      } else {
        return PHY_NO_PRESS;
      }
    }
  } else if (inputType == DIGITOUT_SWITCH_HEAT_OVERRIDE_ON) {
    /* do nothing as we ignore the physical input pin */
    //Serial.println(F("Override is on, doing nothing!"));
    return PHY_NO_PRESS;
  } 
  return PHY_NO_PRESS;
}

void Light::setON() {
  if (type == DIGITOUT_ONOFF || type == DIGITOUT_TIMER) {
    Platform.setOutputPinValue(outPin, Light::high);
    outPinState = Light::high;
    justToggled = true;
    if (type == DIGITOUT_TIMER) {
      timeRun(&onTimer);
    }
  } else if (type == DIGITOUT_SIMPLE_HEAT) {
    if (inputType == DIGITOUT_SWITCH_HEAT_OVERRIDE_ON) {
      Platform.setOutputPinValue(outPin, Light::high);
      outPinState = Light::high;
      justToggled = true;
    } else {
      /* do nothing here */
    }
  }
}

void Light::setOFF() {
  if (type == DIGITOUT_ONOFF || type == DIGITOUT_TIMER) {
    Platform.setOutputPinValue(outPin, Light::low);
    outPinState = Light::low;
    justToggled = true;
  } else if (type == DIGITOUT_SIMPLE_HEAT) {
    if (inputType == DIGITOUT_SWITCH_HEAT_OVERRIDE_ON) {
      Platform.setOutputPinValue(outPin, Light::low);
      outPinState = Light::low;
      justToggled = true;
    } else {
      /* do nothing here */
    }
  }
}

void Light::toggle() {
  if (type == DIGITOUT_SIMPLE_HEAT) {
    inPinState = Settings::getInputPinValue(inPin);
    if (inPinState == HIGH) {
      Platform.setOutputPinValue(outPin, Light::high);
    } else if (inPinState == LOW) {
      Platform.setOutputPinValue(outPin, Light::low);
    } else {
      Serial.println(F("This should never happen"));
    }
  } else if (type == DIGITOUT_COUNTER) {
    /* do nothing the counter type device will not change its output */
  } else {
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
  /* only change the inputType for the allowed light types */
  if (this->type == DIGITOUT_ONOFF || this->type == DIGITOUT_TIMER) {
    inputType = DIGITOUT_SWITCH_PRESS_HOLD;
  }
}

void Light::setInputTypeRelease() {
  if (this->type == DIGITOUT_ONOFF || this->type == DIGITOUT_TIMER) {
    inputType = DIGITOUT_SWITCH_PRESS_RELEASE;
  }
}

void Light::setInputTypeSimpleHeatOverrideOn() {
  if (this->type == DIGITOUT_SIMPLE_HEAT) {
    inputType = DIGITOUT_SWITCH_HEAT_OVERRIDE_ON;
  }
}

void Light::setInputTypeSimpleHeatOverrideOff() {
  if (this->type == DIGITOUT_SIMPLE_HEAT) {
    inputType = DIGITOUT_SWITCH_HEAT_OVERRIDE_OFF;

    /* setting the output pin state based on the input pin state */
    inPinState = Settings::getInputPinValue(inPin);
    if (inPinState == HIGH) {
      Platform.setOutputPinValue(outPin, Light::high);
    } else if (inPinState == LOW) {
      Platform.setOutputPinValue(outPin, Light::low);
    } else {
      /* nothing to do here */
    }
  }
}

void Light::setInputTypeSimpleHeatTempSensor() {
  if (this->type == DIGITOUT_SIMPLE_HEAT) {
    inputType = DIGITOUT_SWITCH_HEAT_TEMP_SENSOR;
  }
}

unsigned long Light::getCounterAndReset() {
  unsigned long reportCounter;
  reportCounter = counter;
  counter = 0;
  return reportCounter;
}

byte Light::getInputType() {
  return inputType;
}
