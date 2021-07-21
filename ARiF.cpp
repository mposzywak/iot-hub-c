#include "ARiF.h"

static byte ARiFClass::version;
static byte ARiFClass::ardID = 0;
static byte ARiFClass::raspyID;
static byte ARiFClass::mac[] = { 0x00, 0xAA, 0xBB, 0xC6, 0xA5, 0x58 };
static IPAddress ARiFClass::raspyIP;
static bool ARiFClass::DHCPFailed = false;
static byte ARiFClass::lastHeartbeat = 0;
static bool ARiFClass::beaconSent = false;
static EthernetUDP ARiFClass::UDP;
static IPAddress ARiFClass::mip(224, 1, 1, 1);
static bool ARiFClass::isConnected = false;
static EthernetServer ARiFClass::ARiFServer(ARiF_HTTP_PORT);
static EthernetClient ARiFClass::ARiFClient;
static byte ARiFClass::lastDevID = 0;
static bool ARiFClass::isRegistered;
static bool ARiFClass::signalIPchange = false;
static ARiFClass::t ARiFClass::t_func1 = {0, 1000 * ARiF_BEACON_INT}; /* for ARiF beacon interval */
static ARiFClass::t ARiFClass::t_func2 = {0, 1000}; /* for everysecond on the DHCP checking */
static byte ARiFClass::lastShadePosition = 0;
static byte ARiFClass::lastShadeTilt = 0;
static int ARiFClass::lastShadePositionTimer = 0;
static int ARiFClass::lastShadeTiltTimer = 0;
static byte ARiFClass::mode = 0;
static byte ARiFClass::lastLightType = 0;
static unsigned long ARiFClass::lastLightTimer = 0;


static byte ARiFClass::begin(byte version, byte mac[]) {
  ARiFClass::version = version;
  //ARiFClass::mac = mac;
  /* initialize Ethernet */
  beginEthernet(mac);
  isRegistered = false;
  return 0;
}

static byte ARiFClass::begin(byte version, byte mac[], IPAddress raspyIP, byte ardID, byte raspyID) {
  ARiFClass::version = version;
  //ARiFClass::mac = mac;
  ARiFClass::raspyIP = raspyIP;
  ARiFClass::ardID = ardID;
  ARiFClass::raspyID = raspyID;
  beginEthernet(mac);
  isRegistered = true;
  return 0;
}

