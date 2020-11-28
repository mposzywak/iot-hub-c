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
  Settings::setInPinMode(inPinUp);
  Settings::setInPinMode(inPinDown);
  inPinUpState    = LOW;
  inPinDownState  = LOW;
  inPinUpPressed    = false;
  inPinDownPressed  = false;

  pinMode(outPinUp, OUTPUT);
  pinMode(outPinDown, OUTPUT);

  Shade::low = Settings::getLow();
  Shade::high = Settings::getHigh();

  Settings::setOutputPinValue(outPinUp, Shade::low);
  Settings::setOutputPinValue(outPinDown, Shade::low);
  outPinUpState = Shade::low;
  outPinDownState = Shade::low;

  position = 0;
  desiredPosition = 0;
  movementRange = DEFAULT_RANGE;
  synced = false;
  justStoppedVar = false;
  justStoppedTiltVar = false;

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

  dir_swap = { 0, DIRECTION_SWITCH_WAIT_TIME, true }; /* The 300ms timer to wait before changing direction up/down */
  updateExec = { 0, 1000, false }; /* The timer to control the update() frequency */
  tiltRun = { 0, 500, true };
  waitBeforeTilt = { 0, 500, true };

  upButtonHold = { 0, BUTTON_HOLD_TIME, true };
  downButtonHold = { 0, BUTTON_HOLD_TIME, true };

  desiredTilt = TILT_H_CLOSED;
  tiltMovement = false;

  secDesiredTilt = TILT_NONE;
  Serial.println(secDesiredTilt);
}

byte Shade::isUpPressed() {
  inPinUpState = Settings::getInputPinValue(inPinUp);
  if (inPinUpState == HIGH) { /* Button pressed and held */
    if (!inPinUpPressed) { /* at the moment of pressing start counting time */
      timeRun(&upButtonHold);
    }
    inPinUpPressed = true;
    delay(10); // this delay here was placed in order for the press button result to be predictable
    return PHY_NO_PRESS;
  } else {
    if (inPinUpPressed) { /* Button is released */
      /* EXECUTED ON BUTTON RELEASE - START */

      /* EXECUTED ON BUTTON RELEASE - END */
      if (timeCheck(&upButtonHold)) {
        inPinUpPressed = false;
        return PHY_PRESS_MORE_THAN_2SEC;
      }
      inPinUpPressed = false;
      return PHY_MOMENTARY_PRESS;
    } else {
      return PHY_NO_PRESS;
    }
  }
}

byte Shade::isDownPressed() {
  inPinDownState = Settings::getInputPinValue(inPinDown);
  if (inPinDownState == HIGH) { /* Button pressed and held */
    if (!inPinDownPressed) { /* at the moment of pressing start counting time */
      timeRun(&downButtonHold);
    }
    inPinDownPressed = true;
    delay(10); // this delay here was placed in order for the press button result to be predictable
    return PHY_NO_PRESS;
  } else {
    if (inPinDownPressed) { /* Button is released */
      /* EXECUTED ON BUTTON RELEASE - START */

      /* EXECUTED ON BUTTON RELEASE - END */
      if (timeCheck(&downButtonHold)) {
        inPinDownPressed = false;
        return PHY_PRESS_MORE_THAN_2SEC;
      }
      inPinDownPressed = false;
      return PHY_MOMENTARY_PRESS;
    } else {
      return PHY_NO_PRESS;
    }
  }
}

