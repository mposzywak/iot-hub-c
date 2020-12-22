#include <Arduino.h>

#ifndef LIGHT_H_
#define LIGHT_H_

/* types of digitOUT devices */
#define DIGITOUT_ONOFF   0     /* Upon activation it changes the state of the output pin from on to off or vice-versa */
#define DIGITOUT_TIMER   1     /* Upon activation it changes the state of the output pin to on for an amount of time, after that it goes to off */

/* other default values */

/* default timer for timer type lights (in seconds) */
#define DEFAULT_TIMER    17

class Light {
  /* types definitions */
  typedef struct t  {
    unsigned long tStart;
    unsigned long tTimeout;
    bool executed;
  };
  
  private:
    /* Device type */
    byte type;

    /* timer value for DIGITOUT_TIMER type */
    unsigned long timer;
    t onTimer;
    
    /* ID of the device */
    byte lightID;

    /* vars holds the number of the input and output pins */
    byte inPin;
    byte outPin;

    /* value used to track the input pin state */
    byte inPinState;

    /* value used to store the press-hold state of the pin */
    bool inPinPressed;

    /* value used to store the output state in a variable */
    byte outPinState;

    /* holds timer for button pressing - for measurement for how long the physical button is held pressed */
    t buttonHold;

    /* functions to control time based execution */
    bool Light::timeCheck(struct t *t );
    void Light::timeRun(struct t *t);

    /* holds state of device just being toggled */
    bool justToggled;


  public:

    static byte low;
    static byte high;

    /* Contructor takes lightID where the value cannot exceed the available in/out pins. */
    Light();
    
    /* Initialization function for the constructor */
    void init(byte shadeID, byte type);

    /* sets the light type */
    void Light::setType(byte type);

    /* returns the light type */
    byte Light::getType();

    /* sets the timer */
    void Light::setTimer(unsigned long timer);
    
    /* get devID value */
    byte Light::getDevID();

    /* function to check if the physical button was pressed - it can also detect a press-hold event */
    byte Light::isPressed();

    /* set the light to ON */
    void Light::setON();

    /* set the light to OFF */
    void Light::setOFF();

    /* toggle the light */
    void Light::toggle();

    /* returns true if the light has just been turned ON */
    bool Light::justTurnedON();

    /* returns true if the light has just been turned OFF */
    bool Light::justTurnedOFF();

    /* return true if the light is ON and false if it is OFF */
    bool Light::getStatus();

    /* reset the light device - this function supposed to be executed only when the mode is changed from lights to something else */
    void Light::reset();
};

#endif
