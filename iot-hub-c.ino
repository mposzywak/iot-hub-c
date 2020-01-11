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

#define ARiF_HTTP_PORT 32302
#define ARiF_HTTP_BUFF 150
#define ARiF_BEACON_INT 5
#define ARiF_BEACON_LENGTH 15
#define ARiF_BEACON_PORT 5007
#define ARiF_BEACON_STRING "/smarthouse/%d"

#define HTTP_200_OK "HTTP/1.1 200 OK\nContent-Type: text/html\nConnnection: close\n\n"
#define HTTP_500_Error "HTTP/1.1 500\nContent-Type: text/html\nConnnection: close\n\n"

/* getValue() codes - i. e. what do we want the function to return */
#define DEVID   0
#define ARDID   1
#define RASPYID 2
#define CMD     3

/* ARiF messages different CMD codes */
#define CMD_REGISTER  0
#define CMD_HEARTBEAT 1
#define CMD_LIGHTON   2
#define CMD_LIGHTOFF  3
#define CMD_UNKNOWN   10

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

/* UDP socket object for sending beacons */
EthernetUDP UDP;

/* ARiF REST HTTP Server */
EthernetServer ARiFServer(ARiF_HTTP_PORT);

/* AriF REST HTTP Client */
EthernetClient ARiFClient;

/* variable to check if beacon was already sent this second */
bool beaconSent = false;

/* ARiF multicast IP for sending beacons */
IPAddress mip(224, 1, 1, 1);

/* starting with not registered iot hub (former arduino) TODO: make this value stored on the EEPROM */
byte ardID = 0;
byte raspyID = 0;

/* holds information if this arduino is registered */
bool isRegistered = false;

/* holds the IP of the Raspy/iot-gw */
IPAddress iotGwIP;

/* stores value how many seconds ago last heartbeat was received */
byte lastHeartbeat = 0;

/* holds information if the iot-hub is connected to the iot-gw */
bool isConnected = false;

/* variable to indicate the DHCP failure at the boot. If DHCP failed
 *  at boot then there is no sense in re-trying every 1 sec as it will block
 *  the Hub for at least one second rendering it unresponsive. 
 *  -- Workaround: reboot the Hub once Connctivity/DHCP server is fixed. */
bool DHCPFailed = false;

/* variable used to track if everySec() has been already executed this second */
byte oldSec = 0;

/* default shade timer (in seconds) */
byte shadeTimer = 10;

#if defined(CONTROLLINO_MEGA)
/* The input pin array */
byte digitIN[IN_PINS] =   { CONTROLLINO_A0, 
                            CONTROLLINO_A1, 
                            CONTROLLINO_A2,
                            CONTROLLINO_A3,
                            CONTROLLINO_A4,
                            CONTROLLINO_A5,
                            CONTROLLINO_A6,
                            CONTROLLINO_A7,
                            CONTROLLINO_A8,
                            CONTROLLINO_A9,
                            CONTROLLINO_A10,
                            CONTROLLINO_A11,
                            CONTROLLINO_A12,
                            CONTROLLINO_A13,
                            CONTROLLINO_A14,
                            CONTROLLINO_A15,
                            CONTROLLINO_I16,
                            CONTROLLINO_I17,
                            CONTROLLINO_I18,
                            CONTROLLINO_IN0,
                            CONTROLLINO_IN1};

/* The output pin array */
byte digitOUT[OUT_PINS] = { CONTROLLINO_D0, 
                            CONTROLLINO_D1,
                            CONTROLLINO_D2,
                            CONTROLLINO_D3,
                            CONTROLLINO_D4,
                            CONTROLLINO_D5,
                            CONTROLLINO_D6,
                            CONTROLLINO_D7,
                            CONTROLLINO_D8,
                            CONTROLLINO_D9,
                            CONTROLLINO_D10,
                            CONTROLLINO_D11,
                            CONTROLLINO_D12,
                            CONTROLLINO_D13,
                            CONTROLLINO_D14,
                            CONTROLLINO_D15,
                            CONTROLLINO_D16,
                            CONTROLLINO_D17,
                            CONTROLLINO_D18,
                            CONTROLLINO_D19,
                            CONTROLLINO_D20};

/* The input pin devID array */
byte digitINdevID[IN_PINS] = { 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30 };

/* the output pin devID array */
byte digitOUTdevID[OUT_PINS] = { 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70 };

/* the shadeID array */
byte shadeIDs[SHADES] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

