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
#include "Wire.h"

/*

  CONTROLLINO - smarthouse test, Version BarnGA-0.4

  Used to control outputs by inputs and provide a REST-like API over network to that system.

  RELEASE NOTES:

  BarnGA-0.5 (or 1.2.1)
    - fixed the tilt and position timers support by ARiF
    - Added Shade state stored in EEPROM
    - Added "User" indication to shades triggered by physical switch
    - issues:
      - Problem with lack of time gap between changing direction when shade not in sync

      
  BarnGA-0.4
    - Added 1-wire temp sensor
    - implemented factory reset on "restore" cmd and on new install.
    - tag for centralON feature (each device can be marked for being applicable for centralON), new digitIO device commands:
       - devCtrlON
       - devCtrlOFF
    - use MAC address provided by raspy on registration
    - Implemented device settings sent as JSON.
    - added uptime in miliseconds to arduino settings sent as JSON.
    - Issues/limitations found:
       - The ctrlON function works only if the device (last pin) is set to inputRelease mode.

  BarnGA-0.3.1
    - fixed bug that was shutting down networking after 12 hours
    - added MAC printing on init

  BarnGA-0.3
    - fixed the issue with code getting stuck after initial DHCP fails.

  BarnGA-0.2
    - Increased the ARiF timer to 10s
    - Changed MAC address

  BarnGA - initial version to support lights in Libor's Barn

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
byte mac[] = { 0x00, 0xAA, 0xBB, 0x13, 0xF9, 0x76 };

/*
   ------------------------
   --- Global Variables ---
   ------------------------
*/

