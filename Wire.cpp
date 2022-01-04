#include "Wire.h"


static WireClass::t WireClass::t_interval = {0, WIRE_INTERVAL}; /* for every second on the DHCP checking */
static OneWire WireClass::oneWire(Settings::getOneWirePin());
static DallasTemperature WireClass::sensors(&oneWire);
static float WireClass::temp1 = 0;
static float WireClass::temp2 = 0;
static bool WireClass::tempRead1 = false;
static bool WireClass::tempRead2 = false;
static byte WireClass::deviceCount = 0;
static float WireClass::temp[WIRE_MAX_DEVICES];
static bool WireClass::tempRead[WIRE_MAX_DEVICES];


static byte WireClass::begin() {
  Serial.println(F("--- 1-wire start ---\n"));
  byte i;
  byte present = 0;
  byte data[12];
  byte addr[8];
  Serial.print(F("1-Wire Pin set to: "));
  Serial.println(Settings::getOneWirePin());
  sensors.begin();
  while(oneWire.search(addr)) {
    Serial.print(F("Found \'1-Wire\' device with address: "));
    for(byte i = 0; i < 8; i++) {
      Serial.print("0x");
      if (addr[i] < 16) {
        Serial.print('0');
      }
      Serial.print(addr[i], HEX);
      if (i < 7) {
        Serial.print(", ");
      }
    }
    if ( OneWire::crc8( addr, 7) != addr[7]) {
        Serial.print(F("CRC is not valid!\n"));
        return;
    }
    Serial.println();
  }

  deviceCount = sensors.getDeviceCount();
  
  Serial.print(F("No of devices found: "));
  Serial.println(deviceCount);
  Serial.print(F("\n--- 1-wire end ---\n"));
  return 0;
}

static byte WireClass::update() {
  if (timeCheck(&t_interval)) {
    /* CODE EXECUTED EVERY SECOND - START */

    sensors.requestTemperatures();  
    for (byte i = 0 ; i < deviceCount; i++) {
      temp[i] = sensors.getTempCByIndex(i);
      tempRead[i] = false;
      Serial.print(F("Temperature sensor "));
      Serial.print(i);
      Serial.print(F(" reports: "));
      Serial.println(temp[i]);
    }
      
    /* CODE EXECUTED EVERY SECOND - END */
    timeRun(&t_interval);
  }
}

bool WireClass::timeCheck(struct t *t) {
  if ((unsigned long)(millis() - t->tStart) > t->tTimeout) {
    return true;
  } else {
    return false;
  }
}

void WireClass::timeRun(struct t *t) {
  t->tStart = millis();
}

static float WireClass::getTemperature(byte devID) {
  switch (devID) {
    case 40:
     tempRead[0] = true;
     return temp[0];
    case 41:
     tempRead[1] = true;
     return temp[1];
    case 42:
     tempRead[2] = true;
     return temp[2];
    case 43:
     tempRead[3] = true;
     return temp[3];
    case 44:
     tempRead[4] = true;
     return temp[4];
    case 45:
     tempRead[5] = true;
     return temp[5];
    case 46:
     tempRead[6] = true;
     return temp[6];
    case 47:
     tempRead[7] = true;
     return temp[7];
    default:
     return 0;
  }
}

static bool WireClass::isTemperatureRead(byte devID) {
  switch (devID) {
    case 40:
     return tempRead[0];
    case 41:
     return tempRead[1];
    case 42:
     return tempRead[2];
    case 43:
     return tempRead[3];
    case 44:
     return tempRead[4];
    case 45:
     return tempRead[5];
    case 46:
     return tempRead[6];
    case 47:
     return tempRead[7];
    default:
     return true;
  }
}

static byte WireClass::getDeviceCount() {
  return deviceCount;
}