static byte ARiFClass::update() {
  char beacon[ARiF_BEACON_LENGTH];
  if (lastHeartbeat > 4) {
    if (timeCheck(&t_func1)) {
      if (beaconSent == false) {
        snprintf(beacon, ARiF_BEACON_LENGTH, ARiF_BEACON_STRING, ardID);
        Serial.print("Sending beacon: ");
        Serial.println(beacon);
        UDP.beginPacket(mip, ARiF_BEACON_PORT);
        UDP.write(beacon);
        UDP.endPacket();
        beaconSent = true;
        isConnected = false; // Update the status of the iot-hub <-> iot-gw connection
      }
      timeRun(&t_func1);
    } else {
      beaconSent = false;
    }
  }
  byte DHCPResult;
  if (timeCheck(&t_func2)) {
    /* CODE EXECUTED EVERY SECOND - START */
    lastHeartbeat++;
    if (!DHCPFailed) {
      DHCPResult = Ethernet.maintain(); // call this func once per sec for DHCP lease renewal
      if (DHCPResult == 1 || DHCPResult == 3)
        DHCPFailed = true;
    }
    /* CODE EXECUTED EVERY SECOND - END */
    timeRun(&t_func2);
  }

  EthernetClient client = ARiFServer.available();
  if (client) {
    Serial.print("HTTP Request received from: ");
    Serial.println(client.remoteIP());
    char buff[ARiF_HTTP_BUFF];
    int index = 0;
    byte devID;
    while (client.available()) {
      char c = client.read();
      //Serial.print(c);
      buff[index] = c;
      index++;
      if (c == '/n' or c == '/r' or index >= ARiF_HTTP_BUFF) break;
    }

    switch (getValue(buff, CMD)) {
      case CMD_HEARTBEAT:
        if (isRegistered) {
          client.println(F(HTTP_200_OK)); /* write 200 OK */
          client.stop();                  /* send */
          lastHeartbeat = 0;              /* reset the heartbeat timer */
          if (isConnected == 0) {
            isConnected = true;
            return U_CONNECTED;
          }
        } else {
          client.println(F(HTTP_403_Error)); /* write 403 because the unexpected HB */
          client.stop();                  /* send */
          Serial.println("Unexpected heartbeat received. Sending 403.");
        }
        break;
      case CMD_REGISTER:
        Serial.println("register received");
        //if (!isRegistered) { // this is commented out to allow the arduino to be "re-registered"
        ardID = getValue(buff, ARDID);
        raspyID = getValue(buff, RASPYID);
        raspyIP = client.remoteIP();
        isRegistered = true;
        client.println(F(HTTP_200_OK));
        client.println();
        client.stop();
        return CMD_REGISTER;
        //}
        break;
      case CMD_SHADEUP:
        if (mode == M_SHADES) {
          lastDevID = getValue(buff, DEVID);
          client.println(F(HTTP_200_OK));
          client.println();
          client.stop();
          return CMD_SHADEUP;
        } else {
          client.println(F(HTTP_403_Error));
          client.println();
          client.stop();
          return U_NOTHING;
        }
        break;
      case CMD_SHADEDOWN:
        if (mode == M_SHADES) {
          lastDevID = getValue(buff, DEVID);
          client.println(F(HTTP_200_OK));
          client.println();
          client.stop();
          return CMD_SHADEDOWN;
        } else {
          client.println(F(HTTP_403_Error));
          client.println();
          client.stop();
          return U_NOTHING;
        }
        break;
      case CMD_LIGHTON:
        if (mode == M_LIGHTS) {
          lastDevID = getValue(buff, DEVID);
          client.println(F(HTTP_200_OK));
          client.println();
          client.stop();
          return CMD_LIGHTON;
        } else {
          client.println(F(HTTP_403_Error));
          client.println();
          client.stop();
          return U_NOTHING;
        }
        break;
      case CMD_LIGHTOFF:
        if (mode == M_LIGHTS) {
          lastDevID = getValue(buff, DEVID);
          client.println(F(HTTP_200_OK));
          client.println();
          client.stop();
          return CMD_LIGHTOFF;
        } else {
          client.println(F(HTTP_403_Error));
          client.println();
          client.stop();
          return U_NOTHING;
        }
        break;
      case CMD_LIGHT_TYPE:
        if (mode == M_LIGHTS) {
          lastDevID = getValue(buff, DEVID);
          lastLightType = getValue(buff, VALUE);
          client.println(F(HTTP_200_OK));
          client.println();
          client.stop();
          return CMD_LIGHT_TYPE;
        } else {
          client.println(F(HTTP_403_Error));
          client.println();
          client.stop();
          return U_NOTHING;
        }
        break;
      case CMD_LIGHT_TIMER:
        if (mode == M_LIGHTS) {
          lastDevID = getValue(buff, DEVID);
          lastLightTimer = getValue(buff, VALUE_L);
          client.println(F(HTTP_200_OK));
          client.println();
          client.stop();
          return CMD_LIGHT_TIMER;
        } else {
          client.println(F(HTTP_403_Error));
          client.println();
          client.stop();
          return U_NOTHING;
        }
        break;
      case CMD_SHADEPOS:
        if (mode == M_SHADES) {
          lastDevID = getValue(buff, DEVID);
          lastShadePosition = getValue(buff, VALUE);
          client.println(F(HTTP_200_OK));
          client.println();
          client.stop();
          return CMD_SHADEPOS;
        } else {
          client.println(F(HTTP_403_Error));
          client.println();
          client.stop();
          return U_NOTHING;
        }
        break;
      case CMD_SHADETILT:
        if (mode == M_SHADES) {
          lastDevID = getValue(buff, DEVID);
          lastShadeTilt = getValue(buff, VALUE);
          client.println(F(HTTP_200_OK));
          client.println();
          client.stop();
          return CMD_SHADETILT;
        } else {
          client.println(F(HTTP_403_Error));
          client.println();
          client.stop();
          return U_NOTHING;
        }
        break;
      case CMD_SHADESTOP:
        if (mode == M_SHADES) {
          client.println(F(HTTP_200_OK));
          client.println();
          client.stop();
          return CMD_SHADESTOP;
        } else {
          client.println(F(HTTP_403_Error));
          client.println();
          client.stop();
          return U_NOTHING;
        }
        break;
      case CMD_MODE_LIGHTS:
        client.println(F(HTTP_200_OK));
        client.println();
        client.stop();
        return CMD_MODE_LIGHTS;
        break;
      case CMD_MODE_SHADES:
        client.println(F(HTTP_200_OK));
        client.println();
        client.stop();
        return CMD_MODE_SHADES;
        break;
      case CMD_TIMER_POS:
        if (mode == M_SHADES) {
          lastDevID = getValue(buff, DEVID);
          lastShadePositionTimer = getValue(buff, VALUE);
          client.println(F(HTTP_200_OK));
          client.println();
          client.stop();
          return CMD_TIMER_POS;
        } else {
          client.println(F(HTTP_403_Error));
          client.println();
          client.stop();
          return U_NOTHING;
        }
        break;
      case CMD_TIMER_TILT:
        if (mode == M_SHADES) {
          lastDevID = getValue(buff, DEVID);
          lastShadeTiltTimer = getValue(buff, VALUE);
          client.println(F(HTTP_200_OK));
          client.println();
          client.stop();
          return CMD_TIMER_TILT;
        } else {
          client.println(F(HTTP_403_Error));
          client.println();
          client.stop();
          return U_NOTHING;
        }
        break;
      case CMD_INPUT_HOLD:
        if (mode == M_LIGHTS) {
          lastDevID = getValue(buff, DEVID);
          client.println(F(HTTP_200_OK));
          client.println();
          client.stop();
          return CMD_INPUT_HOLD;
        } else {
          client.println(F(HTTP_403_Error));
          client.println();
          client.stop();
          return U_NOTHING;
        }
        break;
      case CMD_INPUT_REL:
        if (mode == M_LIGHTS) {
          lastDevID = getValue(buff, DEVID);
          client.println(F(HTTP_200_OK));
          client.println();
          client.stop();
          return CMD_INPUT_REL;
        } else {
          client.println(F(HTTP_403_Error));
          client.println();
          client.stop();
          return U_NOTHING;
        }
        break;
      case CMD_DEREGISTER:
        client.println(F(HTTP_200_OK));
        client.println();
        client.stop();
        return CMD_DEREGISTER;
        break;
      case CMD_UNKNOWN:
        client.println(F(HTTP_500_Error));
        client.stop();
        return CMD_UNKNOWN;
        break;
    }
  }
  if (signalIPchange) {
    signalIPchange = false;
    return U_RASPYIPCHGD;
  }

  return U_NOTHING;
}

