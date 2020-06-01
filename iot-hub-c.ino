#include <SPI.h>
#include <string.h>
#include <Console.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <avr/pgmspace.h>
#include <Controllino.h>
#include <EEPROM.h>
#include "Shade.h"
#include "ARiF.h"
#include "Settings.h"

/*
  CONTROLLINO - smarthouse test, Version 01.00

  Used to control outputs by inputs and provide a REST API like over network to that system.
  
  Created 8 Oct 2019
  by Maciej 
  maciej.poszywak@gmail.com

*/


/* indexes for EEPROM information holding */
#define EEPROM_IDX_ARDID    0  // length 1
#define EEPROM_IDX_RASPYID  1  // length 1
#define EEPROM_IDX_REG      2  // length 1
#define EEPROM_IDX_RASPYIP  3  // length 6
#define EEPROM_IDX_NEXT     9

/* number of pins in/out TODO: need to convert into MEGA/MAXI */
#if defined(CONTROLLINO_MEGA)
#define IN_PINS  21
#define OUT_PINS 21
#define SHADES   10
#elif defined(CONTROLLINO_MAXI) 
#define IN_PINS  12
#define OUT_PINS 12
#define SHADES   6
#endif

/* functional modes of the entire device */
#define FUNC_LIGHTS 0
#define FUNC_SHADES 1

/*
 * ----------------
 * --- Settings ---
 * ----------------
 */

/* variable controlling if the relays are set to NC or NO
 *  NC - Normally Closed - the digitOUT is by default in LOW state, LightON -> HIGH state (true)
 *  NO - Normally Open   - the digitOUT is by default in HIGH state, LightON -> LOW state (false)
 */
bool relaysNC = true;

/* This value is set to:
 * LIGHTS - where every digitIN toggled changes the state of its corresponding digitOUT (1-to-1 mapping)
 * SHADES - the devices are coupled in 4 for one shade device (separate controls for UP and DOWN directions).
 *          Each digitIN enables the corresponding digitOUT, but at the same time disables the paired digitOUT. 
 *          Additional default timer is implemented.
 */
byte funcMode = FUNC_SHADES;

/* MAC address used for initiall boot */
byte mac[] = { 0x00, 0xAA, 0xBB, 0xC6, 0xA5, 0x58 };

/*
 * ------------------------
 * --- Global Variables ---
 * ------------------------
 */

/* starting with not registered iot hub (former arduino) TODO: make this value stored on the EEPROM */
byte ardID = 0;
byte raspyID = 0;

/* holds information if this arduino is registered */
bool isRegistered = false;

/* holds the IP of the Raspy/iot-gw */
IPAddress iotGwIP;

#if defined(CONTROLLINO_MEGA)

byte shadeIDs[SHADES] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

#elif defined(CONTROLLINO_MAXI) 

/* the shadeID array */
byte shadeIDs[SHADES] = { 1, 2, 3, 4, 5, 6 };

#endif


/* initialize the shades */
Shade shades[SHADES];

/* value to control when to print the measurement report */
bool measure;
/*
 * -----------------
 * ---   Setup   ---
 * -----------------
 */
void setup() {

  /* initialize serial interface */
  Serial.begin(9600);
  Serial.println("Setup init!");

  /* initialize the shades */
  for (int i = 0; i < SHADES; i++) {
    shades[i].init(shadeIDs[i]);
  }

  /* initialize RTC, no need to do that, Controllino should survive 2 weeks on internal battery */
  Serial.println("initializing RTC clock... ");
  Controllino_RTC_init(0);
  //Controllino_SetTimeDate(7,4,11,19,11,00,45);
  
  /* get the registration data from EEPROM */
  isRegistered = (bool) EEPROM.read(EEPROM_IDX_REG);
  if (isRegistered) {
    Serial.print("Arduino registered with ardID: ");
    ardID = EEPROM.read(EEPROM_IDX_ARDID);
    Serial.println(ardID);
    Serial.print("Raspy IP: ");
    EEPROM.get(EEPROM_IDX_RASPYIP, iotGwIP);
    Serial.println(iotGwIP);
    Serial.print("RaspyID: ");
    raspyID = EEPROM.read(EEPROM_IDX_RASPYID);
    Serial.println(raspyID);
    ARiF.begin(VER_SHD_1, mac, iotGwIP, ardID, raspyID);
  } else {
    Serial.println("Not registered within any iot-gw");
    ARiF.begin(VER_SHD_1, mac);
  }

  /* uncomment below code to clear the registration bit in the the EEPROM manually */
  //EEPROM.write(EEPROM_IDX_REG, (byte) false);
  
  Serial.println("Setup complete!");
}

/*
 * -----------------
 * --- Main Loop ---
 * -----------------
 */
