#include "Shade.h"
#include "Settings.h"

  Shade::Shade(byte shadeID) {
    this->shadeID = shadeID;
    this->outPinUp = Settings::getShadeOutPinUp(shadeID);
    this->outPinDown = Settings::getShadeOutPinDown(shadeID);
    this->inPinUp = Settings::getShadeInPinUp(shadeID);
    this->inPinDown = Settings::getShadeInPinDown(shadeID);
    init();
  }

  void Shade::init() {
    pinMode(inPinUp, INPUT);
    pinMode(inPinDown, INPUT);
    inPinUpState    = LOW;
    inPinDownState  = LOW;
    inPinUpPressed    = false;
    inPinDownPressed  = false;
 
    pinMode(outPinUp, OUTPUT);
    pinMode(outPinDown, OUTPUT);

    Shade::low = Settings::getLow();
    Shade::high = Settings::getHigh();
    digitalWrite(outPinUp, Shade::low);
    digitalWrite(outPinDown, Shade::low);
    outPinUpState = Shade::low;
    outPinDownState = Shade::low;
  }

  bool Shade::isUpPressed() {
    inPinUpState = digitalRead(inPinUp);
    if (inPinUpState == HIGH) { /* Button pressed and held */
      inPinUpPressed = true;
      delay(10); // this delay here was placed in order for the press button result to be predictable
      return false;
    } else {
      if (inPinUpPressed) {
        /* EXECUTED ON BUTTON RELEASE - START */
        return true;
        /* EXECUTED ON BUTTON RELEASE - END */
      } else {
        inPinUpPressed = false;
        return false;
      }
    }
  }

  void Shade::isDownPressed() {
    inPinDownState = digitalRead(inPinDown);
    if (inPinDownState == HIGH) { /* Button pressed and held */
      inPinDownPressed = true;
      delay(10); // this delay here was placed in order for the press button result to be predictable
      return false;
    } else {
      if (inPinDownPressed) {
        /* EXECUTED ON BUTTON RELEASE - START */
        return true;
        /* EXECUTED ON BUTTON RELEASE - END */
      } else {
        inPinDownPressed = false;
        return false;
      }
    }
  }

  void Shade::Up() {
    
  }

  void Shade::Down() {
    
  }

  void Shade::Stop() {
    
  }
