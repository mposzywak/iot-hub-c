#include <Arduino.h>

#ifndef SHADE_H_
#define SHADE_H_

/* defines how many seconds does it take to move the entire shade from closed to open or vice-versa */
#define DEFAULT_RANGE 64

/* defines the number sections that the range is devided to. This means that by changing the range section the shade will report its position */
#define DEFAULT_PARTS 4

/* predefined possible tilt positions */
#define TILT_F_CLOSED  0
#define TILT_H_CLOSED  45
#define TILT_F_OPEN    90

#define TILT_FULL_MOVE 1100
#define TILT_HALF_MOVE 550

/*  */
#define DIRECTION_DOWN 0
#define DIRECTION_UP   1

/* default tilt range movement length in miliseconds */
#define DEFAULT_TILT_RANGE 1000

/* types of physical button press */
#define PHY_NO_PRESS                0
#define PHY_MOMENTARY_PRESS         1
#define PHY_PRESS_MORE_THAN_2SEC    2

/*
 * Class for handling of the Shades. Each object represents one shade composed of 4 pins (2 inputs for both directions and 2 outputs for both directions)
 */
class Shade {
  /* types definitions */
  typedef struct t  {
    unsigned long tStart;
    unsigned long tTimeout;
    bool executed;
  };
  
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

    /* this is the tilt value of the current shade. It is configurable. */
    byte tiltRange;

    /* variable indicating the desired tilt. This is used to control the shade tilt */
    byte desiredTilt;

    int tiltSections[3];

    /* variables holding states of input and output pins */
    byte outPinUpState;
    byte outPinDownState;
    byte inPinUpState;
    byte inPinDownState;

    /* variables holding information if the in Pins were pressed in previous iteration */
    bool inPinUpPressed;
    bool inPinDownPressed;

    /* contains true if the Shade has just been stopped. Upon reporting it is supposed to be set to false */
    bool justStoppedVar;
    bool justStartedUpVar;
    bool justStartedDownVar;
    bool justStoppedTiltVar;

    byte oldSec;

    byte sections[6];

    /* contains the flag if the last checkpoint value has been already reported */
    bool positionReported;

    /* contains the value of the last checkpoint Position */
    byte reachedPosition;

    /* contains the flag if the information about Shade being unsync has been already reported */
    bool unsyncReported;

    /* contains information into which direction should be enabled after the programmed delay on swapping direction
     *  true  - UP
     *  false - DOWN
     */
    bool swapDirection;

    /* contains information which way the tilt should be moved 
     * true  - UP
     * false - DOWN
     */
    bool tiltDirection;

    /* holds information if the current move is a tilt movement (so that other conditions can be disabled) */
    bool tiltMovement;

    void upToPosition(byte dp);
    void downToPosition(byte dp);

    /* holds time values for time gap on shade movement direction change */
    t dir_swap;

    t updateExec;

    /* holds time that the tilt move has to run */
    t tiltRun;

    /* holds time that the shade should wait after it stops moveing to make the tilting move */
    t waitBeforeTilt;

    /* holds timer for button pressing - for measurement for how long the physical button is held pressed */
    t upButtonHold;
    t downButtonHold;

    /* functions to control time based execution */
    bool Shade::timeCheck(struct t *t );
    void Shade::timeRun(struct t *t);

    /* this function is executed every time the shade stops and the movement adjust the tilt. 
    Depending whether the movement is up or down the respective function is called */
    void setTiltFromUp();
    void setTiltFromDown();

    
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
  byte isUpPressed();

  /* returns true if the "down" button is pressed */
  byte isDownPressed();

  /* function to be executed on every loop. Updates timers */
  byte update();

  /* starts shade up movement */
  void up();

  /* starts shade down movement */
  void down();

  /* stops the shade */
  void stop();

  /* movement stop function, but includes tilt movement after - to be executed if the specific stop order has been received
   *  either form physical button or ARiF.
   */
  void stopWithTilt();

  /* stops the shade after tilt movement */
  void tiltStop();

  /* returns true if the shade is currently moving in either direction */
  bool isMoving();
  bool isMovingUp();
  bool isMovingDown();

  /* returns true if the Shade has just stopped moveing and changes justStopped variable to false */
  bool justStopped();
  bool justStartedUp();
  bool justStartedDown();
  bool justStoppedTilt();

  /* sets the desired position of the shade to a given value */
  void toPosition(byte position);

  /* get current position */
  byte getCurrentPosition();

  /* return the tilt value */
  byte getTilt();

  /* get the current devID/shadeID of the object */
  byte getDevID();

  /* returns if the Shade is in synced state */
  bool isSynced();

  /* set the desired tilt of the shade */
  void setTilt(byte tilt);

  /* toggle tilt - move the tilt in up (make it more open). If fully open - move to fully close */
  void Shade::toggleTiltUp();

  /* toggle tilt - move the tilt in down (make it more close). If fully closed - move to fully open */
  void Shade::toggleTiltDown();

  
};

#endif