/* starting with not registered iot hub */
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
  Serial.println(F("###------------"));
  Serial.println(F("### Setup init!"));
  Serial.println(F("###------------"));
  Serial.print(F("Version: "));
  Serial.println(F(VERSION));

  //Platform.EEPROMSetUID();

  if (Platform.EEPROMIsUIDSet() == true) {
    Serial.println(F("UID set - running normally"));
  } else {
    Serial.println(F("UID not set - restoring EEPROM to factory defaults"));
    Platform.EEPROMRaze();
    Platform.EEPROMSetUID();
  }

  /* Set the default mode of the device based on the EEPROM value */
  funcMode = Platform.EEPROMGetMode();

  /* assume default mode on platform where it couldn't be read */
  if (funcMode != MODE_FAIL && funcMode != MODE_SHADES) {
    funcMode = MODE_LIGHTS;
  }

  if (funcMode == MODE_SHADES) {
    initializeShades();
  } else if (funcMode == MODE_LIGHTS) {
    initializeLights();
  }


  /* disable SD card on the Ethernet shield */ // to be removed, this is part of the Settings::SDCardInit()
  //pinMode(4, OUTPUT);
  //digitalWrite(4, HIGH);

  /* initialize various platform dependend settings */
  Platform.initPlatform();

  /* initialize the SD Card */
  if (Platform.SDCardInit() == SD_INIT_SUCCESS) {
    WebGUI.setSDStatusAvailable();
  } else {
    WebGUI.setSDStatusUnavailable();
  }

  /* get the registration data from EEPROM */
  isRegistered = Platform.EEPROMIsRegistered();
  //isRegistered = false;

  /*Serial.print("Using MAC address: ");
    char macbuff[17];
    sprintf(macbuff, "%x:%x:%x:%x:%x:%x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    Serial.println(macbuff);*/

  //Platform.EEPROMSetUseDefMAC(EEPROM_FLG_DEF_MAC);

  if (isRegistered) {
    Serial.print("Arduino registered with ardID: ");
    ardID = Platform.EEPROMGetArdID();
    Serial.println(ardID);
    Serial.print("Raspy IP: ");
    iotGwIP = Platform.EEPROMGetRaspyIP();
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
    ARiF.setCtrlON(ARIF_CTRLON_ENABLED);
  } else if (centralCtrlMode == DIGITOUT_CENTRAL_CTRL_DISABLE) {
    Light::disableCentralCtrl();
    ARiF.setCtrlON(ARIF_CTRLON_DISABLED);
  } else {
    Light::disableCentralCtrl();
    ARiF.setCtrlON(ARIF_CTRLON_DISABLED);
  }

  Wire.begin();

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
        shades[i].setUserPressed();
        if (shades[i].isMoving()) {
          shades[i].stopWithTilt();
        } else {
          shades[i].toggleTiltUp();
        }
        measure = true;
      } else if (upPressResult == PHY_PRESS_MORE_THAN_2SEC) {
        shades[i].setUserPressed();
        if (shades[i].isMoving()) {
          shades[i].stopWithTilt();
        } else {
          shades[i].up();
        }
      }

      if (downPressResult == PHY_MOMENTARY_PRESS) {
        shades[i].setUserPressed();
        if (shades[i].isMoving()) {
          shades[i].stopWithTilt();
        } else {
          shades[i].toggleTiltDown();
        }
        measure = true;
      } else if (downPressResult == PHY_PRESS_MORE_THAN_2SEC) {
        shades[i].setUserPressed();
        if (shades[i].isMoving()) {
          shades[i].stopWithTilt();
        } else {
          shades[i].down();
        }
      }

      if (shades[i].justStoppedTilt()) { /* executed on shade stopped tilt movement */
        if (shades[i].getUserPressed()) {
          ARiF.sendUserShadeTilt(Settings::shadeIDs[i], shades[i].getTilt());
        } else {
          ARiF.sendShadeTilt(Settings::shadeIDs[i], shades[i].getTilt());
        }
        WebGUI.shadeSetTilt(devID, shades[i].getTilt());
        WebGUI.shadeSetDirection(devID, S_WEBGUI_STOP);
        Platform.EEPROMSetShadeTilt(devID, shades[i].getTilt());
      }

      if (shades[i].justStopped()) { /* executed on shade stopped vertical movement */
        if (shades[i].getUserPressed()) {
          ARiF.sendUserShadeStop(Settings::shadeIDs[i]);
          shades[i].clearUserPressed();
        } else {
          ARiF.sendShadeStop(Settings::shadeIDs[i]);
        }
        ARiF.sendShadePosition(Settings::shadeIDs[i], shades[i].getCurrentPosition());
        WebGUI.shadeSetPosition(devID, shades[i].getCurrentPosition());
        Platform.EEPROMSetShadePosition(devID, shades[i].getPosition());
        Platform.EEPROMSetShadeReachedPosition(devID, shades[i].getCurrentPosition());
      }
      if (shades[i].justStartedDown()) { /* executed on shade started moving down */
        if (shades[i].getUserPressed()) {
          ARiF.sendUserShadeDown(Settings::shadeIDs[i]);
          shades[i].clearUserPressed();
        } else {
          ARiF.sendShadeDown(Settings::shadeIDs[i]);
        }
        WebGUI.shadeSetDirection(devID, S_WEBGUI_DOWN);
      }
      if (shades[i].justStartedUp()) { /* executed on shade started moving up */
        if (shades[i].getUserPressed()) {
          ARiF.sendUserShadeUp(Settings::shadeIDs[i]);
          shades[i].clearUserPressed();
        } else {
          ARiF.sendShadeUp(Settings::shadeIDs[i]);
        }
        WebGUI.shadeSetDirection(devID, S_WEBGUI_UP);
      }
      if (shades[i].justSynced()) {
        Serial.println(F("Just synced!"));
        Platform.EEPROMSetShadeSyncFlag(devID);
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
      pressResult = PHY_NO_PRESS;
      pressResult = lights[i].isPressed();

      if (pressResult == PHY_MOMENTARY_PRESS) {
        Serial.println(F("press detected"));
        lights[i].toggle();
      } else if (pressResult == PHY_PRESS_MORE_THAN_2SEC) {
        lights[i].toggle();
      } else if (pressResult == PHY_CENTRAL_CTRL_MOMENTARY_PRESS) {

        byte devIDCentralPress;
        for (int j = 0; j < LIGHTS; j++) {
          if (lights[j].getType() == DIGITOUT_ONOFF) {
            devIDCentralPress = lights[j].getDevID();
            if (lights[j].getCtrlON() == DIGITOUT_CTRLON_ON) {
              lights[j].setON();
              WebGUI.lightSetON(devIDCentralPress);
              ARiF.sendUserLightON(devIDCentralPress);
              delay(DIGITOUT_CENTRAL_CTRL_DELAY);
            }
          }
        }
      } else if (pressResult == PHY_CENTRAL_CTRL_PRESS_MORE_THAN_2SEC) {
        byte devIDCentralPress;
        for (int k = 0; k < LIGHTS; k++) {
          if (lights[k].getType() == DIGITOUT_ONOFF) {
            devIDCentralPress = lights[k].getDevID();
            if (lights[k].getCtrlON() == DIGITOUT_CTRLON_ON) {
              lights[k].setOFF();
              WebGUI.lightSetOFF(devIDCentralPress);
              ARiF.sendUserLightOFF(devIDCentralPress);
              delay(DIGITOUT_CENTRAL_CTRL_DELAY);
            }
          }
        }
      }

      if (lights[i].justTurnedON()) {
        WebGUI.lightSetON(devID);
        ARiF.sendUserLightON(devID);
      }
      if (lights[i].justTurnedOFF()) {
        WebGUI.lightSetOFF(devID);
        ARiF.sendUserLightOFF(devID);
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
      setModeLights();
      break;
    case CMD_WEBGUI_SET_M_SHADES:                     /* Switch Mode to Shades */
      Serial.println("Set Variant to shades");
      setModeShades();
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
  int lastShadePositionTimer;
  int lastShadeTiltTimer;
  switch (ret) {
    case U_CONNECTED:                                  /* ARiF connection with the Raspy has been re-established (or established for the first time) */
      Serial.println("Connected back!");
      if (funcMode == MODE_SHADES) {
        sendShadeStatus();
      } else if (funcMode == MODE_LIGHTS) {
        sendLightStatus();
      }

      ARiF.sendSettings();
      break;
    case U_NOTHING:                                    /* Do nothing */
      break;
    case CMD_REGISTER:                                 /* Registered to a Raspy */
      Serial.println(F("Registered!"));
      ardID = ARiF.getArdID();
      raspyID = ARiF.getRaspyID();
      iotGwIP = ARiF.getRaspyIP();
      WebGUI.setInfoRegistered(ardID, raspyID, iotGwIP);
      Platform.EEPROMRegister(ardID, raspyID, iotGwIP);
      break;
    case CMD_SHADEUP:                                  /* shadeUP command received */
      Serial.print(F("ShadeUP received for: "));
      Serial.println(ARiF.getLastDevID());
      lastDevID = ARiF.getLastDevID();
      for (int i = 0; i < SHADES; i++) {
        if (shades[i].getDevID() == lastDevID) shades[i].up();
      }
      break;
    case CMD_SHADEDOWN:                                /* shadeDOWN command received */
      Serial.print(F("ShadeDOWN received for: "));
      Serial.println(ARiF.getLastDevID());
      lastDevID = ARiF.getLastDevID();
      for (int i = 0; i < SHADES; i++) {
        if (shades[i].getDevID() == lastDevID) shades[i].down();
      }
      break;
    case CMD_SHADEPOS:                                 /* shadePOS command received */
      Serial.print(F("Shadepos received with value: "));
      Serial.println(ARiF.getLastShadePosition());
      lastDevID = ARiF.getLastDevID();
      //Serial.println(s.getDevID()); // why s.toPosition() doesn't work??
      for (int i = 0; i < SHADES; i++) {
        if (shades[i].getDevID() == lastDevID) shades[i].toPosition(ARiF.getLastShadePosition());
      }
      break;
    case CMD_SHADETILT:                                /* shadeTILT command received */
      Serial.print(F("Shadetilt received: "));
      Serial.println(ARiF.getLastShadeTilt());
      lastDevID = ARiF.getLastDevID();
      for (int i = 0; i < SHADES; i++) {
        if (shades[i].getDevID() == lastDevID) shades[i].setTilt(ARiF.getLastShadeTilt());
      }
      break;
    case CMD_SHADESTOP:                                /* shadeSTOP command received */
      Serial.println(F("ShadeSTOP received"));
      lastDevID = ARiF.getLastDevID();
      //Serial.println(s.getDevID()); // why s.toPosition() doesn't work??
      for (int i = 0; i < SHADES; i++) {
        if (shades[i].getDevID() == lastDevID) {
          shades[i].stopWithTilt();
        }
      }
      break;
    case CMD_LIGHTON:                                  /* lightON command received */
      Serial.println(F("Received lightON command"));
      lastDevID = ARiF.getLastDevID();
      for (int i = 0; i < LIGHTS; i++) {
        if (lights[i].getDevID() == lastDevID) {
          lights[i].setON();
        }
      }
      break;
    case CMD_LIGHTOFF:                                 /* lightOFF command received */
      Serial.println(F("Received lightOFF command"));
      lastDevID = ARiF.getLastDevID();
      for (int i = 0; i < LIGHTS; i++) {
        if (lights[i].getDevID() == lastDevID) {
          lights[i].setOFF();
        }
      }
      break;
    case CMD_LIGHT_TYPE:                               /* lightType command received */
      Serial.print(F("Received lightType command: "));
      lastDevID = ARiF.getLastDevID();
      lastLightType = ARiF.getLastLightType();
      Serial.println(lastLightType);
      for (int i = 0; i < LIGHTS; i++) {
        if (lights[i].getDevID() == lastDevID) {
          lights[i].setType(lastLightType);
          Platform.EEPROMSetLightType(lastDevID, lastLightType);
          ARiF.sendLightSettings(lastDevID, lights[i].getTimer(), lastLightType, lights[i].getInputType(), lights[i].getCtrlON());
        }
      }
      WebGUI.lightSetType(lastDevID, ARiF.getLastLightType());
      break;
    case CMD_LIGHT_TIMER:                              /* lightTimer command received */
      Serial.print(F("Received lightTimer command: "));
      lastDevID = ARiF.getLastDevID();
      lastLightTimer = ARiF.getLastLightTimer();
      Serial.println(lastLightTimer);
      for (int i = 0; i < LIGHTS; i++) {
        if (lights[i].getDevID() == lastDevID) {
          lights[i].setTimer(lastLightTimer);
          Platform.EEPROMSetLightTimer(lastDevID, lastLightTimer);
          ARiF.sendLightSettings(lastDevID, lastLightTimer, lights[i].getType(), lights[i].getInputType(), lights[i].getCtrlON());
        }
      }
      WebGUI.lightSetTimer(lastDevID, ARiF.getLastLightTimer());
      break;
    case CMD_MODE_LIGHTS:                               /* modeLights command received */
      Serial.println(F("Received modeLights command"));
      setModeLights();
      break;
    case CMD_MODE_SHADES:                               /* modeShades command received */
      Serial.println(F("Received modeShades command"));
      setModeShades();
      break;
    case CMD_TIMER_POS:                                 /* shadePTimer command received */
      Serial.print(F("Received shadePTimer command: "));
      lastDevID = ARiF.getLastDevID();
      lastShadePositionTimer = ARiF.getLastShadePositionTimer();
      Platform.EEPROMSetShadePosTimer(lastDevID, lastShadePositionTimer);
      Serial.println(lastShadePositionTimer);
      /* validate if the received timer is within range, if not ignore */
      if (lastShadePositionTimer >= SHADE_POSITION_TIMER_MIN && lastShadePositionTimer <= SHADE_POSITION_TIMER_MAX) {
        for (int i = 0; i < SHADES; i++) {
          if (shades[i].getDevID() == lastDevID) {
            shades[i].setPositionTimer(lastShadePositionTimer);
            Platform.EEPROMSetShadePosTimer(lastDevID, lastShadePositionTimer);
            ARiF.sendShadeSettings(lastDevID, lastShadePositionTimer, shades[i].getTiltTimer());
          }
        }
      } else {
        Serial.println(F("Received shadePTimer value out of range. Ignoring"));
      }
      break;
    case CMD_TIMER_TILT:                                /* shadeTTimer command received */
      Serial.print(F("Received shadeTTimer command: "));
      lastDevID = ARiF.getLastDevID();
      lastShadeTiltTimer = ARiF.getLastShadeTiltTimer();
      Platform.EEPROMSetShadeTiltTimer(lastDevID, lastShadeTiltTimer);
      Serial.println(lastShadeTiltTimer);
      /* validate if the received timer is within range, if not ignore */
      if (lastShadeTiltTimer >= SHADE_TILT_TIMER_MIN && lastShadeTiltTimer <= SHADE_TILT_TIMER_MAX) {
        for (int i = 0; i < SHADES; i++) {
          if (shades[i].getDevID() == lastDevID) {
            shades[i].setTiltTimer(lastShadeTiltTimer);
            Platform.EEPROMSetShadeTiltTimer(lastDevID, lastShadeTiltTimer);
            ARiF.sendShadeSettings(lastDevID, shades[i].getPositionTimer(), lastShadeTiltTimer);
          }
        }
      } else {
        Serial.println(F("Received shadeTTimer value out of range. Ignoring"));
      }
      break;
    case CMD_INPUT_HOLD:                               /* inputHold command received */
      Serial.println(F("Received inputHold command. "));
      lastDevID = ARiF.getLastDevID();
      for (int i = 0; i < LIGHTS; i++) {
        if (lights[i].getDevID() == lastDevID) {
          lights[i].setInputTypeHold();
          Platform.EEPROMSetLightInputType(lastDevID, DIGITOUT_SWITCH_PRESS_HOLD);
          ARiF.sendLightSettings(lastDevID, lights[i].getTimer(), lights[i].getType(), DIGITOUT_SWITCH_PRESS_HOLD, lights[i].getCtrlON());
        }
      }
      break;
    case CMD_INPUT_REL:                                /* inputRelease command received */
      Serial.println(F("Received inputRelease command. "));
      lastDevID = ARiF.getLastDevID();
      for (int i = 0; i < LIGHTS; i++) {
        if (lights[i].getDevID() == lastDevID) {
          lights[i].setInputTypeRelease();
          Platform.EEPROMSetLightInputType(lastDevID, DIGITOUT_SWITCH_PRESS_RELEASE);
          ARiF.sendLightSettings(lastDevID, lights[i].getTimer(), lights[i].getType(), DIGITOUT_SWITCH_PRESS_RELEASE, lights[i].getCtrlON());
        }
      }
      break;
    case CMD_INPUT_OVERRIDE_ON:                         /* inputOverrideOn command received */
      Serial.println(F("Received inputOverrideOn command. "));
      lastDevID = ARiF.getLastDevID();
      for (int i = 0; i < LIGHTS; i++) {
        if (lights[i].getDevID() == lastDevID) {
          lights[i].setInputTypeSimpleHeatOverrideOn();
          Platform.EEPROMSetLightInputType(lastDevID, DIGITOUT_SWITCH_HEAT_OVERRIDE_ON);
          ARiF.sendLightSettings(lastDevID, lights[i].getTimer(), lights[i].getType(), DIGITOUT_SWITCH_HEAT_OVERRIDE_ON, lights[i].getCtrlON());
        }
      }
      break;
    case CMD_INPUT_OVERRIDE_OFF:                         /* inputOverrideOff command received */
      Serial.println(F("Received inputOverrideOff command. "));
      lastDevID = ARiF.getLastDevID();
      for (int i = 0; i < LIGHTS; i++) {
        if (lights[i].getDevID() == lastDevID) {
          lights[i].setInputTypeSimpleHeatOverrideOff();
          Platform.EEPROMSetLightInputType(lastDevID, DIGITOUT_SWITCH_HEAT_OVERRIDE_OFF);
          //ARiF.sendLightInputType(lastDevID, DIGITOUT_SWITCH_HEAT_OVERRIDE_OFF);
          ARiF.sendLightSettings(lastDevID, lights[i].getTimer(), lights[i].getType(), DIGITOUT_SWITCH_HEAT_OVERRIDE_OFF, lights[i].getCtrlON());
        }
      }
      break;
    case CMD_CTRL_DEV_ON:                         /* devCtrlON command received */
      Serial.println(F("Received devCtrlON command. "));
      lastDevID = ARiF.getLastDevID();
      for (int i = 0; i < LIGHTS; i++) {
        if (lights[i].getDevID() == lastDevID) {
          lights[i].setCtrlONEnabled();
          Platform.EEPROMSetLightCtrlON(lastDevID, DIGITOUT_CTRLON_ON);
          Platform.EEPROMGetLightCtrlON(lastDevID);
          ARiF.sendLightSettings(lastDevID, lights[i].getTimer(), lights[i].getType(), lights[i].getInputType(), DIGITOUT_CTRLON_ON);
        }
      }
      break;
    case CMD_CTRL_DEV_OFF:                         /* devCtrlOFF command received */
      Serial.println(F("Received devCtrlOFF command. "));
      lastDevID = ARiF.getLastDevID();
      for (int i = 0; i < LIGHTS; i++) {
        if (lights[i].getDevID() == lastDevID) {
          lights[i].setCtrlONDisabled();
          Platform.EEPROMSetLightCtrlON(lastDevID, DIGITOUT_CTRLON_OFF);
          Platform.EEPROMGetLightCtrlON(lastDevID);
          ARiF.sendLightSettings(lastDevID, lights[i].getTimer(), lights[i].getType(), lights[i].getInputType(), DIGITOUT_CTRLON_OFF);
        }
      }
      break;
    case CMD_CTRL_ON:                                   /* ctrlON command received */
      Serial.println(F("ctrlON received"));
      Light::enableCentralCtrl();
      ARiF.setCtrlON(ARIF_CTRLON_ENABLED);
      Platform.EEPROMSetLightCentral(DIGITOUT_CENTRAL_CTRL_ENABLE);
      ARiF.sendSettings();
      break;
    case CMD_CTRL_OFF:                                  /* ctrlOFF command received */
      Serial.println(F("ctrlOFF received"));
      Light::disableCentralCtrl();
      ARiF.setCtrlON(ARIF_CTRLON_DISABLED);
      Platform.EEPROMSetLightCentral(DIGITOUT_CENTRAL_CTRL_DISABLE);
      ARiF.sendSettings();
      break;
    case CMD_RESTORE:
      Serial.println(F("Received restore cmd"));
      Platform.EEPROMClearUID();
      ARiF.sendSettings();
      break;
    case CMD_DEREGISTER:
      Serial.println(F("Received deregister cmd"));
      ARiF.deregister();
      Platform.EEPROMDeregister();
      break;
    case CMD_UNKNOWN:                                  /* unknown command received */
      Serial.print(F("Received unknown command from: "));
      Serial.print(ARiF.getLastDevID());
      break;

  }

  /*
    -----------------------------------
    --- Handling of oneWire actions ---
    -----------------------------------
  */

  Wire.update();
  float tempValue;
  byte deviceCount = Wire.getDeviceCount();
  for (byte i = 0; i < deviceCount; i++) {
    if (!Wire.isTemperatureRead(40 + i)) {
      tempValue = Wire.getTemperature(40 + i);
      ARiF.sendTempStatus(40 + i, tempValue);
    }
  }
  /*if (!Wire.isTemperatureRead(40)) {
    tempValue = Wire.getTemperature(40);
    ARiF.sendTempStatus(40, tempValue);
    }*/

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
    return false;
  }
}