static byte ARiFClass::beginEthernet(byte mac[]) {
  if (Ethernet.begin(mac) == 0) {
    Serial.println(F("Failed to configure Ethernet using DHCP"));
    isConnected = false;
    DHCPFailed = true; // mark that the DHCP failed at boot
    return 0;
  } else { /* Ethernet initialization succesfull */
    Serial.println(F("NIC initialized. Ethernet cable connected."));
    Serial.print("My IP address: ");
    Serial.println(Ethernet.localIP());

    UDP.beginMulticast(mip, ARiF_BEACON_PORT);
    ARiFServer.begin();
  }
}

static long ARiFClass::getValue(char *buff, int value) {
  char *pos;
  if (value == DEVID) {
    pos = strstr(buff, "devID=");
    return atoi(pos + 6);
  }
  if (value == ARDID) {
    pos = strstr(buff, "ardID=");
    return atoi(pos + 6);
  }
  if (value == RASPYID) {
    pos = strstr(buff, "raspyID=");
    return atoi(pos + 8);
  }
  if (value == VALUE) {
    pos = strstr(buff, "value=");
    return atoi(pos + 6);
  }
  if (value == VALUE_L) {
    pos = strstr(buff, "value=");
    return atol(pos + 6);
  }
  if (value == CMD ) {
    if (strstr(buff, "cmd=lightType")) return CMD_LIGHT_TYPE;
    if (strstr(buff, "cmd=lightTimer")) return CMD_LIGHT_TIMER;
    if (strstr(buff, "cmd=register")) return CMD_REGISTER;
    if (strstr(buff, "cmd=heartbeat")) return CMD_HEARTBEAT;
    if (strstr(buff, "cmd=lightON")) return CMD_LIGHTON;
    if (strstr(buff, "cmd=lightOFF")) return CMD_LIGHTOFF;
    if (strstr(buff, "cmd=shadePOS")) return CMD_SHADEPOS;
    if (strstr(buff, "cmd=shadeTILT")) return CMD_SHADETILT;
    if (strstr(buff, "cmd=shadeUP")) return CMD_SHADEUP;
    if (strstr(buff, "cmd=shadeDOWN")) return CMD_SHADEDOWN;
    if (strstr(buff, "cmd=shadeSTOP")) return CMD_SHADESTOP;
    if (strstr(buff, "cmd=ctrlON")) return CMD_CTRL_ON;
    if (strstr(buff, "cmd=ctrlOFF")) return CMD_CTRL_OFF;
    if (strstr(buff, "cmd=modeLights")) return CMD_MODE_LIGHTS;
    if (strstr(buff, "cmd=modeShades")) return CMD_MODE_SHADES;
    if (strstr(buff, "cmd=shadePTimer")) return CMD_TIMER_POS;
    if (strstr(buff, "cmd=shadeTTimer")) return CMD_TIMER_TILT;
    if (strstr(buff, "cmd=inputHold")) return CMD_INPUT_HOLD;
    if (strstr(buff, "cmd=inputRelease")) return CMD_INPUT_REL;
    if (strstr(buff, "cmd=deregister")) return CMD_DEREGISTER;
    return CMD_UNKNOWN;
  }
}

