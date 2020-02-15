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

  /* filling in the section borders with the border seconds of the movement range */
  sections[0] = 0;
  sections[4] = DEFAULT_RANGE;
  for (int i = 1; i < DEFAULT_PARTS; i++) {
    sections[i] = DEFAULT_RANGE / DEFAULT_PARTS * i;
  }
  sections[4] = DEFAULT_RANGE;
  positionReported = false;
  unsyncReported = false;

  /* filling in the section borders with the border miliseconds of the tilt movement range */
  tiltRange = DEFAULT_TILT_RANGE;
  tiltSections[0] = DEFAULT_TILT_RANGE;
  tiltSections[1] = DEFAULT_TILT_RANGE / 2;
  tiltSections[2] = 0;

  dir_swap = { 0, 300, true }; /* The 300ms timer to wait before changing direction up/down */
  updateExec = { 0, 1000, false }; /* The timer to control the update() frequency */
  tiltRun = { 0, 500, true };
  waitBeforeTilt = { 0, 500, true };

  desiredTilt = TILT_H_CLOSED;
  tiltMovement = false;
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

  if (timeCheck(&dir_swap)) {
    Serial.println("Executing delayed change *");
    Serial.println(millis());
    if (swapDirection == true) {
      digitalWrite(outPinUp, Shade::high);
      outPinUpState = Shade::high;
    } else { /* swapDirection == false */
      digitalWrite(outPinDown, Shade::high);
      outPinDownState = Shade::high;
    }
  }

  if (timeCheck(&waitBeforeTilt)) {
    /* waited time to start the tilt movement */
    tiltMovement = true;
    Serial.println("Starting tilt movement");
    if (tiltDirection == true) {
      digitalWrite(outPinUp, Shade::high);
      //outPinUpState = Shade::high;
    } else { /* tiltDirection == false */
      digitalWrite(outPinDown, Shade::high);
      //outPinDownState = Shade::high;
    }
    timeRun(&tiltRun);
  }

  if (timeCheck(&tiltRun)) {
    Serial.println("Stopping tilt movement");
    tiltMovement = false;
    this->stop();
  }
  
  if (timeCheck(&updateExec)) { 
    /* no combining with any other condition! If timeCheck executes but there is no timeRun(), 
       this code will stop executing.  */
    /* CODE EXECUTED EVERY SECOND - START */    
    bool movingUp = this->isMovingUp();
    bool movingDown = this->isMovingDown();
    /*if (shadeID == 1) {
      Serial.print("position: ");
      Serial.print(position);
      Serial.print(" desired position: ");
      Serial.print(desiredPosition);
      Serial.print(" down: ");
      Serial.print(movingDown);
      Serial.print(" up: ");
      Serial.println(movingUp);
      Serial.println(tiltMovement);
    }*/
 
    if (!synced) { /* not synced */
      if (movingUp && position > 0) {
        position--;
        positionReported = false;
      } else if (movingUp && position == 0) {
        synced = true;
        this->stop();
        //setTiltFromUp();
        Serial.println("Shade synced");
      }
      if (movingDown && position < movementRange) {
        position++;
        positionReported = false;
      } else if (movingDown && position == movementRange) {
        synced = true;
        this->stop();
        //setTiltFromDown();
        Serial.println("Shade synced");
      } 
    } else { /* synced */
      if (position > desiredPosition) { /* need to move up */
        if (!movingUp)
          upToPosition(desiredPosition); /* in this case the argument doesn't change anything as the desiredPosition has already been set */
        position--;
        positionReported = false;
        Serial.println("setting reported false");
      } else if (movingUp && position == desiredPosition) {
        Serial.println("Reached pos by moving Up");
        this->stop();
        setTiltFromUp();
      }
      if (position < desiredPosition) { /* need to move down */
        if (!movingDown)
          downToPosition(desiredPosition); /* in this case the argument doesn't change anything as the desiredPosition has already been set */
        position++;
        positionReported = false;
        Serial.println("setting reported false");
      } else if (movingDown && position == desiredPosition) {
        Serial.println("Reached pos by moving Down");
        this->stop();
        setTiltFromDown();
      }
      
    }
    timeRun(&updateExec);
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
  digitalWrite(outPinDown, Shade::low);
  if (outPinDownState == Shade::high) { /* on condition shade was moving in the opposite direction */
    timeRun(&dir_swap);
    swapDirection = true;
  } else { /* on condition shade was already moving in the desired direction or stopped */
    digitalWrite(outPinUp, Shade::high);
    outPinUpState = Shade::high;
  }
  outPinDownState = Shade::low;
  if (!synced) {
    position = movementRange;
  } else {
    desiredPosition = dp;
  }
  justStartedUpVar = true;
}