/* set mode into shades */
void setModeShades() {
  funcMode = MODE_SHADES;
  WebGUI.setSystemMode(M_WEBGUI_SHADES);
  Platform.EEPROMSetMode(MODE_SHADES);
  ARiF.setMode(M_SHADES);
  ARiF.sendSettings();
  for (int i = 0; i < LIGHTS; i++) {
    lights[i].reset();
  }
  initializeShades();
  sendShadeStatus();
}

/* set mode into lights */
void setModeLights() {
  byte devID;
  funcMode = MODE_LIGHTS;
  WebGUI.setSystemMode(M_WEBGUI_LIGHTS);
  Platform.EEPROMSetMode(MODE_LIGHTS);
  ARiF.setMode(M_LIGHTS);
  ARiF.sendSettings();
  for (int i = 0; i < SHADES; i++) {
    shades[i].reset();
  }
  initializeLights();
  sendLightStatus();
}

/* initialize all Lights from the EEPROM */
void initializeLights() {
  Serial.println(F("Initialize system as: MODE_LIGHTS"));
  byte type;
  unsigned long timer;
  byte inputType;
  byte overrideFlag;
  byte devCtrlON;

  for (int i = 0; i < LIGHTS; i++) {
    type = Platform.EEPROMGetLightType(Platform.lightIDs[i]);
    timer = Platform.EEPROMGetLightTimer(Platform.lightIDs[i]);

    if (type == DIGITOUT_ONOFF) {
      lights[i].init(Platform.lightIDs[i], DIGITOUT_ONOFF);
      Serial.print(F("Light: "));
      Serial.print(Platform.lightIDs[i]);
      Serial.print(F(" set to: DIGITOUT_ONOFF"));
      WebGUI.lightInit(i, Platform.lightIDs[i], S_WEBGUI_L_ONOFF);
    } else if (type == DIGITOUT_TIMER) {
      Serial.print(F("Light: "));
      Serial.print(Platform.lightIDs[i]);
      Serial.print(F(" set to: DIGITOUT_TIMER, with timer:"));
      Serial.print(timer);
      lights[i].init(Platform.lightIDs[i], DIGITOUT_TIMER);
      WebGUI.lightInit(i, Platform.lightIDs[i], S_WEBGUI_L_TIMER);
      WebGUI.lightSetTimer(Platform.lightIDs[i], timer);
    } else if (type == DIGITOUT_SIMPLE_HEAT) {
      Serial.print(F("Device: "));
      Serial.print(Platform.lightIDs[i]);
      Serial.print(F(" set to: DIGITOUT_SIMPLE_HEAT"));
      lights[i].init(Platform.lightIDs[i], DIGITOUT_SIMPLE_HEAT);
    } else {
      /* if type cannot be recognized set it by default to DIGITOUT_ONOFF */
      lights[i].init(Platform.lightIDs[i], DIGITOUT_ONOFF);
      Serial.print(F("Light: "));
      Serial.print(Platform.lightIDs[i]);
      Serial.print(F(" set to: DIGITOUT_ONOFF (defaulting)"));
      WebGUI.lightInit(i, Platform.lightIDs[i], S_WEBGUI_L_ONOFF);
      Platform.EEPROMSetLightConfig(Platform.lightIDs[i], DIGITOUT_ONOFF, DIGITOUT_DEFAULT_TIMER);
    }
    /* timer must be set after init() */
    lights[i].setTimer(timer);

    inputType = Platform.EEPROMGetLightInputType(Platform.lightIDs[i]);
    Serial.print(F(" , input type: "));

    if (inputType == DIGITOUT_SWITCH_PRESS_HOLD) {
      lights[i].setInputTypeHold();
      Serial.print(F("DIGITOUT_SWITCH_PRESS_HOLD"));
    } else if (inputType == DIGITOUT_SWITCH_PRESS_RELEASE) {
      lights[i].setInputTypeRelease();
      Serial.print(F("DIGITOUT_SWITCH_PRESS_RELEASE"));
    } else if (inputType == DIGITOUT_SWITCH_HEAT_OVERRIDE_ON) {
      lights[i].setInputTypeSimpleHeatOverrideOn();
      Serial.print(F("DIGITOUT_SWITCH_HEAT_OVERRIDE_ON"));
    } else if (inputType == DIGITOUT_SWITCH_HEAT_OVERRIDE_OFF) {
      lights[i].setInputTypeSimpleHeatOverrideOff();
      Serial.print(F("DIGITOUT_SWITCH_HEAT_OVERRIDE_OFF"));
    } else if (inputType == DIGITOUT_SWITCH_HEAT_TEMP_SENSOR) {
      lights[i].setInputTypeSimpleHeatTempSensor();
      Serial.print(F("DIGITOUT_SWITCH_HEAT_TEMP_SENSOR"));
    } else {
      /* if inputType cannot be recognized set it by default to DIGITOUT_SWITCH_PRESS_RELEASE */
      lights[i].setInputTypeRelease();
      Platform.EEPROMSetLightInputType(Platform.lightIDs[i], DIGITOUT_SWITCH_PRESS_RELEASE);
      Serial.println(F("DIGITOUT_SWITCH_PRESS_RELEASE (defaulting)"));
    }
    devCtrlON = Platform.EEPROMGetLightCtrlON(Platform.lightIDs[i]);
    Serial.print(F(" CtrlON: "));
    if (devCtrlON == DIGITOUT_CTRLON_ON) {
      lights[i].setCtrlONEnabled();
      Serial.print(F("Enabled, "));
    } else if (devCtrlON == DIGITOUT_CTRLON_OFF) {
      lights[i].setCtrlONDisabled();
      Serial.print(F("Disabled, "));
    } else {
      lights[i].setCtrlONDisabled();
      Serial.print(F("Disabled (defaulting), "));
      Platform.EEPROMSetLightCtrlON(Platform.lightIDs[i], DIGITOUT_CTRLON_OFF);
    }
    Serial.print(F("timer: "));
    Serial.println(lights[i].getTimer());
  }

  for (int i = 0; i < LIGHTS; i++) {
    WebGUI.lightInit(i, Platform.lightIDs[i], S_WEBGUI_L_TIMER);
  }
  WebGUI.setSystemMode(M_WEBGUI_LIGHTS);
}

