#include "RS485.h"

static RS485Class::t RS485Class::t_interval = {0, RS485_INTERVAL}; /* for every second on the DHCP checking */
static RS485Class::t RS485Class::sec_interval = {0, 1000};
static Modbus RS485Class::ControllinoModbusMaster(RS485_MASTER_MODBUS_ADD, RS485_SERIAL, 0);
static modbus_t RS485Class::query;
static byte RS485Class::status;
static uint16_t RS485Class::modbusSlaveRegisters[8];
static float RS485Class::temp[RS485_MAX_DEVICES];
static float RS485Class::humidity[RS485_MAX_DEVICES];
static bool RS485Class::tempRead[RS485_MAX_DEVICES];
static bool RS485Class::humidityRead[RS485_MAX_DEVICES];
static byte RS485Class::devState[RS485_MAX_DEVICES];

static byte RS485Class::begin() {

  query.u8id = 1; // slave address
  query.u8fct = 4; // function code (this one is registers read)
  query.u16RegAdd = 2; // start address in slave ( 1 - to read temp, 2 - to read humidity )
  query.u16CoilsNo = 0x0001; // number of elements (coils or registers) to read (alwyas needs to be 1)
  query.au16reg = modbusSlaveRegisters; // pointer to a memory array in the CONTROLLINO

  ControllinoModbusMaster.begin(9600); // baud-rate at 19200
  ControllinoModbusMaster.setTimeOut( 5000 ); // if there is no answer in 5000 ms, roll over

  status = RS485_QUERY_IDLE;

  temp[0] = 0;
  tempRead[0] = true;
  humidity[0] = 0;
  humidityRead[0] = true;
}

static byte RS485Class::update() {
  if (timeCheck(&t_interval)) {
    /* CODE EXECUTED EVERY RS485_INTERVAL - START */
    status = RS485_QUERY_READY;
      
    /* CODE EXECUTED EVERY RS485_INTERVAL - END */
    timeRun(&t_interval);
  }
  if (timeCheck(&sec_interval)) {
    /* CODE EXECUTED EVERY SECOND - START */
    if (status == RS485_QUERY_READY) {
      Serial.println(F("Sending temp query"));
      query.u16RegAdd = 1; /* 1 - temp */
      ControllinoModbusMaster.query(query); // send query (only once)
      status = RS485_QUERY_TEMP_WAITING;
    } else if (status == RS485_QUERY_TEMP_WAITING) {
      uint8_t pollResult;
      pollResult = ControllinoModbusMaster.poll(); // check incoming messages if return 0 - no message. > 0 message waiting -> e
      if (ControllinoModbusMaster.getState() == COM_IDLE) {
        Serial.print(F("Temp value: "));
        Serial.println(modbusSlaveRegisters[0], DEC);
        temp[0] = modbusSlaveRegisters[0] / 10.0;
        tempRead[0] = false;
        status = RS485_QUERY_TEMP_RECEIVED;
      }
    } else if (status == RS485_QUERY_TEMP_RECEIVED) {
      Serial.println(F("Sending humidity query"));
      query.u16RegAdd = 2; /* 2 - humidity */
      ControllinoModbusMaster.query(query); // send query (only once)
      status = RS485_QUERY_HUMIDITY_WAITING;
    } else if (status == RS485_QUERY_HUMIDITY_WAITING) {
      uint8_t pollResult;
      pollResult = ControllinoModbusMaster.poll(); // check incoming messages if return 0 - no message. > 0 message waiting -> e
      if (ControllinoModbusMaster.getState() == COM_IDLE) {
        Serial.print(F("Humidity value: "));
        Serial.println(modbusSlaveRegisters[0], DEC);
        humidity[0] = modbusSlaveRegisters[0] / 10.0;
        humidityRead[0] = false;
        status = RS485_QUERY_IDLE;
      }
    } else {
      /* nothing to be done here */
    }
      
    /* CODE EXECUTED EVERY SECOND - END */
    timeRun(&sec_interval);
  }
}

static bool RS485Class::timeCheck(struct t *t) {
  if ((unsigned long)(millis() - t->tStart) > t->tTimeout) {
    return true;
  } else {
    return false;
  }
}

static void RS485Class::timeRun(struct t *t) {
  t->tStart = millis();
}

static float RS485Class::getTemperature(byte devID) {
  tempRead[0] = true;
  return temp[0];
}

static bool RS485Class::isTemperatureRead(byte devID) {
  return tempRead[0];
}

static float RS485Class::getHumidity(byte devID) {
  humidityRead[0] = true;
  return humidity[0];
}

static bool RS485Class::isHumidityRead(byte devID) {
  return humidityRead[0];
}