static bool ARiFClass::checkIotGwIP(IPAddress ip) {
  if (raspyIP == ip) {
    return true;
  } else {
    raspyIP = ip;
    return false;
  }
}

static void ARiFClass::sendShadeStatus(byte devID, byte dataType, byte value) {
  if (!isConnected) return;  // exit function if the link is dead;
  if (ARiFClient.connect(ARiFClass::raspyIP, ARiF_HTTP_PORT)) {
    Serial.print("devID: ");
    Serial.print(devID);
    Serial.print(" sending to "); // to be removed
    Serial.print(ARiFClient.remoteIP()); // to be removed
    // Make a HTTP request:
    ARiFClient.print("POST /?devID=");
    ARiFClient.print(devID);
    ARiFClient.print("&ardID=");
    ARiFClient.print(ardID);
    ARiFClient.print("&raspyID=");
    if (raspyID < 10) {
      ARiFClient.print("00");
      ARiFClient.print(raspyID);
    } else {
      if (raspyID >= 10 < 100) {
        ARiFClient.print("0");
        ARiFClient.print(raspyID);
      } else {
        ARiFClient.print(raspyID);
      }
    }
    if (dataType == DT_DIRECTION) {
      Serial.print(" direction -> ");
      ARiFClient.print(F("&cmd=status&devType=shade&dataType=direction&value="));
      if (value == VAL_MOVE_DOWN) {
        ARiFClient.print("down\n");
        Serial.println(" down. ");
      } else if (value == VAL_MOVE_UP) {
        ARiFClient.print("up\n");
        Serial.println(" up. ");
      } else if (value == VAL_STOPPED) {
        ARiFClient.print("stop\n");
        Serial.println(" stop. ");
      }
    } else if (dataType == DT_POSITION) {
      Serial.print(" position: ");
      Serial.println(value);
      ARiFClient.print(F("&cmd=status&devType=shade&dataType=position&value="));
      ARiFClient.print(value);
      ARiFClient.print("\n");
    } else if (dataType == DT_TILT) {
      ARiFClient.print(F("&cmd=status&devType=shade&dataType=tilt&value="));
      Serial.print(" tilt: ");
      Serial.println(value);
      ARiFClient.print(value);
      ARiFClient.print("\n");
    } else if (dataType == DT_SYNC) {
      ARiFClient.print(F("&cmd=status&devType=shade&dataType=sync&value="));
      if (value == VAL_UNSYNC) {
        ARiFClient.print("0\n");
      } else if (value == VAL_SYNC) {
        ARiFClient.print("1\n");
      }
    }

    ARiFClient.println("Host: raspy");
    ARiFClient.println("Connection: close");
    ARiFClient.println();
  } else {
    Serial.println("Problem with connecting");
  }
}