void loop() {

  /* execute every second */


  /* -- measurement code start -- */
  measure = false;
  TCCR1A = 0;
  TCCR1B = 1;
  uint16_t start = TCNT1;
  /* -- measurement code end -- */

for (int i = 0; i < SHADES; i++) {
  //byte pos = shades[i].update();
  if (shades[i].update() <= 100) {
    Serial.println("Sending 1 shade position");
    //ARiF.sendShadePosition(shadeIDs[i], shades[i].getCurrentPosition());
  }

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

  if (shades[i].justStoppedTilt()) {
    ARiF.sendShadeTilt(shadeIDs[i], shades[i].getTilt());
  }
  
  if (shades[i].justStopped()) {
    ARiF.sendShadeStop(shadeIDs[i]);
    ARiF.sendShadePosition(shadeIDs[i], shades[i].getCurrentPosition());
  }
  if (shades[i].justStartedDown()) {
    ARiF.sendShadeDown(shadeIDs[i]);
  }
  if (shades[i].justStartedUp()) {
    ARiF.sendShadeUp(shadeIDs[i]);
  }
  
}

byte ret = ARiF.update();
byte lastDevID;
switch (ret) {
  case U_CONNECTED:
    Serial.println("Connected back!");
    for (int i = 0; i < SHADES; i++) {
      if (shades[i].isSynced()) {
        ARiF.sendShadeSynced(shadeIDs[i]);
        ARiF.sendShadeTilt(shadeIDs[i], shades[i].getTilt());
        ARiF.sendShadePosition(shadeIDs[i], shades[i].getCurrentPosition());
      } else {
        ARiF.sendShadeUnsynced(shadeIDs[i]);
      }
    }
    break;
  case U_NOTHING:
    break;
  case CMD_REGISTER:
    Serial.println("Registered!");
    break;
  case CMD_SHADEUP:
    Serial.print("ShadeUP received for: ");
    Serial.println(ARiF.getLastDevID());
    lastDevID = ARiF.getLastDevID();
    for (int i = 0; i < SHADES; i++) {
      if (shades[i].getDevID() == lastDevID) shades[i].up();
    }
    break;
  case CMD_SHADEDOWN:
    Serial.print("ShadeDOWN received for: ");
    Serial.println(ARiF.getLastDevID());
    lastDevID = ARiF.getLastDevID();
    for (int i = 0; i < SHADES; i++) {
      if (shades[i].getDevID() == lastDevID) shades[i].down();
    }
    break;    
  case CMD_SHADEPOS:
    Serial.print("Shadepos received with value: ");
    Serial.println(ARiF.getLastShadePosition());
    lastDevID = ARiF.getLastDevID();
    //Serial.println(s.getDevID()); // why s.toPosition() doesn't work??
    for (int i = 0; i < SHADES; i++) {
      if (shades[i].getDevID() == lastDevID) shades[i].toPosition(ARiF.getLastShadePosition());
    }
    break;
  case CMD_SHADETILT:
    Serial.print("Shadetilt received: ");
    Serial.println(ARiF.getLastShadeTilt());
    lastDevID = ARiF.getLastDevID();
    //Serial.println(s.getDevID()); // why s.toPosition() doesn't work??
    for (int i = 0; i < SHADES; i++) {
      if (shades[i].getDevID() == lastDevID) shades[i].setTilt(ARiF.getLastShadeTilt());
    }
    break;
  case CMD_SHADESTOP:
    Serial.print("ShadeSTOP received: ");
    lastDevID = ARiF.getLastDevID();
    //Serial.println(s.getDevID()); // why s.toPosition() doesn't work??
    for (int i = 0; i < SHADES; i++) {
      if (shades[i].getDevID() == lastDevID) {
        shades[i].stopWithTilt();
      }
    }
    break;  
  case CMD_LIGHTON:
    Serial.print("Received lightON command from: ");
    Serial.print(ARiF.getLastDevID());
    break;
  case CMD_LIGHTOFF:
    Serial.print("Received lightOFF command from: ");
    Serial.print(ARiF.getLastDevID());
    break;
  case CMD_UNKNOWN:
    Serial.print("Received unknown command from: ");
    Serial.print(ARiF.getLastDevID());
    break;
    
}

/*
  if (funcMode == FUNC_LIGHTS) {
    for (int i = 0; i < IN_PINS; i++) { 
      digitINState[i] = digitalRead(digitIN[i]);
      if (digitINState[i] == HIGH) {
        digitINPressed[i] = true;
        delay(10); // this delay here was placed in order for the press button result to be predictable
      } else {
        if (digitINPressed[i]) {
          /* EXECUTED ON BUTTON RELEASE - START */
 /*         if (digitOUTState[i] == high) {
            digitalWrite(digitOUT[i], low);
            sendDeviceStatus(digitOUTdevID[i], false);
            digitOUTState[i] = low;
          } else {
            digitalWrite(digitOUT[i], high);
            sendDeviceStatus(digitOUTdevID[i], true);
            digitOUTState[i] = high;
          }
          /* EXECUTED ON BUTTON RELEASE - END */
 /*       }
        digitINPressed[i] = false;
      }
    }
  } */

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
 * -----------------
 * --- Functions ---
 * -----------------
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
    EEPROM.put(EEPROM_IDX_RASPYIP, iotGwIP);
    Serial.println("IP changed. Writing to EEPROM");
    return false;
  }
}