byte Shade::update() {

  /* code executed on direction switch */
  if (timeCheck(&dir_swap)) {
    Serial.println("Executing delayed change *");
    Serial.println(millis());
    if (swapDirection == true) {
      //digitalWrite(outPinUp, Shade::high);
      Settings::setOutputPinValue(outPinUp, Shade::high);
      outPinUpState = Shade::high;
    } else { /* swapDirection == false */
      //digitalWrite(outPinDown, Shade::high);
      Settings::setOutputPinValue(outPinDown, Shade::high);
      outPinDownState = Shade::high;
    }
  }

  if (timeCheck(&waitBeforeTilt)) {
    /* waited time to start the tilt movement */
    tiltMovement = true;
    Serial.println("Starting tilt movement");
    if (tiltDirection == true) {
      //digitalWrite(outPinUp, Shade::high);
      Settings::setOutputPinValue(outPinUp, Shade::high);
      //outPinUpState = Shade::high;
    } else { /* tiltDirection == false */
      //digitalWrite(outPinDown, Shade::high);
      Settings::setOutputPinValue(outPinDown, Shade::high);
      //outPinDownState = Shade::high;
    }
    timeRun(&tiltRun);
  }

  if (timeCheck(&tiltRun)) {
    Serial.println("Stopping tilt movement");
    tiltMovement = false;
    this->tiltStop();
  }
  
  if (timeCheck(&updateExec)) { 
    /* no combining with any other condition! If timeCheck executes but there is no timeRun(), 
       this code will stop executing.  */
    /* CODE EXECUTED EVERY SECOND - START */    
    bool movingUp = this->isMovingUp();
    bool movingDown = this->isMovingDown();
 
    if (!synced) { /* not synced */
      if (movingUp && position > 0) {
        position--;
        positionReported = false;
      } else if (movingUp && position == 0) {
        synced = true;
        this->stop();
        tiltStop(); /* this is just to report the state of the tilt if up move is used to sync the shade */
        //Serial.println("Shade synced");
      }
      if (movingDown && position < movementRange) {
        position++;
        positionReported = false;
      } else if (movingDown && position == movementRange) {
        synced = true;
        this->stop();
        setTiltFromDown(); /* this is to report the state of the shade once synced */
        //Serial.println("Shade synced");
      } 
    } else { /* synced */
      if (position > desiredPosition) { /* need to move up */
        if (!movingUp)
          upToPosition(desiredPosition); /* in this case the argument doesn't change anything as the desiredPosition has already been set */
        position--;
        positionReported = false;
      } else if (movingUp && position == desiredPosition) {
        //Serial.println("Reached pos by moving Up");
        this->stop();
        if (position != 0) /* this condition is added in order to prevent tilt movement when the shade is fully open */
          setTiltFromUp();
        else
          tiltStop();
      }
      if (position < desiredPosition) { /* need to move down */
        if (!movingDown)
          downToPosition(desiredPosition); /* in this case the argument doesn't change anything as the desiredPosition has already been set */
        position++;
        positionReported = false;

      } else if (movingDown && position == desiredPosition) {
        this->stop();
        setTiltFromDown();
      }
      
    }
    timeRun(&updateExec);
    if (position == sections[0] && positionReported == false && synced) {
      reachedPosition = 0;
      return 0;
    } else if (position == sections[1] && positionReported == false && synced) {
      reachedPosition = 25;
      return 25;
    } else if (position == sections[2] && positionReported == false && synced) {
      reachedPosition = 50;
      return 50;
    } else if (position == sections[3] && positionReported == false && synced) {
      reachedPosition = 75;
      return 75; 
    } else if (position == sections[4] && positionReported == false && synced) {
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
  //digitalWrite(outPinDown, Shade::low);
  Settings::setOutputPinValue(outPinDown, Shade::low);
  if (outPinDownState == Shade::high) { /* on condition shade was moving in the opposite direction */
    timeRun(&dir_swap);
    swapDirection = true;
  } else { /* on condition shade was already moving in the desired direction or stopped */
    //digitalWrite(outPinUp, Shade::high);
    Settings::setOutputPinValue(outPinUp, Shade::high);
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
  Settings::setOutputPinValue(outPinUp, Shade::low);
  if (outPinUpState == Shade::high) { /* on condition shade was moving in the opposite direction */
    timeRun(&dir_swap);
    swapDirection = false;
  } else { /* on condition shade was already moving in the desired direction or stopped */
    //digitalWrite(outPinDown, Shade::high);
    Settings::setOutputPinValue(outPinDown, Shade::high);
    outPinDownState = Shade::high;
  }
  outPinUpState = Shade::low;
  if (!synced) {
    position = 0;
  } else {
    desiredPosition = dp;
  }
  justStartedDownVar = true;
  Serial.println("Shade downToPosition()");
}

void Shade::stop() {
  Settings::setOutputPinValue(outPinUp, Shade::low);
  Settings::setOutputPinValue(outPinDown, Shade::low);
  outPinUpState = Shade::low;
  outPinDownState = Shade::low;
  justStoppedVar = true;
  tiltMovement = false;
  desiredPosition = position;
}

void Shade::stopWithTilt() {
  bool upMove = false;
  bool downMove = false;
  if (isMovingUp())
    upMove = true;
  if (isMovingDown())
    downMove = true;
  stop();
  if (upMove)
    setTiltFromUp();
  if (downMove)
    setTiltFromDown();
    
}

void Shade::tiltStop() {
  Settings::setOutputPinValue(outPinUp, Shade::low);
  Settings::setOutputPinValue(outPinDown, Shade::low);
  outPinUpState = Shade::low;
  outPinDownState = Shade::low;
  justStoppedTiltVar = true;
  tiltMovement = false;
  desiredPosition = position;
  /* handle situation where the setTilt() functiotn has been executed while the tilt was moving 
     and hence we need to do one more tilt move. The new tilt value has been saved in secDesiredTilt */
  Serial.print("secDesiredTilt: ");
  Serial.println(secDesiredTilt);
  if (secDesiredTilt == TILT_F_CLOSED || secDesiredTilt == TILT_H_CLOSED || secDesiredTilt == TILT_F_OPEN ) {
    Serial.println("Another tilt move to make");
    setTilt(secDesiredTilt);
    secDesiredTilt = TILT_NONE;
  }
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

bool Shade::justStoppedTilt() {
  if (justStoppedTiltVar) {
    justStoppedTiltVar = false;
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

  if (tiltMovement == true) { /* condition for executing while tilt moving */
    Serial.println("======= tiltMoving");
    secDesiredTilt = tilt;
    return;
  } else {
    oldTilt = desiredTilt;
    desiredTilt = tilt;
  }

  Serial.print("old Tilt: ");
  Serial.println(oldTilt);
  Serial.print("desired Tilt: ");
  Serial.println(desiredTilt);
  
  
  if (position == 0 && (oldTilt != desiredTilt) && !this->isMoving()) { /* for condition when shade is at the top - don't need tilt move */
    this->tiltStop();
    return;
  } 

  if (!this->isMoving() && (oldTilt != desiredTilt)) {
    Serial.println("Shade not moving. Triggering tilt change");
    if (oldTilt == TILT_F_CLOSED && desiredTilt == TILT_F_CLOSED) {
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

void Shade::toggleTiltUp() {
  if (desiredTilt == TILT_F_OPEN) {
    setTilt(TILT_H_CLOSED);
  } else if (desiredTilt == TILT_H_CLOSED) {
    setTilt(TILT_F_CLOSED);
  } else if (desiredTilt == TILT_F_CLOSED) {
    setTilt(TILT_F_OPEN);
  }
}

void Shade::toggleTiltDown() {
  if (desiredTilt == TILT_F_OPEN) {
    setTilt(TILT_F_CLOSED);
  } else if (desiredTilt == TILT_F_CLOSED) {
    setTilt(TILT_H_CLOSED);
  } else if (desiredTilt == TILT_H_CLOSED) {
    setTilt(TILT_F_OPEN);
  } 
}

void Shade::reset() {
  Settings::setOutputPinValue(outPinUp, Shade::low);
  Settings::setOutputPinValue(outPinDown, Shade::low);
  outPinUpState = Shade::low;
  outPinDownState = Shade::low;
  synced = false;
  justStoppedVar = false;
  justStoppedTiltVar = false;
}
