#include <Arduino.h>

#ifndef SHADE_H_
#define SHADE_H_

#define DEFAULT_RANGE 16

/*
 * Class for handling of the Shades. Each object represents one shade composed of 4 pins (2 inputs for both directions and 2 outputs for both directions)
 */
class Shade {
  private:
    byte shadeID;

    /* pin numbers that are related to defined hardware */
    byte outPinUp;
    byte outPinDown;
    byte inPinUp;
    byte inPinDown;

    /* variable holding the current position of the shade (in seconds) */
    byte position;

    /* variable indicating the desired position. This is used to control the movement of the shade */
    byte desiredPosition;

    /* variable holding information if the shade is synced (i. e. if the system knows in which position it is in) */
    bool synced;

    /* the amount of time (in seconds) it takes for the shade to fully open from fully closed state (or vice-versa) */
    byte movementRange;

    /* variables holding states of input and output pins */
    byte outPinUpState;
    byte outPinDownState;
    byte inPinUpState;
    byte inPinDownState;

    /* variables holding information if the in Pins were pressed in previous iteration */
    bool inPinUpPressed;
    bool inPinDownPressed;

    byte oldSec;

  public:

    static byte low;
    static byte high;
  
  /*
   * Contructor takes shadesID where the value cannot exceed the available in/out pins devided by 2.
   */
  Shade();
  /*
   * Initialization function for the constructor
   */
  void init(byte shadeID);

  /* returns true if the "up" button is pressed */
  bool isUpPressed();

  /* returns true if the "down" button is pressed */
  bool isDownPressed();

  /* function to be executed on every loop. Updates timers */
  void update();

  /* starts shade up movement */
  void up();

  /* starts shade down movement */
  void down();

  /* stops the shade */
  void stop();

  /* returns true if the shade is currently moving in either direction */
  bool isMoving();

  bool isMovingUp();

  bool isMovingDown();
  
};

#endif