#elif defined(CONTROLLINO_MAXI) 
byte digitIN[IN_PINS] =   { CONTROLLINO_A0, 
                            CONTROLLINO_A1, 
                            CONTROLLINO_A2,
                            CONTROLLINO_A3,
                            CONTROLLINO_A4,
                            CONTROLLINO_A5,
                            CONTROLLINO_A6,
                            CONTROLLINO_A7,
                            CONTROLLINO_A8,
                            CONTROLLINO_A9,
                            CONTROLLINO_IN0,
                            CONTROLLINO_IN1};

/* The output pin array */
byte digitOUT[OUT_PINS] = { CONTROLLINO_D0, 
                            CONTROLLINO_D1,
                            CONTROLLINO_D2,
                            CONTROLLINO_D3,
                            CONTROLLINO_D4,
                            CONTROLLINO_D5,
                            CONTROLLINO_D6,
                            CONTROLLINO_D7,
                            CONTROLLINO_D8,
                            CONTROLLINO_D9,
                            CONTROLLINO_D10,
                            CONTROLLINO_D11};
                            
/* The input pin devID array */
byte digitINdevID[IN_PINS] = { 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21 };

/* the output pin devID array */
byte digitOUTdevID[OUT_PINS] = { 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61 };

/* the shadeID array */
byte shadeIDs[SHADES] = { 1, 2, 3, 4, 5, 6 };

#endif

/* variables representing the states */
byte low  = LOW;
byte high = HIGH;

/* pins states variables */
byte digitINState[IN_PINS];
bool digitINPressed[IN_PINS];
byte digitOUTState[OUT_PINS];

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
  
  oldSec = Controllino_GetSecond() - 1;
  Serial.println("Setup complete!");
}

/*
 * -----------------
 * --- Main Loop ---
 * -----------------
 */
void loop() {

  /* execute every second */
  everySec();

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
    ARiF.sendShadePosition(shadeIDs[i], shades[i].getCurrentPosition());
  }
    
  if (shades[i].isUpPressed()) {
    if (shades[i].isMoving()) {
      shades[i].stop();
    } else {
      shades[i].up();
    }
    measure = true;
  }

  if (shades[i].isDownPressed()) {
    if (shades[i].isMoving()) {
      shades[i].stop();
    } else {
      shades[i].down();
    }
    measure = true;
  }
  
  if (shades[i].justStopped()) {
    ARiF.sendShadeStop(shadeIDs[i]);
  }
  if (shades[i].justStartedDown()) {
    ARiF.sendShadeDown(shadeIDs[i]);
  }
  if (shades[i].justStartedUp()) {
    ARiF.sendShadeUp(shadeIDs[i]);
  }
  
}