static void ARiFClass::sendShadeUp(byte devID) {
  sendShadeStatus(devID, DT_DIRECTION, VAL_MOVE_UP);
}

static void ARiFClass::sendShadeDown(byte devID) {
  sendShadeStatus(devID, DT_DIRECTION, VAL_MOVE_DOWN);
}

static void ARiFClass::sendShadeStop(byte devID) {
  sendShadeStatus(devID, DT_DIRECTION, VAL_STOPPED);
}

static void ARiFClass::sendShadePosition(byte devID, byte position) {
  sendShadeStatus(devID, DT_POSITION, position);
}

static void ARiFClass::sendShadeTilt(byte devID, byte tilt) {
  sendShadeStatus(devID, DT_TILT, tilt);
}

static void ARiFClass::sendShadeSynced(byte devID) {
  sendShadeStatus(devID, DT_SYNC, VAL_SYNC);
}

static void ARiFClass::sendShadeUnsynced(byte devID) {
  sendShadeStatus(devID, DT_SYNC, VAL_UNSYNC);
}

static IPAddress ARiFClass::getRaspyIP() {
  return ARiFClass::raspyIP;
}

static byte ARiFClass::getRaspyID() {
  return ARiFClass::raspyID;
}

static byte ARiFClass::getArdID() {
  return ARiFClass::ardID;
}

bool ARiFClass::timeCheck(struct t *t ) {
  if ((unsigned long)(millis() - t->tStart) > t->tTimeout) {
    return true;
  } else {
    return false;
  }
}

void ARiFClass::timeRun(struct t *t) {
  t->tStart = millis();
}

byte ARiFClass::getLastDevID() {
  return ARiFClass::lastDevID;
}

byte ARiFClass::getLastShadePosition() {
  return lastShadePosition;
}

byte ARiFClass::getLastShadeTilt() {
  return lastShadeTilt;
}

byte ARiFClass::getLastLightType() {
  return lastLightType;
}

unsigned long ARiFClass::getLastLightTimer() {
  return lastLightTimer;
}

int ARiFClass::getLastShadePositionTimer() {
  return lastShadePositionTimer;
}

int ARiFClass::getLastShadeTiltTimer() {
  return lastShadeTiltTimer;
}

void ARiFClass::deregister() {
  isRegistered = false;
  ardID = 0;
}

static void ARiFClass::sendLightStatus(byte devID, byte value) {
  if (!isConnected) return;  // exit function if the link is dead;
  if (ARiFClient.connect(ARiFClass::raspyIP, ARiF_HTTP_PORT)) {
    Serial.print("devID: ");
    Serial.print(devID);
    Serial.print(" sending to "); // to be removed
    Serial.print(ARiFClient.remoteIP()); // to be removed
    // Make a HTTP request:
    ARiFClient.print("POST /?devID=");
    ARiFClient.print(devID);
    ARiFClient.print("&ardID=");
    ARiFClient.print(ardID);
    ARiFClient.print("&raspyID=");
    if (raspyID < 10) {
      ARiFClient.print("00");
      ARiFClient.print(raspyID);
    } else {
      if (raspyID >= 10 < 100) {
        ARiFClient.print("0");
        ARiFClient.print(raspyID);
      } else {
        ARiFClient.print(raspyID);
      }
    }

    ARiFClient.print(F("&cmd=status&devType=digitOUT&dataType=bool&value="));
    if (value == VAL_OFF) {
      ARiFClient.print("0\n");
      Serial.println(" OFF. ");
    } else if (value == VAL_ON) {
      ARiFClient.print("1\n");
      Serial.println(" ON. ");
    }


    ARiFClient.println("Host: raspy");
    ARiFClient.println("Connection: close");
    ARiFClient.println();
  } else {
    Serial.println("Problem with connecting");
  }
}

static void ARiFClass::sendLightON(byte devID) {
  sendLightStatus(devID, VAL_ON);
}

static void ARiFClass::sendLightOFF(byte devID) {
  sendLightStatus(devID, VAL_OFF);
}

static void ARiFClass::setMode(byte m) {
  mode = m;
}
