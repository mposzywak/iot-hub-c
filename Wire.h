#include <OneWire.h>
#include <DallasTemperature.h>
#include "Settings.h"


#ifndef WIRE_H_
#define WIRE_H_

#define WIRE_PIN         2  /* pin number of the OneWire bus */
#define WIRE_INTERVAL    5000  /* probing interval */
#define WIRE_MAX_DEVICES 8

class WireClass {

  /* time structure */
  typedef struct t  {
    unsigned long tStart;
    unsigned long tTimeout;
  };
  
  private:

    static float temp[WIRE_MAX_DEVICES];
    static float temp1;
    static float temp2;

    static bool tempRead[WIRE_MAX_DEVICES];
    static bool tempRead1;
    static bool tempRead2;

    /* interval variable controlling how often is the bus queried for values */
    static t t_interval;

    /* the sensors objects */
    static OneWire oneWire;
    static DallasTemperature sensors;

    /* number of detected devices */
    static byte deviceCount;
  public:

  /* Class constructor */
  static byte WireClass::begin();

  /* processing function for the main loop */
  static byte WireClass::update();

  /* check if it's the time to execute */
  static bool WireClass::timeCheck(struct t *t);

  /* reset time */
  static void WireClass::timeRun(struct t *t);

  /* get the latest Temperature value */
  static float WireClass::getTemperature(byte devID);

  /* returns if the latest value recorded have already been read */
  static bool WireClass::isTemperatureRead(byte devID);

  /* returns the amount of the devices detected */
  static byte WireClass::getDeviceCount();
};

extern WireClass Wire;

#endif
