#include <Arduino.h>

#ifndef SHADE_H_
#define SHADE_H_

/* defines how many seconds does it take to move the entire shade from closed to open or vice-versa */
#define SHADE_DEFAULT_POSITION_TIMER 64

/* default tilt range movement length in miliseconds */
#define SHADE_DEFAULT_TILT_TIMER     1000

/* defines the number sections that the range is devided to. This means that by changing the range section the shade will report its position */
#define DEFAULT_PARTS 4

/* predefined possible tilt positions */
#define TILT_F_CLOSED  0
#define TILT_H_CLOSED  45
#define TILT_F_OPEN    90
#define TILT_NONE      255     /* only to be assigned to secDesiredTilt */

#define TILT_FULL_MOVE 1100
#define TILT_HALF_MOVE 550

/* direction of the move  */
#define DIRECTION_DOWN 0
#define DIRECTION_UP   1

/* timer value ranges */
#define SHADE_POSITION_TIMER_MIN  10
#define SHADE_POSITION_TIMER_MAX  255
#define SHADE_TILT_TIMER_MIN      100
#define SHADE_TILT_TIMER_MAX      30000

/* wait time between direction change (in ms) */
#define DIRECTION_SWITCH_WAIT_TIME 300

/* amount of time a button needs to be held in order to enable action on press-hold event */
#define BUTTON_HOLD_TIME 700

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

    /* variable holding the current position of the shade (in 100s of ms) */
    int position;

    /* variable indicating the desired position. This is used to control the movement of the shade */
    int desiredPosition;

    /* variable holding information if the shade is synced (i. e. if the system knows in which position it is in) */
    bool synced;

    /* the amount of time (in seconds) it takes for the shade to fully open from fully closed state (or vice-versa) */
    int movementRange;

    /* this is the tilt value of the current shade. It is configurable. */
    int tiltRange;

    /* variable indicating the desired tilt. This is used to control the shade tilt */
    byte desiredTilt;

    /* variable used to hold the desired Tilt value that is received during tilt move (so that desiredTilt is not overwritten) */
    byte secDesiredTilt;

    int tiltSections[3];

    /* indication if the recent action on the shade was triggered by a physical switch */
    bool userPressed;

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

    /* contains the flag if the information about Shade being unsync has been already reported */
    bool justSyncedVar;

    byte oldSec;

    int sections[6];

    /* contains the flag if the last checkpoint value has been already reported */
    bool positionReported;

    /* contains the value of the last checkpoint Position */
    int reachedPosition;

    /* the secondary position value. Used to store temporary desired position value before movement can be started */
    int secPosition;

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

    void upToPosition(int dp);
    void downToPosition(int dp);

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

    /* indicator variable to hold info if during tilt movement a position change request has been received */
    bool positionAfterTilt;
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
  void init(byte shadeID, bool sync, int tilt, int pos, byte reachedPos, int positionTimer, int tiltTimer);

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

  bool justSynced();

  /* sets the desired position of the shade to a given value */
  void toPosition(byte position);

  /* get current position (in percentage) */
  byte getCurrentPosition();

  /* get current position (in 100s of miliseconds) */
  int Shade::getPosition();

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

  /* reset the light device - this function supposed to be executed only when the mode is changed from shades to something else */
  void Shade::reset();

  /* set/get shade Position timer - setting the position timer will also reset the shade i. e. place it into the unsync state
     timer value is in seconds */
  void Shade::setPositionTimer(int timer);
  int Shade::getPositionTimer();

  /* set/get shade Tilt timer - setting the tilt timer will also reset the shade i. e. place it into the unsync state  
     timer value is in miliseconds */
  void Shade::setTiltTimer(int timer);
  int Shade::getTiltTimer();

  /* control the userPressed state */
  void Shade::setUserPressed();
  void Shade::clearUserPressed();
  bool Shade::getUserPressed();

  /* validation of Position Timer, if argument is not within range, returns default timer value */
  static byte Shade::validatePositionTimer(byte timer);

  /* validation of Tilt Timer, if argument is not within range, returns default timer value */
  static int Shade::validateTiltTimer(int timer);
};

#endif
