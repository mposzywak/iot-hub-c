#include <SPI.h>
#include <string.h>
#include <Console.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <avr/pgmspace.h>
#include <Controllino.h>
#include <EEPROM.h>

/*
  CONTROLLINO - smarthouse test, Version 01.00

  Used to control outputs by inputs and provide a REST API like over network to that system.
  
  Created 8 Oct 2019
  by Maciej

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

#define EEPROM_IDX_ARDID    0  // length 1
#define EEPROM_IDX_RASPYID  1  // length 1
#define EEPROM_IDX_REG      2  // length 1
#define EEPROM_IDX_RASPYIP  3  // length 6
#define EEPROM_IDX_NEXT     9

// various global variables
byte mac[] = {
  0x00, 0xAA, 0xBB, 0xC9, 0xD2, 0x49
};

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

/* holds information if the raspy/iot-gw timed out on heartbeat */
bool heartbeatTimedOut = true;

/* holds the IP of the Raspy/iot-gw */
IPAddress iotGwIP;

/* stores value how many seconds ago last heartbeat was received */
byte lastHeartbeat = 0;

/* variable used to track if everySec() has been already executed this second */
//bool everySec = false;
byte oldSec = 0;

// the setup function runs once when you press reset (CONTROLLINO RST button) or connect the CONTROLLINO to the PC
void setup() {
  // initialize all used digital output pins as outputs
  pinMode(CONTROLLINO_D0, OUTPUT);
  pinMode(CONTROLLINO_D1, OUTPUT);  // note that we are using CONTROLLINO aliases for the digital outputs
  pinMode(CONTROLLINO_D2, OUTPUT);
  pinMode(CONTROLLINO_D3, OUTPUT);  // the alias is always like CONTROLLINO_
  pinMode(CONTROLLINO_D4, OUTPUT);  // and the digital output label as you can see at the CONTROLLINO device
  pinMode(CONTROLLINO_D5, OUTPUT);  // next to the digital output screw terminal
  pinMode(CONTROLLINO_D6, OUTPUT);
  pinMode(CONTROLLINO_D7, OUTPUT);
  pinMode(CONTROLLINO_A0, INPUT);
  Serial.begin(9600);
  while (!Serial);
  Serial.flush();

  Serial.println("Setup init!");
  if (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable disconnected");
  }
  
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println(F("Ethernet shield was not found."));
    } else if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println(F("Ethernet cable is not connected."));
    }
  } else { /* Ethernet initialization succesfull */
    Serial.println(F("NIC initialized. Ethernet cable connected."));
    Serial.print("My IP address: ");
    Serial.println(Ethernet.localIP());
    
    UDP.beginMulticast(mip, ARiF_BEACON_PORT);
    ARiFServer.begin();
  }
  Serial.println("initializing RTC clock... ");
  Controllino_RTC_init(0);
  //Controllino_SetTimeDate(16,2,10,19,23,54,45);

  UDP.beginMulticast(mip, ARiF_BEACON_PORT);
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
  } else {
    Serial.println("Arduino not registered before.");
  }

  /* uncomment below code to clear the registration bit in the the EEPROM manually */
  //EEPROM.write(EEPROM_IDX_REG, (byte) false);
  
  oldSec = Controllino_GetSecond() - 1;
  Serial.println("Setup complete!");
}

int A0_state = 0;
int A0_pressed = 0;
int D0_state = 0;
bool measure;
// the loop function runs over and over again forever
void loop() {

  /* execute every second */
  everySec();
  
  measure = false;
  TCCR1A = 0;
  TCCR1B = 1;

  uint16_t start = TCNT1;
  A0_state = digitalRead(CONTROLLINO_A0);
  if (A0_state == 1) {
    A0_pressed = 1;
    delay(10); // this delay here was placed in order for the press button result to be predictable
  } else {
    if (A0_pressed) {
      if (D0_state) {
        digitalWrite(CONTROLLINO_D0, LOW);
        sendDeviceStatus(1, false);
        D0_state = 0;
      } else {
        digitalWrite(CONTROLLINO_D0, HIGH);
        sendDeviceStatus(1, true);
        measure = true;
        D0_state = 1;
      }
    }
    A0_pressed = 0;
  }

  /* handle here all Beacon operations and decisions */
  sendBeacon();

  /* handle the incoming HTTP message over ARiF here */
  EthernetClient client = ARiFServer.available();
  if (client) {
    Serial.print("HTTP Request received from: ");
    Serial.println(client.remoteIP());
    
    handleARiFClient(client);
    measure = true;
    
  }
  uint16_t finish = TCNT1;
  uint16_t overhead = 8;
  uint16_t cycles = finish - start - overhead;
  if (measure) {
    Serial.print("cycles: ");
    Serial.println(cycles);
  }
}

char* handleARiFClient(EthernetClient cl) {
  char buff[ARiF_HTTP_BUFF];
  int index = 0;
  while (cl.available()) {
    char c = cl.read();
    //Serial.print(c);
    buff[index] = c;
    index++;
    if (c == '/n' or c == '/r' or index >= ARiF_HTTP_BUFF) break;
  }
  switch (getValue(buff, CMD)) {
    case CMD_HEARTBEAT:
      cl.println(F(HTTP_200_OK)); /* write 200 OK */
      cl.stop();                  /* send */
      lastHeartbeat = 0;          /* reset the heartbeat timer */
      break;
    case CMD_REGISTER:
      Serial.println("register received");
      //if (!isRegistered) {
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
        digitalWrite(CONTROLLINO_D0, HIGH);
        cl.println(F(HTTP_200_OK));
        cl.println();
        cl.stop();
        sendDeviceStatus(1, true);
        D0_state = 1;
      break;
    case CMD_LIGHTOFF:
        digitalWrite(CONTROLLINO_D0, LOW);
        cl.println(F(HTTP_200_OK));
        cl.println();
        cl.stop();
        sendDeviceStatus(1, false);
        D0_state = 0;
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
  //Serial.println(sec);
  //Serial.println(oldSec);
  if (sec != oldSec) {
    oldSec = sec;
    lastHeartbeat++;
  }
}

/* send device status to the iotGW */
void sendDeviceStatus(int devID, bool devStatus) {
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
    Serial.print("Problem with connecting");
  }
}


/* End of the example. Visit us at https://controllino.biz/ or contact us at info@controllino.biz if you have any questions or troubles. */

/* 2016-12-13: The sketch was successfully tested with Arduino 1.6.13, Controllino Library 1.0.0 and CONTROLLINO MINI, MAXI and MEGA. */
