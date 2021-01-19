#include <SPI.h>
#include <string.h>
#include <Console.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <avr/pgmspace.h>
#include <EEPROM.h>
#include "Shade.h"
#include "ARiF.h"
#include "Settings.h"
#include "WebGUI.h"
#include "Light.h"

/*

  CONTROLLINO - smarthouse test, Version 1.1.0

  Used to control outputs by inputs and provide a REST-like API over network to that system.

  Created 8 Oct 2019
  by Maciej
  maciej.poszywak@gmail.com

*/


/*
   ----------------
   --- Settings ---
   ----------------
*/

/* variable controlling if the relays are set to NC or NO
    NC - Normally Closed - the digitOUT is by default in LOW state, LightON -> HIGH state (true)
    NO - Normally Open   - the digitOUT is by default in HIGH state, LightON -> LOW state (false)
*/
bool relaysNC = true;

/* This value is set to:
   MODE_LIGHTS - where every digitIN toggled changes the state of its corresponding digitOUT (1-to-1 mapping)
   MODE_SHADES - the devices are coupled in 4 for one shade device (separate controls for UP and DOWN directions).
                 Each digitIN enables the corresponding digitOUT, but at the same time disables the paired digitOUT.
                 Additional default timer is implemented.
*/
byte funcMode;

/* MAC address used for initiall boot */
byte mac[] = { 0x00, 0xAA, 0xBB, 0xC6, 0xDD, 0x52 };

/*
   ------------------------
   --- Global Variables ---
   ------------------------
*/

/* starting with not registered iot hub (former arduino) TODO: make this value stored on the EEPROM */
byte ardID = 0;
byte raspyID = 0;

/* holds information if this arduino is registered */
bool isRegistered = false;

/* holds the IP of the Raspy/iot-gw */
IPAddress iotGwIP;


/* instantiate the shades objects */
Shade shades[SHADES];

/* instantiate the lights objects */
Light lights[LIGHTS];