/* Initialize all Shades from EEPROM */
void initializeShades() {
  Serial.println(F("Initialize system as: MODE_SHADES"));
  int posTimer;
  int tiltTimer;
  bool synced;
  int position;
  int tilt;
  byte reachedPosition;
  for (int i = 0; i < SHADES; i++) {
    posTimer = Platform.EEPROMGetShadePosTimer(Platform.shadeIDs[i]);
    tiltTimer = Platform.EEPROMGetShadeTiltTimer(Platform.shadeIDs[i]);
    synced = Platform.EEPROMGetShadeSyncFlag(Platform.shadeIDs[i]);

    /* if recorded EEPROM value is different than the allowed ones, overwrite EEPROM with default */
    /*if (posTimer != Shade::validatePositionTimer(posTimer)) {
      Platform.EEPROMSetShadePosTimer(Platform.shadeIDs[i], Shade::validatePositionTimer(posTimer));
      posTimer = Platform.EEPROMGetShadePosTimer(Platform.shadeIDs[i]);
    }
    if (tiltTimer != Shade::validateTiltTimer(tiltTimer)) {
      Platform.EEPROMSetShadeTiltTimer(Platform.shadeIDs[i], Shade::validateTiltTimer(tiltTimer));
      tiltTimer = Platform.EEPROMGetShadeTiltTimer(Platform.shadeIDs[i]);
    }*/
    
    if (synced) {
      position = Platform.EEPROMGetShadePosition(Platform.shadeIDs[i]);
      tilt = Platform.EEPROMGetShadeTilt(Platform.shadeIDs[i]);
      reachedPosition = Platform.EEPROMGetShadeReachedPosition(Platform.shadeIDs[i]);
      shades[i].init(Settings::shadeIDs[i], synced, tilt, position, reachedPosition, posTimer, tiltTimer);
    } else {
      shades[i].init(Settings::shadeIDs[i], synced, TILT_H_CLOSED, 0, 0, posTimer, tiltTimer);
    }
    WebGUI.shadeInit(i, Settings::shadeIDs[i]);
    
    Serial.print(F("Shade: "));
    Serial.print(shades[i].getDevID());
    Serial.print(F(", posTimer: "));
    Serial.print(posTimer);
    Serial.print(F(", tiltTimer: "));
    Serial.print(tiltTimer);
    Serial.print(F(", synced: "));
    if (synced) {
      Serial.print(synced);
      Serial.print(F(", position: "));
      Serial.print(position);
      Serial.print(F(", tilt: "));
      Serial.println(tilt);
    } else {
      Serial.println(synced);
    }

    //shades[i].setPositionTimer(posTimer);
    //shades[i].setTiltTimer(tiltTimer);
  }

  for (int i = 0; i < SHADES; i++) {
    WebGUI.shadeInit(i, Settings::shadeIDs[i]);
  }
  WebGUI.setSystemMode(M_WEBGUI_SHADES);
}