void Shade::downToPosition(byte dp) {
  digitalWrite(outPinUp, Shade::low);
  if (outPinUpState == Shade::high) { /* on condition shade was moving in the opposite direction */
    timeRun(&dir_swap);
    swapDirection = false;
  } else { /* on condition shade was already moving in the desired direction or stopped */
    digitalWrite(outPinDown, Shade::high);
    outPinDownState = Shade::high;
  }
  outPinUpState = Shade::low;
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
  tiltMovement = false;
  desiredPosition = position;
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

byte Shade::getDevID() {
  return this->shadeID;
}

bool Shade::isSynced() {
  if (!unsyncReported) {
    unsyncReported = true;
    return synced;
  } else {
    return true;
  }
}

bool Shade::timeCheck(struct t *t ) {
  if ((unsigned long)(millis() - t->tStart) > t->tTimeout && t->executed == false) {
    t->executed = true;
    return true;    
  } else {
    return false;
  }
}

byte Shade::getTilt() {
  return desiredTilt;
}

void Shade::timeRun(struct t *t) {
    t->tStart = millis();
    t->executed = false;
}

void Shade::setTilt(byte tilt) {
  byte oldTilt;
  oldTilt = desiredTilt;
  desiredTilt = tilt;
  if (!this->isMoving() && (oldTilt != desiredTilt)) {
    Serial.println("Shade not moving. Triggering tilt change");
    if        (oldTilt == TILT_F_CLOSED && desiredTilt == TILT_F_CLOSED) {
      /* nothing */
    } else if (oldTilt == TILT_F_CLOSED && desiredTilt == TILT_H_CLOSED) {
      tiltRun.tTimeout = TILT_HALF_MOVE;
      tiltDirection = false;
      timeRun(&waitBeforeTilt);
    } else if (oldTilt == TILT_F_CLOSED && desiredTilt == TILT_F_OPEN) {
      tiltRun.tTimeout = TILT_FULL_MOVE;
      tiltDirection = false;
      timeRun(&waitBeforeTilt);
    } else if (oldTilt == TILT_H_CLOSED && desiredTilt == TILT_F_CLOSED) {
      tiltRun.tTimeout = TILT_HALF_MOVE;
      tiltDirection = true;
      timeRun(&waitBeforeTilt);
    } else if (oldTilt == TILT_H_CLOSED && desiredTilt == TILT_H_CLOSED) {
      /* nothing */
    } else if (oldTilt == TILT_H_CLOSED && desiredTilt == TILT_F_OPEN) {
      tiltRun.tTimeout = TILT_HALF_MOVE;
      tiltDirection = false;
      timeRun(&waitBeforeTilt);
    } else if (oldTilt == TILT_F_OPEN && desiredTilt == TILT_F_CLOSED) {
      tiltRun.tTimeout = TILT_FULL_MOVE;
      tiltDirection = true;
      timeRun(&waitBeforeTilt);
    } else if (oldTilt == TILT_F_OPEN && desiredTilt == TILT_H_CLOSED) {
      tiltRun.tTimeout = TILT_HALF_MOVE;
      tiltDirection = true;
      timeRun(&waitBeforeTilt);
    } else if (oldTilt == TILT_F_OPEN && desiredTilt == TILT_F_OPEN) {
      /* nothing */
    }
  }
}

void Shade::setTiltFromUp() {
  if (desiredTilt == TILT_F_OPEN) {        /* move 1000ms down */
    tiltRun.tTimeout = TILT_FULL_MOVE;
    tiltDirection = false;
    timeRun(&waitBeforeTilt);
  } else if (desiredTilt == TILT_H_CLOSED) { /* move 500ms down */
    tiltRun.tTimeout = TILT_HALF_MOVE;
    tiltDirection = false;
    timeRun(&waitBeforeTilt);
  } else if (desiredTilt == TILT_F_CLOSED) {   /* do nothing */
    
  }
}

void Shade::setTiltFromDown() {
  if (desiredTilt == TILT_F_OPEN) {        /* do nothing */
    
  } else if (desiredTilt == TILT_H_CLOSED) { /* move 500ms up */
    tiltRun.tTimeout = TILT_HALF_MOVE;
    tiltDirection = true;
    timeRun(&waitBeforeTilt);
  } else if (desiredTilt == TILT_F_CLOSED) {   /* move 1000ms up */
    tiltRun.tTimeout = TILT_FULL_MOVE;
    tiltDirection = true;
    timeRun(&waitBeforeTilt);
  }
}