/* value to control when to print the measurement report */
bool measure;
/*
   -----------------
   ---   Setup   ---
   -----------------
*/
void setup() {

  /* initialize serial interface */
  Serial.begin(9600);
  Serial.println("Setup init!");

  /* Set the default mode of the device based on the EEPROM value */
  funcMode = Platform.EEPROMGetMode();

  /* assume default mode on platform where it couldn't be read */
  if (funcMode == MODE_FAIL) {
    funcMode = MODE_LIGHTS;
  }

  if (funcMode == MODE_SHADES) {
    /* initialize the shades */
    for (int i = 0; i < SHADES; i++) {
      shades[i].init(Settings::shadeIDs[i]);
      WebGUI.shadeInit(i, Settings::shadeIDs[i]);
    }
    for (int i = 0; i < LIGHTS; i++) {
      WebGUI.lightInit(i, Platform.lightIDs[i], S_WEBGUI_L_TIMER);
    }
    WebGUI.setSystemMode(M_WEBGUI_SHADES);
  } else if (funcMode == MODE_LIGHTS) {
    /* initialize the lights */
    byte type;
    unsigned long timer;
    for (int i = 0; i < LIGHTS; i++) {
      type = Platform.EEPROMGetLightType(Platform.lightIDs[i]);
      if (type == DIGITOUT_ONOFF) {
        lights[i].init(Platform.lightIDs[i], DIGITOUT_ONOFF);
        WebGUI.lightInit(i, Platform.lightIDs[i], S_WEBGUI_L_ONOFF);
      } else if (type == DIGITOUT_TIMER) {
        timer = Platform.EEPROMGetLightTimer(Platform.lightIDs[i]);
        lights[i].init(Platform.lightIDs[i], DIGITOUT_TIMER);
        lights[i].setTimer(timer);
        WebGUI.lightInit(i, Platform.lightIDs[i], S_WEBGUI_L_TIMER);
        WebGUI.lightSetTimer(Platform.lightIDs[i], timer);
      } else {
        lights[i].init(Platform.lightIDs[i], DIGITOUT_ONOFF);
        WebGUI.lightInit(i, Platform.lightIDs[i], S_WEBGUI_L_ONOFF);
        Platform.EEPROMSetLightConfig(Platform.lightIDs[i], DIGITOUT_ONOFF, DIGITOUT_DEFAULT_TIMER);
      }
    }
    for (int i = 0; i < SHADES; i++) {
      WebGUI.shadeInit(i, Settings::shadeIDs[i]);
    }
    WebGUI.setSystemMode(M_WEBGUI_LIGHTS);
  }

  /* disable SD card on the Ethernet shield */
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);

  /* initialize various platform dependend settings */
  Platform.initPlatform();

  /* get the registration data from EEPROM */
  isRegistered = Platform.EEPROMIsRegistered();
  //isRegistered = false;

  if (isRegistered) {
    Serial.print("Arduino registered with ardID: ");
    ardID = Platform.EEPROMGetArdID();
    Serial.println(ardID);
    Serial.print("Raspy IP: ");
    iotGwIP = Platform.EEPROMGetRaspyIP(iotGwIP);
    Serial.println(iotGwIP);
    Serial.print("RaspyID: ");
    raspyID = Platform.EEPROMGetRaspyID();
    Serial.println(raspyID);
    ARiF.begin(VER_SHD_1, mac, iotGwIP, ardID, raspyID);
    WebGUI.setInfoRegistered(ardID, raspyID, iotGwIP);
  } else {
    Serial.println("Not registered within any iot-gw");
    ARiF.begin(VER_SHD_1, mac);
    WebGUI.setInfoDeregistered();
  }

  /* set ARiF mode */
  if (funcMode == MODE_SHADES) {
    ARiF.setMode(M_SHADES);
  } else if (funcMode == MODE_LIGHTS) {
    ARiF.setMode(M_LIGHTS);
  }

  /* Read the light Central Control setting and set Light class global mode */
  byte centralCtrlMode;
  centralCtrlMode = Platform.EEPROMGetLightCentral();
  if (centralCtrlMode == DIGITOUT_CENTRAL_CTRL_ENABLE) {
    Light::enableCentralCtrl();
  } else if (centralCtrlMode == DIGITOUT_CENTRAL_CTRL_DISABLE) {
    Light::disableCentralCtrl();
  } else {
    Light::disableCentralCtrl();
  }

  WebGUI.begin();

  Serial.println("Setup complete!");
}

