#include "ModbusRtu.h"

#ifndef RS485_H_
#define RS485_H_

#define RS485_INTERVAL        30000  /* probing interval */
#define RS485_MAX_DEVICES     2      /* maximum amount of devices that can be connected to the RS485 port */
#define RS485_MODBUS_TIMEOUT  3000   /* MODBUS timeout */
#define RS485_MASTER_MODBUS_ADD 0
#define RS485_SERIAL            3

#define RS485_QUERY_IDLE                 0  /* nothing to be done */
#define RS485_QUERY_READY                1  /* status being set to RS485_QUERY_READY every RS485_INTERVAL, when RS485_QUERY_READY, send temp query and change to RS485_QUERY_TEMP_WAITING */
#define RS485_QUERY_TEMP_WAITING         2  /* When RS485_QUERY_TEMP_WAITING poll for query output, once received change state to: RS485_QUERY_TEMP_RECEIVED */
#define RS485_QUERY_TEMP_RECEIVED        3  /* When RS485_QUERY_TEMP_RECEIVED query humidity and set status to RS485_QUERY_HUMIDITY_WAITING */
#define RS485_QUERY_HUMIDITY_WAITING     4  /* When RS485_QUERY_HUMIDITY_WAITING poll for query output, once received change state to RS485_QUERY_IDLE */

class RS485Class {

  /* time structure */
  typedef struct t  {
    unsigned long tStart;
    unsigned long tTimeout;
  };
  
  private:

    static Modbus ControllinoModbusMaster;

    static float temp[RS485_MAX_DEVICES];
    static float humidity[RS485_MAX_DEVICES];


    static bool tempRead[RS485_MAX_DEVICES];
    static bool humidityRead[RS485_MAX_DEVICES];

    static byte devState[RS485_MAX_DEVICES];

    /* interval variable controlling how often is the bus queried for values */
    static t t_interval;

    /* one second interval to handle query related tasks */
    static t sec_interval;

    static modbus_t query;

    static uint16_t modbusSlaveRegisters[8];

    /* status of the query */
    static byte status;

  public:

  /* Class constructor */
  static byte RS485Class::begin();

  /* processing function for the main loop */
  static byte RS485Class::update();

  /* check if it's the time to execute */
  static bool RS485Class::timeCheck(struct t *t);

  /* reset time */
  static void RS485Class::timeRun(struct t *t);

  /* get the latest Temperature value */
  static float RS485Class::getTemperature(byte devID);

  /* returns if the latest value recorded have already been read */
  static bool RS485Class::isTemperatureRead(byte devID);

  /* get the latest Temperature value */
  static float RS485Class::getHumidity(byte devID);

  /* returns if the latest value recorded have already been read */
  static bool RS485Class::isHumidityRead(byte devID);

};

extern RS485Class RS485;

#endif
