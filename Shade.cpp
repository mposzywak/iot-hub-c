#include "Shade.h"
#include "Settings.h"

byte Shade::low = Settings::getLow();
byte Shade::high = Settings::getHigh();



Shade::Shade() {

  //init();
}

void Shade::init(byte shadeID) {
  this->shadeID = shadeID;
  this->outPinUp = Settings::getShadeOutPinUp(shadeID);
  this->outPinDown = Settings::getShadeOutPinDown(shadeID);
  this->inPinUp = Settings::getShadeInPinUp(shadeID);
  this->inPinDown = Settings::getShadeInPinDown(shadeID);
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

  position = 0;
  desiredPosition = 0;
  movementRange = DEFAULT_RANGE;
  synced = false;
  oldSec = 0;
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

      /* EXECUTED ON BUTTON RELEASE - END */
      inPinUpPressed = false;
      return true;
    } else {
      return false;
    }
  }
}

bool Shade::isDownPressed() {
  this->update();
  inPinDownState = digitalRead(inPinDown);
  if (inPinDownState == HIGH) { /* Button pressed and held */
    inPinDownPressed = true;
    delay(10); // this delay here was placed in order for the press button result to be predictable
    return false;
  } else {
    if (inPinDownPressed) {
      /* EXECUTED ON BUTTON RELEASE - START */

      /* EXECUTED ON BUTTON RELEASE - END */
      inPinDownPressed = false;
      return true;
    } else {
      return false;
    }
  }
}

void Shade::update() {
  int sec = Controllino_GetSecond();

  if (sec != Shade::oldSec) {
    oldSec = sec;
    /* CODE EXECUTED EVERY SECOND - START */
    Serial.print("ShadeID: ");
    Serial.print(shadeID);
    Serial.print(" position: ");
    Serial.println(position);
    bool movingUp = this->isMovingUp();
    bool movingDown = this->isMovingDown();
    if (!synced) {
      if (movingUp && position > 0) {
        position--;
      } else if (movingUp && position == 0) {
        synced = true;
        this->stop();
        Serial.println("Shade synced");
      }
      if (movingDown && position < movementRange) {
        position++;
      } else if (movingDown && position == movementRange) {
        synced = true;
        this->stop();
        Serial.println("Shade synced");
      } 
    } else {
      if (movingUp && position > desiredPosition) {
        position--;
      } else if (movingUp && position == desiredPosition) {
        this->stop();
      }
      if (movingDown && position < desiredPosition) {
        position++;
      } else if (movingDown && position == desiredPosition) {
        this->stop();
      }
      
    }
    /* CODE EXECUTED EVERY SECOND - END */
  }
}

void Shade::up() {
  digitalWrite(outPinUp, Shade::high);
  digitalWrite(outPinDown, Shade::low);
  outPinUpState = Shade::high;
  if (!synced) {
    position = movementRange;
  } else {
    desiredPosition = 0;
  }
}

void Shade::down() {
  digitalWrite(outPinUp, Shade::low);
  digitalWrite(outPinDown, Shade::high);
  outPinDownState = Shade::high;
  if (!synced) {
    position = 0;
  } else {
    desiredPosition = DEFAULT_RANGE;
  }
}

void Shade::stop() {
  digitalWrite(outPinUp, Shade::low);
  digitalWrite(outPinDown, Shade::low);
  outPinUpState = Shade::low;
  outPinDownState = Shade::low;
}

bool Shade::isMoving() {
  if (outPinDownState == Shade::high || outPinUpState == Shade::high) {
    return true;
  } else {
    return false;
  }
}

bool Shade::isMovingUp() {
  if (outPinUpState == Shade::high) {
    return true;
  } else {
    return false;
  }
}

bool Shade::isMovingDown() {
  if (outPinDownState == Shade::high) {
    return true;
  } else {
    return false;
  }

}