/*
   -----------------
   --- Main Loop ---
   -----------------
*/
void loop() {

  /* -- measurement code start -- */
  measure = false;
  TCCR1A = 0;
  TCCR1B = 1;
  uint16_t start = TCNT1;
  /* -- measurement code end -- */

  /*
     ----------------------------
     --- Shade mode operation ---
     ----------------------------
  */
  if (funcMode == MODE_SHADES) {
    for (int i = 0; i < SHADES; i++) {
      shades[i].update();
      /*if (shades[i].update() <= 100) {
        Serial.println("Sending 1 shade position");
        //ARiF.sendShadePosition(shadeIDs[i], shades[i].getCurrentPosition());
      }*/

      byte devID = shades[i].getDevID();
      byte upPressResult = shades[i].isUpPressed();
      byte downPressResult = shades[i].isDownPressed();

      if (upPressResult == PHY_MOMENTARY_PRESS) {
        if (shades[i].isMoving()) {
          shades[i].stopWithTilt();
        } else {
          shades[i].toggleTiltUp();
        }
        measure = true;
      } else if (upPressResult == PHY_PRESS_MORE_THAN_2SEC) {
        if (shades[i].isMoving()) {
          shades[i].stopWithTilt();
        } else {
          shades[i].up();
        }
      }

      if (downPressResult == PHY_MOMENTARY_PRESS) {
        if (shades[i].isMoving()) {
          shades[i].stopWithTilt();
        } else {
          shades[i].toggleTiltDown();
        }
        measure = true;
      } else if (downPressResult == PHY_PRESS_MORE_THAN_2SEC) {
        if (shades[i].isMoving()) {
          shades[i].stopWithTilt();
        } else {
          shades[i].down();
        }
      }

      if (shades[i].justStoppedTilt()) { /* executed on shade stopped tilt movement */
        ARiF.sendShadeTilt(Settings::shadeIDs[i], shades[i].getTilt());
        WebGUI.shadeSetTilt(devID, shades[i].getTilt());
        WebGUI.shadeSetDirection(devID, S_WEBGUI_STOP);
      }

      if (shades[i].justStopped()) { /* executed on shade stopped vertical movement */
        ARiF.sendShadeStop(Settings::shadeIDs[i]);
        ARiF.sendShadePosition(Settings::shadeIDs[i], shades[i].getCurrentPosition());
        WebGUI.shadeSetPosition(devID, shades[i].getCurrentPosition());
      }
      if (shades[i].justStartedDown()) { /* executed on shade started moving down */
        ARiF.sendShadeDown(Settings::shadeIDs[i]);
        WebGUI.shadeSetDirection(devID, S_WEBGUI_DOWN);
      }
      if (shades[i].justStartedUp()) { /* executed on shade started moving up */
        ARiF.sendShadeUp(Settings::shadeIDs[i]);
        WebGUI.shadeSetDirection(devID, S_WEBGUI_UP);
      }
    }

    /*
      -----------------------------
      --- Lights mode operation ---
      -----------------------------
    */
  } else if (funcMode == MODE_LIGHTS) {
    byte devID;
    byte pressResult;
    for (int i = 0; i < LIGHTS; i++) {
      devID = lights[i].getDevID();
      pressResult = lights[i].isPressed();

      if (pressResult == PHY_MOMENTARY_PRESS) {
        lights[i].toggle();
      } else if (pressResult == PHY_PRESS_MORE_THAN_2SEC) {
        lights[i].toggle();
      } else if (pressResult == PHY_CENTRAL_CTRL_MOMENTARY_PRESS) {
        byte devIDCentralPress;
        for (int j = 0; j < LIGHTS; j++) {
          if (lights[j].getType() == DIGITOUT_ONOFF) {
            devIDCentralPress = lights[j].getDevID();
            lights[j].setON();
            WebGUI.lightSetON(devIDCentralPress);
            ARiF.sendLightON(devIDCentralPress);
            delay(DIGITOUT_CENTRAL_CTRL_DELAY);
          }
        }
      } else if (pressResult == PHY_CENTRAL_CTRL_PRESS_MORE_THAN_2SEC) {
        byte devIDCentralPress;
        for (int j = 0; j < LIGHTS; j++) {
          if (lights[j].getType() == DIGITOUT_ONOFF) {
            devIDCentralPress = lights[j].getDevID();
            lights[j].setOFF();
            WebGUI.lightSetOFF(devIDCentralPress);
            ARiF.sendLightOFF(devIDCentralPress);
            delay(DIGITOUT_CENTRAL_CTRL_DELAY);
          }
        }
      }

      if (lights[i].justTurnedON()) {
        WebGUI.lightSetON(devID);
        ARiF.sendLightON(devID);
      }
      if (lights[i].justTurnedOFF()) {
        WebGUI.lightSetOFF(devID);
        ARiF.sendLightOFF(devID);
      }
    }
  }

  /*
    ----------------------------------
    --- Handling of WebGUI actions ---
    ----------------------------------
  */

  byte webGuiRet = WebGUI.update();
  switch (webGuiRet) {
    case CMD_WEBGUI_DEREGISTER:                       /* Deregister button pressed */
      ARiF.deregister();
      Platform.EEPROMDeregister();
      break;
    case CMD_WEBGUI_SET_M_LIGHTS:                     /* Switch Mode to Lights */
      Serial.println("Set Variant to lights");
      funcMode = MODE_LIGHTS;
      WebGUI.setSystemMode(M_WEBGUI_LIGHTS);
      Platform.EEPROMSetMode(MODE_LIGHTS);
      ARiF.setMode(M_LIGHTS);
      for (int i = 0; i < SHADES; i++) {
        shades[i].reset();
      }
      for (int i = 0; i < LIGHTS; i++) {
        lights[i].init(Platform.lightIDs[i], DIGITOUT_TIMER);
        WebGUI.lightInit(i, Platform.lightIDs[i], S_WEBGUI_L_TIMER);
      }
      break;
    case CMD_WEBGUI_SET_M_SHADES:                     /* Switch Mode to Shades */
      Serial.println("Set Variant to shades");
      funcMode = MODE_SHADES;
      WebGUI.setSystemMode(M_WEBGUI_SHADES);
      Platform.EEPROMSetMode(MODE_SHADES);
      ARiF.setMode(M_SHADES);
      for (int i = 0; i < LIGHTS; i++) {
        lights[i].reset();
      }
      for (int i = 0; i < SHADES; i++) {
        shades[i].init(Settings::shadeIDs[i]);
        WebGUI.shadeInit(i, Settings::shadeIDs[i]);
      }
      break;
    case CMD_WEBGUI_NOTHING:                          /* Do nothing */
      break;
  }

  /*
    --------------------------------
    --- Handling of ARiF actions ---
    --------------------------------
  */

  byte ret = ARiF.update();
  byte lastDevID;
  byte lastLightType;
  unsigned long lastLightTimer;
  switch (ret) {
    case U_CONNECTED:                                  /* ARiF connection with the Raspy has been re-established (or established for the first time) */
      Serial.println("Connected back!");
      if (funcMode == MODE_SHADES) {
        for (int i = 0; i < SHADES; i++) {
          if (shades[i].isSynced()) {
            ARiF.sendShadeSynced(Settings::shadeIDs[i]);
            ARiF.sendShadeTilt(Settings::shadeIDs[i], shades[i].getTilt());
            ARiF.sendShadePosition(Settings::shadeIDs[i], shades[i].getCurrentPosition());
          } else {
            ARiF.sendShadeUnsynced(Settings::shadeIDs[i]);
          }
        }
      } else if (funcMode == MODE_LIGHTS) {
        byte devID;
        for (int i = 0; i < LIGHTS; i++) {
          devID = lights[i].getDevID();
          if (lights[i].getStatus()) {
            ARiF.sendLightON(devID);
          } else {
            ARiF.sendLightOFF(devID);
          }
        }
      }
      break;
    case U_NOTHING:                                    /* Do nothing */
      break;
    case CMD_REGISTER:                                 /* Registered to a Raspy */
      Serial.println("Registered!");
      ardID = ARiF.getArdID();
      raspyID = ARiF.getRaspyID();
      iotGwIP = ARiF.getRaspyIP();
      WebGUI.setInfoRegistered(ardID, raspyID, iotGwIP);
      Platform.EEPROMRegister(ardID, raspyID, iotGwIP);
      break;
    case CMD_SHADEUP:                                  /* shadeUP command received */
      Serial.print("ShadeUP received for: ");
      Serial.println(ARiF.getLastDevID());
      lastDevID = ARiF.getLastDevID();
      for (int i = 0; i < SHADES; i++) {
        if (shades[i].getDevID() == lastDevID) shades[i].up();
      }
      break;
    case CMD_SHADEDOWN:                                /* shadeDOWN command received */
      Serial.print("ShadeDOWN received for: ");
      Serial.println(ARiF.getLastDevID());
      lastDevID = ARiF.getLastDevID();
      for (int i = 0; i < SHADES; i++) {
        if (shades[i].getDevID() == lastDevID) shades[i].down();
      }
      break;
    case CMD_SHADEPOS:                                 /* shadePOS command received */
      Serial.print("Shadepos received with value: ");
      Serial.println(ARiF.getLastShadePosition());
      lastDevID = ARiF.getLastDevID();
      //Serial.println(s.getDevID()); // why s.toPosition() doesn't work??
      for (int i = 0; i < SHADES; i++) {
        if (shades[i].getDevID() == lastDevID) shades[i].toPosition(ARiF.getLastShadePosition());
      }
      break;
    case CMD_SHADETILT:                                /* shadeTILT command received */
      Serial.print("Shadetilt received: ");
      Serial.println(ARiF.getLastShadeTilt());
      lastDevID = ARiF.getLastDevID();
      //Serial.println(s.getDevID()); // why s.toPosition() doesn't work??
      for (int i = 0; i < SHADES; i++) {
        if (shades[i].getDevID() == lastDevID) shades[i].setTilt(ARiF.getLastShadeTilt());
      }
      break;
    case CMD_SHADESTOP:                                /* shadeSTOP command received */
      Serial.println("ShadeSTOP received");
      lastDevID = ARiF.getLastDevID();
      //Serial.println(s.getDevID()); // why s.toPosition() doesn't work??
      for (int i = 0; i < SHADES; i++) {
        if (shades[i].getDevID() == lastDevID) {
          shades[i].stopWithTilt();
        }
      }
      break;
    case CMD_LIGHTON:                                  /* lightON command received */
      Serial.print("Received lightON command");
      lastDevID = ARiF.getLastDevID();
      for (int i = 0; i < LIGHTS; i++) {
        if (lights[i].getDevID() == lastDevID) {
          lights[i].toggle();
        }
      }
      break;
    case CMD_LIGHTOFF:                                 /* lightOFF command received */
      Serial.print("Received lightOFF command");
      lastDevID = ARiF.getLastDevID();
      for (int i = 0; i < LIGHTS; i++) {
        if (lights[i].getDevID() == lastDevID) {
          lights[i].toggle();
        }
      }
      break;
    case CMD_LIGHT_TYPE:
      Serial.print("Received lightType command: ");
      lastDevID = ARiF.getLastDevID();
      lastLightType = ARiF.getLastLightType();
      Serial.println(lastLightType);
      for (int i = 0; i < LIGHTS; i++) {
        if (lights[i].getDevID() == lastDevID) {
          lights[i].setType(lastLightType);
          Platform.EEPROMSetLightType(lastDevID, lastLightType);
        }
      }
      WebGUI.lightSetType(lastDevID, ARiF.getLastLightType());
      break;
    case CMD_LIGHT_TIMER:
      Serial.print("Received lightTimer command: ");
      lastDevID = ARiF.getLastDevID();
      lastLightTimer = ARiF.getLastLightTimer();
      Serial.println(lastLightTimer);
      for (int i = 0; i < LIGHTS; i++) {
        if (lights[i].getDevID() == lastDevID) {         
          lights[i].setTimer(lastLightTimer);
          Platform.EEPROMSetLightTimer(lastDevID, lastLightTimer);
        }
      }
      WebGUI.lightSetTimer(lastDevID, ARiF.getLastLightTimer());
      break;
    case CMD_CTRL_ON:
      Light::enableCentralCtrl();
      Platform.EEPROMSetLightCentral(DIGITOUT_CENTRAL_CTRL_ENABLE);
      break;
    case CMD_CTRL_OFF:
      Light::disableCentralCtrl();
      Platform.EEPROMSetLightCentral(DIGITOUT_CENTRAL_CTRL_DISABLE);
      break;
    case CMD_UNKNOWN:                                  /* unknown command received */
      Serial.print("Received unknown command from: ");
      Serial.print(ARiF.getLastDevID());
      break;

  }

  /* -- measurement code start -- */
  uint16_t finish = TCNT1;
  uint16_t overhead = 8;
  uint16_t cycles = finish - start - overhead;
  if (measure) {
    Serial.print("cycles: ");
    Serial.println(cycles);
  }
  /* -- measurement code end -- */
}

/*
   -----------------
   --- Functions ---
   -----------------
*/

/* return the shade object with the given devID */
Shade getShade(byte devID) {
  for (int i = 0; i < SHADES; i++) {
    if (shades[i].getDevID() == devID) return shades[i];
  }
}


/* check if the IP address is the same as the one save in iotGwIP
   if it is not save the new IP into EEPROM*/
bool checkIotGwIP(IPAddress ip) {
  if (iotGwIP == ip) {
    return true;
  } else {
    iotGwIP = ip;
    Platform.EEPROMSetRaspyIP(ip);
    Serial.println("IP changed. Writing to EEPROM");
    return false;
  }
}