/* ARiF send all Shades statuses and settings */
void sendShadeStatus() {
  for (int i = 0; i < SHADES; i++) {
    if (shades[i].isSynced()) {
      ARiF.sendShadeSynced(Settings::shadeIDs[i]);
      ARiF.sendShadeTilt(Settings::shadeIDs[i], shades[i].getTilt());
      ARiF.sendShadePosition(Settings::shadeIDs[i], shades[i].getCurrentPosition());
    } else {
      ARiF.sendShadeUnsynced(Settings::shadeIDs[i]);
      Serial.println("Sending unsync");
    }
    ARiF.sendShadeSettings(Settings::shadeIDs[i], shades[i].getPositionTimer(), shades[i].getTiltTimer());
  }
}

/* ARiF send all Lights statuses and settings */
void sendLightStatus() {
  byte devID;
  byte lightType;
  byte lightInputType;
  for (int i = 0; i < LIGHTS; i++) {
    devID = lights[i].getDevID();
    lightType = lights[i].getType();
    lightInputType = lights[i].getInputType();
    if (lights[i].getStatus()) {
      ARiF.sendLightON(devID);
    } else {
      ARiF.sendLightOFF(devID);
    }
    ARiF.sendLightSettings(devID, lights[i].getTimer(), lightType, lightInputType, lights[i].getCtrlON());
  }
}
