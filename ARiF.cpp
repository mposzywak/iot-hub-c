#include "ARiF.h"

static byte ARiFClass::version;
static byte ARiFClass::ardID = 0;
static byte ARiFClass::raspyID;
static byte ARiFClass::mac[] = ARiF_INITIAL_MAC;
static byte ARiFClass::raspyMAC[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
static IPAddress ARiFClass::raspyIP;
static bool ARiFClass::DHCPFailed = false;
static bool ARiFClass::restartNewMAC = false;
static bool ARiFClass::initDHCPFailed = false;
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
static ARiFClass::t ARiFClass::t_func3 = {0, 60000}; /* for every minute send settings with timeout */
static byte ARiFClass::lastShadePosition = 0;
static byte ARiFClass::lastShadeTilt = 0;
static int ARiFClass::lastShadePositionTimer = 0;
static int ARiFClass::lastShadeTiltTimer = 0;
static byte ARiFClass::mode = 0;
static byte ARiFClass::lastLightType = 0;
static byte ARiFClass::lastLightInputType = 0;
static byte ARiFClass::lastLightCtrlON = 0;
static unsigned long ARiFClass::lastLightTimer = 0;
static byte ARiFClass::ctrlON = 0;
static byte ARiFClass::restore = ARIF_RESTORE_DISABLED;

static byte ARiFClass::begin(byte version, byte mac[]) {
  ARiFClass::version = version;
  beginEthernet();
  isRegistered = false;
  return 0;
}

static byte ARiFClass::begin(byte version, byte mac[], IPAddress raspyIP, byte ardID, byte raspyID) {
  ARiFClass::version = version;
  ARiFClass::raspyIP = raspyIP;
  ARiFClass::ardID = ardID;
  ARiFClass::raspyID = raspyID;
  beginEthernet();
  isRegistered = true;
  return 0;
}

static byte ARiFClass::beginEthernet() {
  /*for (byte i = 0; i < 6; i++) {
    mac[i] = m[i];
    }*/
  if (Platform.EEPROMGetUseDefMAC() == EEPROM_FLG_MEM_MAC) {
    Serial.println(F("Using MAC from EEPROM"));
    Platform.EEPROMGetMAC(mac);
  } else {
    Serial.println(F("Using default MAC"));
    mac[0] = 0x00;
    mac[1] = 0xAA;
    mac[2] = 0xBB;
    mac[3] = 0x13;
    mac[4] = 0xF9;
    mac[5] = 0x45;
  }
  Serial.print(F("Using MAC: "));
  printMAC(mac);
  if (Ethernet.begin(mac) == 0) {
    Serial.println(F("Failed to configure Ethernet using DHCP"));
    isConnected = false;
    initDHCPFailed = true; // mark that the DHCP failed at boot
    return 0;
  } else { /* Ethernet initialization succesfull */
    Serial.println(F("NIC initialized. Ethernet cable connected."));
    Serial.print(F("My IP address: "));
    Serial.println(Ethernet.localIP());

    UDP.beginMulticast(mip, ARiF_BEACON_PORT);
    ARiFServer.begin();
  }
}

static byte ARiFClass::replaceMAC() {
  Serial.print(F("Received MAC from raspy: "));
  printMAC(raspyMAC);
  bool sameMAC = true;
  for (byte i = 0; i < 6; i++) {
    if (mac[i] != raspyMAC[i])
      sameMAC = false;
  }
  if (sameMAC) {
    Serial.print(F("Same MAC arrived from raspy."));
    restartNewMAC = false;
  } else {
    Serial.print(F("New MAC arrived from raspy."));
    for (byte i = 0; i < 6; i++) {
      mac[i] = raspyMAC[i];
    }
    restartNewMAC = true;
  }
}

static byte ARiFClass::update() {
  if (initDHCPFailed) /* abort the function if we don't have an IP address */
    return U_NOTHING;
  char beacon[ARiF_BEACON_LENGTH];
  if (lastHeartbeat > ARiF_HB_TIMEOUT) {
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

  if (timeCheck(&t_func3)) {
    //Serial.println("Sending settings!!!");
    sendSettings();
    timeRun(&t_func3);
  }

  if (restartNewMAC == true) {
    Serial.println(F("Restarting Ethernet after obtaining new MAC"));
    Platform.EEPROMSetUseDefMAC(EEPROM_FLG_MEM_MAC);
    Platform.EEPROMSetMAC(mac); /* this must be called before beginEthernet(), because it reads MAC from EEPROM */
    beginEthernet();
    restartNewMAC = false;
  }

  EthernetClient client = ARiFServer.available();
  if (client) {
    Serial.print("HTTP Request received from: ");
    IPAddress clientIP;
    clientIP = client.remoteIP();
    Serial.println(clientIP);
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
          if (!compareIP(clientIP, raspyIP)) { /* IP is different than the stored one */
            raspyIP = clientIP;
            return U_RASPYIPCHGD;
          }
        } else {
          client.println(F(HTTP_403_Error)); /* write 403 because the unexpected HB */
          client.stop();                  /* send */
          Serial.println(F("Unexpected heartbeat received. Sending 403."));
        }
        break;
      case CMD_REGISTER:
        //if (!isRegistered) { // this is commented out to allow the arduino to be "re-registered"
        ardID = getValue(buff, ARDID);
        raspyID = getValue(buff, RASPYID);
        raspyIP = client.remoteIP();
        isRegistered = true;
        getMAC(buff, raspyMAC);
        replaceMAC();
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
      case CMD_INPUT_OVERRIDE_ON:
        if (mode == M_LIGHTS) {
          lastDevID = getValue(buff, DEVID);
          client.println(F(HTTP_200_OK));
          client.println();
          client.stop();
          return CMD_INPUT_OVERRIDE_ON;
        } else {
          client.println(F(HTTP_403_Error));
          client.println();
          client.stop();
          return U_NOTHING;
        }
        break;
      case CMD_INPUT_OVERRIDE_OFF:
        if (mode == M_LIGHTS) {
          lastDevID = getValue(buff, DEVID);
          client.println(F(HTTP_200_OK));
          client.println();
          client.stop();
          return CMD_INPUT_OVERRIDE_OFF;
        } else {
          client.println(F(HTTP_403_Error));
          client.println();
          client.stop();
          return U_NOTHING;
        }
        break;
      case CMD_CTRL_ON:
        if (mode == M_LIGHTS) {
          client.println(F(HTTP_200_OK));
          client.println();
          client.stop();
          return CMD_CTRL_ON;
        } else {
          client.println(F(HTTP_403_Error));
          client.println();
          client.stop();
          return U_NOTHING;
        }
        break;
      case CMD_CTRL_OFF:
        if (mode == M_LIGHTS) {
          client.println(F(HTTP_200_OK));
          client.println();
          client.stop();
          return CMD_CTRL_OFF;
        } else {
          client.println(F(HTTP_403_Error));
          client.println();
          client.stop();
          return U_NOTHING;
        }
        break;
      case CMD_CTRL_DEV_ON:
        if (mode == M_LIGHTS) {
          lastDevID = getValue(buff, DEVID);
          client.println(F(HTTP_200_OK));
          client.println();
          client.stop();
          return CMD_CTRL_DEV_ON;
        } else {
          client.println(F(HTTP_403_Error));
          client.println();
          client.stop();
          return U_NOTHING;
        }
        break;
      case CMD_CTRL_DEV_OFF:
        if (mode == M_LIGHTS) {
          lastDevID = getValue(buff, DEVID);
          client.println(F(HTTP_200_OK));
          client.println();
          client.stop();
          return CMD_CTRL_DEV_OFF;
        } else {
          client.println(F(HTTP_403_Error));
          client.println();
          client.stop();
          return U_NOTHING;
        }
        break;
      case CMD_LIGHT_SETTINGS:
        if (mode == M_LIGHTS) {
          lastDevID = getValue(buff, DEVID);
          Serial.print("lightSettings: ");
          Serial.println(buff);
          client.println(F(HTTP_200_OK));
          client.println();
          client.stop();
          return CMD_LIGHT_SETTINGS;
        } else {
          client.println(F(HTTP_404_Error));
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
      case CMD_RESTORE:
        client.println(F(HTTP_200_OK));
        client.println();
        client.stop();
        restore = ARIF_RESTORE_ENABLED;
        return CMD_RESTORE;
        break;
      case CMD_UNKNOWN:
        client.println(F(HTTP_404_Error));
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
    if (strstr(buff, "cmd=inputOverrideON")) return CMD_INPUT_OVERRIDE_ON;
    if (strstr(buff, "cmd=inputOverrideOFF")) return CMD_INPUT_OVERRIDE_OFF;
    if (strstr(buff, "cmd=devCtrlON")) return CMD_CTRL_DEV_ON;
    if (strstr(buff, "cmd=devCtrlOFF")) return CMD_CTRL_DEV_OFF;
    if (strstr(buff, "cmd=lightSettings")) return CMD_LIGHT_SETTINGS;
    if (strstr(buff, "cmd=restore")) return CMD_RESTORE;
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
    Serial.print(F("devID: "));
    Serial.print(devID);
    Serial.print(F(" sending to ")); // to be removed
    Serial.println(ARiFClient.remoteIP()); // to be removed
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
    if (dataType == DT_DIRECTION || dataType == DT_DIRECTION_USER) {
      Serial.print(" direction -> ");
      ARiFClient.print(F("&cmd=status&devType=shade&dataType=direction&value="));
      if (value == VAL_MOVE_DOWN) {
        ARiFClient.print("down\n");
        Serial.println(F(" down. "));
      } else if (value == VAL_MOVE_UP) {
        ARiFClient.print("up\n");
        Serial.println(F(" up. "));
      } else if (value == VAL_STOPPED) {
        ARiFClient.print("stop\n");
        Serial.println(F(" stop. "));
      }
    } else if (dataType == DT_POSITION) {
      Serial.print(F(" position: "));
      Serial.println(value);
      ARiFClient.print(F("&cmd=status&devType=shade&dataType=position&value="));
      ARiFClient.print(value);
      ARiFClient.print("\n");
    } else if (dataType == DT_TILT || dataType == DT_TILT_USER) {
      ARiFClient.print(F("&cmd=status&devType=shade&dataType=tilt&value="));
      Serial.print(F(" tilt: "));
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

    if (dataType == DT_DIRECTION_USER || dataType == DT_TILT_USER) {
      ARiFClient.println("iot-user: true");
    }

    ARiFClient.println("Host: raspy");
    ARiFClient.println("Connection: close");
    ARiFClient.println();
  } else {
    Serial.println(F("Problem with connecting"));
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

static void ARiFClass::sendUserShadeUp(byte devID) {
  sendShadeStatus(devID, DT_DIRECTION_USER, VAL_MOVE_UP);
}

static void ARiFClass::sendUserShadeDown(byte devID) {
  sendShadeStatus(devID, DT_DIRECTION_USER, VAL_MOVE_DOWN);
}

static void ARiFClass::sendUserShadeStop(byte devID) {
  sendShadeStatus(devID, DT_DIRECTION_USER, VAL_STOPPED);
}

static void ARiFClass::sendShadePosition(byte devID, byte position) {
  sendShadeStatus(devID, DT_POSITION, position);
}

static void ARiFClass::sendShadeTilt(byte devID, byte tilt) {
  sendShadeStatus(devID, DT_TILT, tilt);
}

static void ARiFClass::sendUserShadeTilt(byte devID, byte tilt) {
  sendShadeStatus(devID, DT_TILT_USER, tilt);
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
  Serial.print(F("Getting last positionTimer: "));
  Serial.println(lastShadePositionTimer);
  return lastShadePositionTimer;
}

int ARiFClass::getLastShadeTiltTimer() {
  return lastShadeTiltTimer;
}

void ARiFClass::deregister() {
  isRegistered = false;
  ardID = 0;
}

static void ARiFClass::sendTempStatus(byte devID, float value) {
  sendFloatStatus(devID, _MTYPE_TEMP, value);
}

static void ARiFClass::sendHumidityStatus(byte devID, float value) {
  sendFloatStatus(devID, _MTYPE_HUMIDITY, value);
}

static void ARiFClass::sendFloatStatus(byte devID, byte messageType, float value) {
  if (!isConnected) return;  // exit function if the link is dead;
  if (ARiFClient.connect(ARiFClass::raspyIP, ARiF_HTTP_PORT)) {

    addPreamble(devID);
    ARiFClient.print("&cmd=status&devType=");
    if (messageType == _MTYPE_TEMP) {
      ARiFClient.print("temp");
    } else if (messageType == _MTYPE_HUMIDITY) {
      ARiFClient.print("humidity");
    }
    ARiFClient.print("&dataType=float&value=");
    ARiFClient.println(value);

    ARiFClient.println("Host: raspy");
    ARiFClient.println("Connection: keep-alive");
    ARiFClient.println();
  } else {
    Serial.println(F("Problem with connecting"));
  }
}

static void ARiFClass::sendSettings() {
  if (!isConnected) return;  // exit function if the link is dead;
  if (ARiFClient.connect(ARiFClass::raspyIP, ARiF_HTTP_PORT)) {
    addPreamble(0);
    ARiFClient.print(F("&cmd=settings\n"));
    Serial.println(F("Sending settings"));

    ARiFClient.println("Host: raspy");
    ARiFClient.println("Connection: close");
    ARiFClient.println("Content-Type: application/json");
    String json = "{\"version\":\"";
    json = json + VERSION;
    json = json + "\",\"ctrlON\":" + ctrlON + ",\"mode\":" + mode + ", \"uptime\":" + millis() + ", \"restore\":" + restore + "}";
    String content = "Content-Length: ";
    content = content + json.length() + "\r\n";
    ARiFClient.println(content);
    ARiFClient.println(json);
    ARiFClient.println();
  } else {
    Serial.println(F("Problem with connecting"));
  }
}

static void ARiFClass::sendShadeSettings(byte devID, int positionTimer, int tiltTimer) {
  if (!isConnected) return;  // exit function if the link is dead;
  //unsigned long sPositionTimer = 0;
  //sPositionTimer = (unsigned long) positionTimer * 100;
  if (ARiFClient.connect(ARiFClass::raspyIP, ARiF_HTTP_PORT)) {
    addPreamble(devID);
    ARiFClient.print(F("&cmd=shadeSettings\n"));
    Serial.println(F("Sending shade settings"));

    ARiFClient.println("Host: raspy");
    ARiFClient.println("Connection: close");
    ARiFClient.println("Content-Type: application/json");
    String json = "{\"posTimer\":\"";
    json = json + positionTimer;
    json = json + "\",\"tiltTimer\":" + tiltTimer + "}";
    String content = "Content-Length: ";
    content = content + json.length() + "\r\n";
    ARiFClient.println(content);
    ARiFClient.println(json);
    ARiFClient.println();
  } else {
    Serial.println(F("Problem with connecting"));
  }
}

static void ARiFClass::sendLightSettings(byte devID, unsigned long timer, byte type, byte inputType, byte ctrlON) {
  if (!isConnected) return;  // exit function if the link is dead;
  if (ARiFClient.connect(ARiFClass::raspyIP, ARiF_HTTP_PORT)) {
    addPreamble(devID);
    ARiFClient.print(F("&cmd=lightSettings\n"));
    Serial.println(F("Sending device settings"));

    ARiFClient.println("Host: raspy");
    ARiFClient.println("Connection: close");
    ARiFClient.println("Content-Type: application/json");
    String json = "{\"timer\":\"";
    json = json + timer;
    json = json + "\",\"ctrlON\":" + ctrlON + ",\"lightType\":" + type + ",\"lightInputType\":" + inputType + "}";
    String content = "Content-Length: ";
    content = content + json.length() + "\r\n";
    ARiFClient.println(content);
    ARiFClient.println(json);
    ARiFClient.println();
  } else {
    Serial.println(F("Problem with connecting"));
  }
}

static void ARiFClass::sendMessage(byte devID, byte messageType, unsigned int value) {
  if (!isConnected) return;  // exit function if the link is dead;
  if (ARiFClient.connect(ARiFClass::raspyIP, ARiF_HTTP_PORT)) {
    addPreamble(devID);
    ARiFClient.print("&cmd=status&devType=digitOUT&dataType=");
    switch (messageType) {
      case _MTYPE_LIGHT_STATUS_ON:
      case _MTYPE_LIGHT_STATUS_ON_USER:
        ARiFClient.println("bool&value=1");
        break;
      case _MTYPE_LIGHT_STATUS_OFF:
      case _MTYPE_LIGHT_STATUS_OFF_USER:
        ARiFClient.println("bool&value=0");
        break;
      case _MTYPE_LIGHT_STATUS_INT_VALUE:
        ARiFClient.print("int&value=");
        ARiFClient.println(value);
    }

    if (messageType == _MTYPE_LIGHT_STATUS_ON_USER || messageType == _MTYPE_LIGHT_STATUS_OFF_USER) {
      ARiFClient.println("iot-user: true");
    }

    ARiFClient.println("Host: raspy");
    ARiFClient.println("Connection: keep-alive");
    ARiFClient.println();
  } else {
    Serial.println(F("Problem with connecting"));
  }
}

static void ARiFClass::addPreamble(byte devID) {
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
}

static void ARiFClass::sendLightON(byte devID) {
  sendMessage(devID, _MTYPE_LIGHT_STATUS_ON, 0);
}

static void ARiFClass::sendUserLightON(byte devID) {
  sendMessage(devID, _MTYPE_LIGHT_STATUS_ON_USER, 0);
}

static void ARiFClass::sendLightOFF(byte devID) {
  sendMessage(devID, _MTYPE_LIGHT_STATUS_OFF, 0);
}

static void ARiFClass::sendUserLightOFF(byte devID) {
  sendMessage(devID, _MTYPE_LIGHT_STATUS_OFF_USER, 0);
}

static void ARiFClass::sendCounter(byte devID, unsigned int counter) {
  sendMessage(devID, _MTYPE_LIGHT_STATUS_INT_VALUE, counter);
}

static void ARiFClass::setMode(byte m) {
  mode = m;
}

static void ARiFClass::setCtrlON(byte c) {
  ctrlON = c;
}

static void ARiFClass::getMAC(char *buff, byte *mac) {
  char *pos;
  pos = strstr(buff, "value=");
  for (int i = 0; i < 6; i++) {
    mac[i] = getByteFromHex(pos + 6 + (i * 3));
  }
}

static void ARiFClass::printMAC(const byte *mac) {
  for (int i = 0; i < 5; i++) {
    if (mac[i] < 16)
      Serial.print("0");
    Serial.print(mac[i], HEX);
    Serial.print(":");
  }
  Serial.println(mac[5], HEX);
}

static byte ARiFClass::getByteFromHex(const char *string) {
  byte v1, v2;
  v1 = nibble(string[0]);
  v2 = nibble(string[1]);

  return (v1 * 16) + v2;
}

static byte ARiFClass::nibble(char c) {
  if (c >= '0' && c <= '9')
    return c - '0';
  if (c >= 'a' && c <= 'f')
    return c - 'a' + 10;
  if (c >= 'A' && c <= 'F')
    return c - 'A' + 10;
  return 0;  // Not a valid hexadecimal character
}

static bool ARiFClass::compareIP(IPAddress ip1, IPAddress ip2) {
  bool ipIsSame = true;

  for (int i = 0; i <= 3; i++) {
    if (ip1[i] != ip2[i]) {
      ipIsSame = false;
    }
  }
  return ipIsSame;
}

static bool ARiFClass::isARiFConnected() {
  return isConnected;
}