switch (ARiF.update()) {
  case U_NOTHING:
    break;
  case CMD_REGISTER:
    Serial.println("Registered!");
    break;
  case CMD_SHADEPOS:
    Serial.print("Shadepos received with value: ");
    Serial.println(ARiF.getLastShadePosition());
    shades[0].toPosition(ARiF.getLastShadePosition());
    break;
  case CMD_SHADETILT:
    Serial.println("Shadetilt received!");
    break;
  case U_CONNECTED:
    Serial.println("Connected back!");
    break;
  case CMD_LIGHTON:
    Serial.print("Received lightON command from: ");
    Serial.print(ARiF.getLastDevID());
    break;
  case CMD_LIGHTOFF:
    Serial.print("Received lightOFF command from: ");
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

  /* handle here all Beacon operations and decisions */
  //sendBeacon();

  /* handle the incoming HTTP message over ARiF here */
  /*EthernetClient client = ARiFServer.available();
  if (client) {
    Serial.print("HTTP Request received from: ");
    Serial.println(client.remoteIP());
    
    handleARiFClient(client);
    measure = true;
    
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
 * -----------------
 * --- Functions ---
 * -----------------
 */


/* 
 *  Handle the incoming HTTP connection.
 */
char* handleARiFClient(EthernetClient cl) {
  char buff[ARiF_HTTP_BUFF];
  int index = 0;
  byte devID;
  while (cl.available()) {
    char c = cl.read();
    //Serial.print(c);
    buff[index] = c;
    index++;
    if (c == '/n' or c == '/r' or index >= ARiF_HTTP_BUFF) break;
  }
  switch (getValue(buff, CMD)) {
    case CMD_HEARTBEAT:
      checkIotGwIP(cl.remoteIP());
      cl.println(F(HTTP_200_OK)); /* write 200 OK */
      cl.stop();                  /* send */
      lastHeartbeat = 0;          /* reset the heartbeat timer */
      if (!isConnected) {
        isConnected = true;
        sendDeviceStatusAll();
      }
      break;
    case CMD_REGISTER:
      Serial.println("register received");
      //if (!isRegistered) { // this is commented out to allow the hub to "over-register"
        ardID = getValue(buff, ARDID);
        EEPROM.write(EEPROM_IDX_ARDID, ardID);
        raspyID = getValue(buff, RASPYID);
        EEPROM.write(EEPROM_IDX_RASPYID, raspyID);
        iotGwIP = cl.remoteIP();
        EEPROM.put(EEPROM_IDX_RASPYIP, iotGwIP);
        isRegistered = true;
        EEPROM.write(EEPROM_IDX_REG, (byte) true);
        cl.println(F(HTTP_200_OK));
        cl.println();
        cl.stop();
      //}
      break;
    case CMD_LIGHTON:
        devID = getValue(buff, DEVID);
        digitalWrite(digitOUT[getDigitOUTFromDevID(devID)], high);
        cl.println(F(HTTP_200_OK));
        cl.println();
        cl.stop();
        sendDeviceStatus(devID, true);
        digitOUTState[getDigitOUTFromDevID(devID)] = high;
      break;
    case CMD_LIGHTOFF:
        devID = getValue(buff, DEVID);
        digitalWrite(digitOUT[getDigitOUTFromDevID(devID)], low);
        cl.println(F(HTTP_200_OK));
        cl.println();
        cl.stop();
        sendDeviceStatus(devID, false);
        digitOUTState[getDigitOUTFromDevID(devID)] = low;
      break;
    case CMD_UNKNOWN:
        cl.println(F(HTTP_500_Error));
        cl.stop();
      break;
  }

  
  //Serial.println(buff);
  return buff;
}

/* handle beacon sending and decision if send */
void sendBeacon() {
  char beacon[ARiF_BEACON_LENGTH];
  if (lastHeartbeat > 4) {
    int sec = Controllino_GetSecond();
    if (sec % ARiF_BEACON_INT == 0) {
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
    } else {
     beaconSent = false;
    }
  }
}

/* get single value from the ARiF URL */
int getValue(char *buff, int value) {
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
  if (value == CMD ) {
    if (strstr(buff, "cmd=register")) return CMD_REGISTER;
    if (strstr(buff, "cmd=heartbeat")) return CMD_HEARTBEAT;
    if (strstr(buff, "cmd=lightON")) return CMD_LIGHTON;
    if (strstr(buff, "cmd=lightOFF")) return CMD_LIGHTOFF;
    return CMD_UNKNOWN;
  }
}

/* execute code every second */
void everySec() {
  int sec = Controllino_GetSecond();
  byte DHCPResult;
  //Serial.println(sec);
  //Serial.println(oldSec);
  if (sec != oldSec) {
    oldSec = sec;
    /* CODE EXECUTED EVERY SECOND - START */
    lastHeartbeat++;
    if (!DHCPFailed) {
      DHCPResult = Ethernet.maintain(); // call this func once per sec for DHCP lease renewal
      if (DHCPResult == 1 || DHCPResult == 3)
        DHCPFailed = true;
    }
    /* CODE EXECUTED EVERY SECOND - END */
  }
}

/* send device status to the iot-gw */
void sendDeviceStatus(byte devID, bool devStatus) {
  if (!isConnected) return;  // exit function if the link is dead;
  if (ARiFClient.connect(iotGwIP, ARiF_HTTP_PORT)) {
    Serial.print("connected to "); // to be removed 
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
    ARiFClient.print(F("&cmd=status&devType=digitOUT&dataType=bool&value="));
    if (devStatus) {
      ARiFClient.print("1\n");
    } else {
      ARiFClient.print("0\n");
    }
    ARiFClient.println("Host: raspy");
    ARiFClient.println("Connection: close");
    ARiFClient.println();
  } else {
    Serial.println("Problem with connecting");
  }
}

/* send Status of all digitOUT pins */
void sendDeviceStatusAll() {
  for (int i = 0; i < OUT_PINS; i++) {
    sendDeviceStatus(digitOUTdevID[i], digitOUTState[i]);
  }
}

/* function to get the digitIN index based on devID */
byte getDigitINFromDevID(byte devID) {
  for (int i = 0; i < IN_PINS; i++) {
    if (digitINdevID[i] == devID) {
      return i;
    }
  }
}

/* function to get the digitOUT index based on devID */
byte getDigitOUTFromDevID(byte devID) {
  for (int i = 0; i < OUT_PINS; i++) {
    if (digitOUTdevID[i] == devID) {
      return i;
    }
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
