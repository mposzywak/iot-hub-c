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
  justStoppedVar = false;
  oldSec = 0;

  /* filling in the section borders with the border seconds of the movement range */
  sections[0] = 0;
  sections[4] = DEFAULT_RANGE;
  for (int i = 1; i < DEFAULT_PARTS; i++) {
    sections[i] = DEFAULT_RANGE / DEFAULT_PARTS * i;
  }
  sections[4] = DEFAULT_RANGE;
  positionReported = false;
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

byte Shade::update() {
  int sec = Controllino_GetSecond();
  
  if (sec != oldSec) {
    oldSec = sec;
    /* CODE EXECUTED EVERY SECOND - START */    
    bool movingUp = this->isMovingUp();
    bool movingDown = this->isMovingDown();
    if (!synced) {
      if (movingUp && position > 0) {
        position--;
        positionReported = false;
      } else if (movingUp && position == 0) {
        synced = true;
        this->stop();
        Serial.println("Shade synced");
      }
      if (movingDown && position < movementRange) {
        position++;
        positionReported = false;
      } else if (movingDown && position == movementRange) {
        synced = true;
        this->stop();
        Serial.println("Shade synced");
      } 
    } else {
      if (position > desiredPosition) { 
        if (!movingUp)
          upToPosition(desiredPosition); /* in this case the argument doesn't change anything as the desiredPosition has already been set */
        position--;
        positionReported = false;
        Serial.println("setting reported false");
      } else if (movingUp && position == desiredPosition) {
        this->stop();
      }
      if (position < desiredPosition) {
        if (!movingDown)
          downToPosition(desiredPosition); /* in this case the argument doesn't change anything as the desiredPosition has already been set */
        position++;
        positionReported = false;
        Serial.println("setting reported false");
      } else if (movingDown && position == desiredPosition) {
        this->stop();
      }
      
    }

    if (position == sections[0] && positionReported == false && synced) {
      Serial.println("setting reported true");
      reachedPosition = 0;
      return 0;
    } else if (position == sections[1] && positionReported == false && synced) {
      Serial.println("setting reported true");
      reachedPosition = 25;
      return 25;
    } else if (position == sections[2] && positionReported == false && synced) {
      Serial.println("setting reported true");
      reachedPosition = 50;
      return 50;
    } else if (position == sections[3] && positionReported == false && synced) {
      Serial.println("setting reported true");
      reachedPosition = 75;
      return 75; 
    } else if (position == sections[4] && positionReported == false && synced) {
      Serial.println("setting reported true");
      reachedPosition = 100;
      return 100;
    } else {
        return 255;
    }
    /* CODE EXECUTED EVERY SECOND - END */
  } else {
    return 255; /* this is needed for the update function to return something meaningful if it 
    runs second and more times during a second. Apparently if there is no return value statement 
    specified in the function code path, it gets randomized. That in our case here caused unpredictable
    results */
  }
}

void Shade::up() {
  this->upToPosition(0);
}


void Shade::down() {
  this->downToPosition(DEFAULT_RANGE);
}

void Shade::upToPosition(byte dp) {
  digitalWrite(outPinUp, Shade::high);
  digitalWrite(outPinDown, Shade::low);
  outPinUpState = Shade::high;
  if (!synced) {
    position = movementRange;
  } else {
    desiredPosition = dp;
  }
  justStartedUpVar = true;
}

void Shade::downToPosition(byte dp) {
  digitalWrite(outPinUp, Shade::low);
  digitalWrite(outPinDown, Shade::high);
  outPinDownState = Shade::high;
  if (!synced) {
    position = 0;
  } else {
    desiredPosition = dp;
  }
  justStartedDownVar = true;
}

void Shade::stop() {
  digitalWrite(outPinUp, Shade::low);
  digitalWrite(outPinDown, Shade::low);
  outPinUpState = Shade::low;
  outPinDownState = Shade::low;
  justStoppedVar = true;
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

bool Shade::justStopped() {
  if (justStoppedVar) {
    justStoppedVar = false;
    return true;
  } else {
    return false;
  }
}

bool Shade::justStartedUp() {
  if (justStartedUpVar) {
    justStartedUpVar = false;
    return true;
  } else {
    return false;
  }
}

bool Shade::justStartedDown() {
  if (justStartedDownVar) {
    justStartedDownVar = false;
    return true;
  } else {
    return false;
  }
}

byte Shade::getCurrentPosition() {
  positionReported = true;
  return reachedPosition;
}

void Shade::toPosition(byte position) {
  if (position == 0) desiredPosition = sections[0];
  if (position == 25) desiredPosition = sections[1];
  if (position == 50) desiredPosition = sections[2];
  if (position == 75) desiredPosition = sections[3];
  if (position == 100) desiredPosition = sections[4];
}
