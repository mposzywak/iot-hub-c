
#include "Settings.h"

/* ----- Platform related defines - START ----- */

#if defined(CONTROLLINO_MEGA)

#include <Controllino.h>

#elif defined(CONTROLLINO_MAXI)

#include <Controllino.h>

#elif defined(CONTROLLINO_MAXI_AUTOMATION)

#include <Controllino.h>

#elif defined(ARDUINO_AVR_MEGA2560)
/* nothing needed here */

#else 
  #error "Unsupported board"
#endif

/* ----- Platform related defines - END ----- */

#if defined(CONTROLLINO_MEGA)
/* The input pin array */
    static byte Settings::digitIN[IN_PINS] =   { CONTROLLINO_A0, 
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
                                                 CONTROLLINO_IN1 };

    /* The output pin array */
    static byte Settings::digitOUT[OUT_PINS] = { CONTROLLINO_D0, 
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
                                                 CONTROLLINO_D20 };

    /* The input pin devID array */
    //static byte Settings::digitINdevID[IN_PINS] = { 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30 };

    /* the output pin devID array */
    //static byte Settings::digitOUTdevID[OUT_PINS] = { 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70 };

    /* the shadeID array (numbers must be consecutive) */
    static byte Settings::shadeIDs[SHADES] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

    /* the lightID array (numbers must be consecutive) */
    static byte Settings::lightIDs[LIGHTS] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21 };

    /* 1-wire pin number */
    static byte Settings::oneWirePin = 20;

#elif defined(CONTROLLINO_MAXI) 
    static byte Settings::digitIN[IN_PINS] =   { CONTROLLINO_A0, 
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
                                                 CONTROLLINO_IN1 };

    /* The output pin array */
    static byte Settings::digitOUT[OUT_PINS] = { CONTROLLINO_D0, 
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
                                                 CONTROLLINO_D11 };
                            
    /* The input pin devID array */
    //static byte Settings::digitINdevID[IN_PINS] = { 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21 };

    /* the output pin devID array */
    //static byte Settings::digitOUTdevID[OUT_PINS] = { 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61 };

    /* the shadeID array (numbers must be consecutive) */
    static byte Settings::shadeIDs[SHADES] = { 1, 2, 3, 4, 5, 6 };

    /* the lightID array (numbers must be consecutive) */
    static byte Settings::lightIDs[LIGHTS] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12 };

    /* 1-wire pin number */
    static byte Settings::oneWirePin = 20;

#elif defined(CONTROLLINO_MAXI_AUTOMATION) 
    static byte Settings::digitIN[IN_PINS] =   { CONTROLLINO_A0, 
                                                 CONTROLLINO_A1, 
                                                 CONTROLLINO_A2,
                                                 CONTROLLINO_A3,
                                                 CONTROLLINO_A4,
                                                 CONTROLLINO_A5,
                                                 CONTROLLINO_A6,
                                                 CONTROLLINO_A7 };

    /* The output pin array */
    static byte Settings::digitOUT[OUT_PINS] = { CONTROLLINO_D0, 
                                                 CONTROLLINO_D1,
                                                 CONTROLLINO_D2,
                                                 CONTROLLINO_D3,
                                                 CONTROLLINO_D4,
                                                 CONTROLLINO_D5,
                                                 CONTROLLINO_D6,
                                                 CONTROLLINO_D7 };

    /* the shadeID array (numbers must be consecutive) */
    static byte Settings::shadeIDs[SHADES] = { 1, 2, 3, 4 };

    /* the lightID array (numbers must be consecutive) */
    static byte Settings::lightIDs[LIGHTS] = { 1, 2, 3, 4, 5, 6, 7, 8 };

    /* 1-wire pin number */
    static byte Settings::oneWirePin = 20;
    
#elif defined(ARDUINO_AVR_MEGA2560)
    static byte Settings::digitIN[IN_PINS] =   { 3, 
                                                 5,
                                                 6,
                                                 7,
                                                 8,
                                                 9,
                                                 14,
                                                 15,
                                                 16,
                                                 17,
                                                 18,
                                                 19,
                                                 20,
                                                 21,
                                                 22,
                                                 23,
                                                 24,
                                                 25,
                                                 26,
                                                 27,
                                                 28,
                                                 29,
                                                 30,
                                                 31,
                                                 32,
                                                 33,
                                                 34 };

    /* The output pin array */
    static byte Settings::digitOUT[OUT_PINS] = { 38, 
                                                 39,
                                                 40,
                                                 41,
                                                 42,
                                                 43,
                                                 44,
                                                 45,
                                                 46,
                                                 47,
                                                 48,
                                                 49,
                                                 50,
                                                 51,
                                                 52,
                                                 53,
                                                 54,
                                                 55,
                                                 56,
                                                 57,
                                                 58,
                                                 59,
                                                 60,
                                                 61,
                                                 62,
                                                 63,
                                                 64 };
                            
    /* The input pin devID array */
    //static byte Settings::digitINdevID[IN_PINS] = { 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21 };

    /* the output pin devID array */
    //static byte Settings::digitOUTdevID[OUT_PINS] = { 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61 };

    /* the shadeID array (numbers must be consecutive) */
    static byte Settings::shadeIDs[SHADES] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14 };

    /* the lightID array (numbers must be consecutive) */
    static byte Settings::lightIDs[LIGHTS] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28 };

    /* 1-wire pin number */
    static byte Settings::oneWirePin = 2;
#else 
  #error "Unsupported board"
#endif

static bool Settings::getLow() {
  if (Settings::relaysNC)
    return LOW;
  else
    return HIGH;
}

static bool Settings::getHigh() {
  if (Settings::relaysNC)
    return HIGH;
  else
    return LOW;
}

static byte Settings::getLastLightDevID() {
  return lightIDs[LIGHTS - 1];
}

static byte Settings::getLightOutPin(byte lightID) {
  return Settings::digitOUT[(lightID - 1)];
}

static byte Settings::getLightInPin(byte lightID) {
  return Settings::digitIN[(lightID - 1)];
}

static byte Settings::getShadeOutPinUp(byte shadeID) {
  return Settings::digitOUT[(shadeID - 1) * 2];
}

static byte Settings::getShadeInPinUp(byte shadeID) {
  return Settings::digitIN[(shadeID - 1) * 2];
}

static byte Settings::getShadeOutPinDown(byte shadeID) {
  return Settings::digitOUT[((shadeID - 1) * 2) + 1];
}

static byte Settings::getShadeInPinDown(byte shadeID) {
  return Settings::digitIN[((shadeID - 1) * 2) + 1];
}

static void Settings::setInPinMode(uint8_t pin) {
#if defined(CONTROLLINO_MEGA)
  pinMode(pin, INPUT);
#elif defined(CONTROLLINO_MAXI)
  pinMode(pin, INPUT);
#elif defined(CONTROLLINO_MAXI_AUTOMATION)
  pinMode(pin, INPUT);
#elif defined(ARDUINO_AVR_MEGA2560)
  pinMode(pin, INPUT_PULLUP);
#endif
}

static int Settings::getInputPinValue(uint8_t pin) {
#if defined(CONTROLLINO_MEGA)
  return digitalRead(pin);
#elif defined(CONTROLLINO_MAXI)
  return digitalRead(pin);
#elif defined(CONTROLLINO_MAXI_AUTOMATION)
  return digitalRead(pin);
#elif defined(ARDUINO_AVR_MEGA2560)
  int pinValue;
  pinValue = digitalRead(pin);
  if (pinValue == LOW) {
    return HIGH;
  } else {
    return LOW;
  }
#endif
}

static void Settings::setOutputPinValue(uint8_t pin, uint8_t value) {
#if defined(CONTROLLINO_MEGA)
  digitalWrite(pin, value);
#elif defined(CONTROLLINO_MAXI)
  digitalWrite(pin, value);
#elif defined(CONTROLLINO_MAXI_AUTOMATION)
  digitalWrite(pin, value);
#elif defined(ARDUINO_AVR_MEGA2560)
  if (value == HIGH) {
    digitalWrite(pin, LOW);
  } else {
    digitalWrite(pin, HIGH);
  }
#endif
}

static void Settings::initPlatform() {
#if defined(CONTROLLINO_MEGA)
  Controllino_RTC_init(0);
  //Controllino_SetTimeDate(7,4,11,19,11,00,45);
#elif defined(CONTROLLINO_MAXI) 
  Controllino_RTC_init(0);
#elif defined(CONTROLLINO_MAXI_AUTOMATION)
  Controllino_RTC_init(0);
#elif defined(ARDUINO_AVR_MEGA2560)
  /* nothing needed here */
#endif
}

static bool Settings::EEPROMIsRegistered() {
   return (bool) EEPROM.read(EEPROM_IDX_REG);
}

static IPAddress Settings::EEPROMGetRaspyIP( ) {
  byte address[4];
  EEPROM.get(EEPROM_IDX_RASPYIP, address);
  return address;
}

static byte Settings::EEPROMGetRaspyID() {
  return EEPROM.read(EEPROM_IDX_RASPYID);
}

static byte Settings::EEPROMGetArdID() {
  return EEPROM.read(EEPROM_IDX_ARDID);
}

static void Settings::EEPROMDeregister() {
  EEPROM.write(EEPROM_IDX_REG, (byte) false);
}

static void Settings::EEPROMSetRaspyIP(IPAddress addr) {
  EEPROM.put(EEPROM_IDX_RASPYIP, addr);
}

static void Settings::EEPROMRegister(byte ardID, byte raspyID, IPAddress addr) {
  EEPROM.write(EEPROM_IDX_REG, (byte) true);
  EEPROM.write(EEPROM_IDX_ARDID, ardID);
  EEPROM.write(EEPROM_IDX_RASPYID, raspyID);
  EEPROM.write(EEPROM_IDX_RASPYIP, addr[0]);
  EEPROM.write(EEPROM_IDX_RASPYIP + 1, addr[1]);
  EEPROM.write(EEPROM_IDX_RASPYIP + 2, addr[2]);
  EEPROM.write(EEPROM_IDX_RASPYIP + 3, addr[3]);
}

static byte Settings::EEPROMGetMode() {
  byte mode;
  mode = EEPROM.read(EEPROM_IDX_MODE);
  if (mode != MODE_LIGHTS && mode != MODE_SHADES) {
    mode = MODE_LIGHTS;
    EEPROM.write(EEPROM_IDX_MODE, mode);
    Serial.print(F("Wrong value set as MODE value in EEPROM"));
  }
  return mode;
}

static void Settings::EEPROMSetMode(byte mode) {
  EEPROM.write(EEPROM_IDX_MODE, mode);
}

/* The shadeIDs and lightIDs are used as index values for the entries in the EEPROM hence they must all start from 1 and be consecutive in order for the 
   EEPROM storage of individual configuration of devices to work properly */

/* The following scheme shows how the data is stored:
 *  xx xx xx xxxxxxxx xx
 *  |  |  |  |        |__ The ctrlON setting of the light (if it is subject to global ctrlON cmd) (byte)
 *  |  |  |  |____ The timer value in miliseconds: (unsigned long)
 *  |  |  |_______ The sub-type of light device: lightType (byte)
 *  |  |__________ The ON/OFF status of the light device: lightInputType (byte) 
 *  |_____________ Status of the light (wether it is on or off)
 */

static void Settings::EEPROMSetLightCtrlON(byte devID, byte value) {
  int index = EEPROM_IDX_LIGHTS + (EEPROM_IDX_LIGHTS_LENGTH * (devID - 1)); /* start address for Light data structure */
  EEPROM.write(index + 7, value);
}

static byte Settings::EEPROMGetLightCtrlON(byte devID) {
  int index = EEPROM_IDX_LIGHTS + (EEPROM_IDX_LIGHTS_LENGTH * (devID - 1)); /* start address for Light data structure */
  byte ctrlON;
  ctrlON = EEPROM.read(index + 7);
  return ctrlON;
}

static void Settings::EEPROMSetLightConfig(byte devID, byte type, unsigned long timer) {
  if (devID > LIGHTS) return;
  int index = EEPROM_IDX_LIGHTS + (EEPROM_IDX_LIGHTS_LENGTH * (devID - 1)); /* start address for Light data structure */
  EEPROM.write(index + 1, type);
  EEPROMWritelong(index + 2, timer);
}

static byte Settings::EEPROMGetLightType(byte devID) {
  int index = EEPROM_IDX_LIGHTS + (EEPROM_IDX_LIGHTS_LENGTH * (devID - 1)); /* start address for Light data structure */
  byte type;
  type = EEPROM.read(index + 1);
  return type;
}

static void Settings::EEPROMSetLightType(byte devID, byte type) {
  if (devID > LIGHTS) return;
  int index = EEPROM_IDX_LIGHTS + (EEPROM_IDX_LIGHTS_LENGTH * (devID - 1)); /* start address for Light data structure */
  EEPROM.write(index + 1, type);
}

static unsigned long Settings::EEPROMGetLightTimer(byte devID) {
  int index = EEPROM_IDX_LIGHTS + (EEPROM_IDX_LIGHTS_LENGTH * (devID - 1)); /* start address for Light data structure */
  unsigned long timer;
  timer = EEPROMReadlong(index + 2);
  return timer;
}

static void Settings::EEPROMSetLightTimer(byte devID, unsigned long timer) {
  if (devID > LIGHTS) return;
  int index = EEPROM_IDX_LIGHTS + (EEPROM_IDX_LIGHTS_LENGTH * (devID - 1)); /* start address for Light data structure */
  EEPROMWritelong(index + 2, timer);
}

static void Settings::EEPROMSetLightStatus(byte devID, byte status) {
  if (devID > LIGHTS) return;
  int index = EEPROM_IDX_LIGHTS + (EEPROM_IDX_LIGHTS_LENGTH * (devID - 1)); /* start address for Light data structure */
  EEPROM.write(index, status);
}

static byte Settings::EEPROMGetLightInputType(byte devID) {
  if (devID > LIGHTS) return;
  int index = EEPROM_IDX_LIGHTS + (EEPROM_IDX_LIGHTS_LENGTH * (devID - 1));
  byte inputType;
  inputType = EEPROM.read(index + 6);
  return inputType;
}

static void Settings::EEPROMSetLightInputType(byte devID, byte inputType) {
  if (devID > LIGHTS) return;
  int index = EEPROM_IDX_LIGHTS + (EEPROM_IDX_LIGHTS_LENGTH * (devID - 1));
  EEPROM.write(index + 6, inputType);
}


static unsigned long Settings::EEPROMReadlong(unsigned long address) {
  unsigned long value;
  long four = EEPROM.read(address);
  long three = EEPROM.read(address + 1);
  long two = EEPROM.read(address + 2);
  long one = EEPROM.read(address + 3);

  value = ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);

  return value;
}

static void Settings::EEPROMWritelong(int address, unsigned long value) {
  byte four = (value & 0xFF);
  byte three = ((value >> 8) & 0xFF);
  byte two = ((value >> 16) & 0xFF);
  byte one = ((value >> 24) & 0xFF);
 
  EEPROM.write(address, four);
  EEPROM.write(address + 1, three);
  EEPROM.write(address + 2, two);
  EEPROM.write(address + 3, one);
}

static int Settings::EEPROMReadInt(int address) {
  byte byte1 = EEPROM.read(address);
  byte byte2 = EEPROM.read(address + 1);
  return (byte1 << 8) + byte2;
}

static void Settings::EEPROMWriteInt(int address, int value) {
  byte byte1 = value >> 8;
  byte byte2 = value & 0xFF;
  EEPROM.write(address, byte1);
  EEPROM.write(address + 1, byte2);
}


static void Settings::EEPROMSetLightCentral(byte mode) {
  EEPROM.write(EEPROM_IDX_CENT_CTRL, mode);
}

static byte Settings::EEPROMGetLightCentral() {
  byte mode;
  mode = EEPROM.read(EEPROM_IDX_CENT_CTRL);
  return mode;
}

/* The shadeIDs and lightIDs are used as index values for the entries in the EEPROM hence they must all start from 1 and be consecutive in order for the 
   EEPROM storage of individual configuration of devices to work properly */

/* The following scheme shows how the data is stored (for a shade type device):
 *  xx xxxx xx xx xx xxxx
 *  |  |    |  |  |  |__________ The tile timer value (int)
 *  |  |    |  |  |__________ The position timer value (byte)
 *  |  |    |  |__________ The shade device type: (byte)
 *  |  |    |_________ flags field (byte) 
 *  |  |_______ The Tilt position of shade device: (int)
 *  |____ The Position of the shade (current state): (byte)
 */

static void Settings::EEPROMSetShadeType(byte devID, byte type) {
  if (devID > SHADES) return;
  int index = EEPROM_IDX_SHADES + (7 * (devID - 1));
  EEPROM.write(index + 4, type);
}

static void Settings::EEPROMSetShadeTiltTimer(byte devID, int timer) {
  if (devID > SHADES) return;
  int index = EEPROM_IDX_SHADES + (7 * (devID - 1));
  EEPROMWriteInt(index + 6, timer);
}

static void Settings::EEPROMSetShadePosTimer(byte devID, byte timer) {
  if (devID > SHADES) return;
  int index = EEPROM_IDX_SHADES + (7 * (devID - 1));
  EEPROM.write(index + 5, timer);
}

static byte Settings::EEPROMGetShadeType(byte devID) {
  int index = EEPROM_IDX_SHADES + (7 * (devID - 1));
  byte type;
  type = EEPROM.read(index + 4);
  return type;
}

static int Settings::EEPROMGetShadeTiltTimer(byte devID) {
  int index = EEPROM_IDX_SHADES + (7 * (devID - 1));
  int timer;
  timer = EEPROMReadInt(index + 6);
  return timer;
}

static byte Settings::EEPROMGetShadePosTimer(byte devID) {
  int index = EEPROM_IDX_SHADES + (7 * (devID - 1));
  byte timer;
  timer = EEPROM.read(index + 5);
  return timer;
}

static void Settings::EEPROMSetMAC(byte *mac) {
  for (byte i = 0; i < 6; i++) {
    EEPROM.write(EEPROM_IDX_MAC + i, mac[i]);
  }
}

static void Settings::EEPROMGetMAC(byte *mac) {
  for (byte i = 0; i < 6; i++) {
    mac[i] = EEPROM.read(EEPROM_IDX_MAC + i);
  }
}

static void Settings::EEPROMSetUseDefMAC(byte status) {
  EEPROM.write(EEPROM_IDX_USE_DEF_MAC, status);
}

static byte Settings::EEPROMGetUseDefMAC() {
  return EEPROM.read(EEPROM_IDX_USE_DEF_MAC);
}

static void Settings::EEPROMSetUID() {
  EEPROMWritelong(EEPROM_IDX_UID, EEPROM_UID);
}

static bool Settings::EEPROMIsUIDSet() {
  unsigned long uid;
  uid = EEPROMReadlong(EEPROM_IDX_UID);
  Serial.print("UID: ");
  Serial.println(uid);
  if (uid == EEPROM_UID) {
    return true;
  } else {
    return false;
  }
}

static void Settings::EEPROMRaze() {
  uint16_t len;
  len = EEPROM.length();
  for (int i = 0; i < len; i++) {
    if (i == EEPROM_IDX_ARDID) {
      EEPROM.write(EEPROM_IDX_ARDID, 0);
    } else if (i == EEPROM_IDX_REG) {
      EEPROM.write(EEPROM_IDX_REG, (byte) false);
    } else {
      EEPROM.write(i, 255);
    }
  }
}

static bool Settings::EEPROMClearUID() {
  unsigned long uid;
  uid = 0;
  EEPROMWritelong(EEPROM_IDX_UID, uid);
}

static byte Settings::SDCardInit() {
#if defined(CONTROLLINO_MEGA)
  /* no support on Controllino for SD Card */
#elif defined(CONTROLLINO_MAXI) 
  /* no support on Controllino for SD Card */
#elif defined(CONTROLLINO_MAXI_AUTOMATION)
  /* no support on Controllino for SD Card */
#elif defined(ARDUINO_AVR_MEGA2560)
  pinMode(SD_ARD_MEGA_CS, OUTPUT);     /* set the Select pin */
  digitalWrite(SD_ARD_MEGA_CS, LOW);   /* set the SD Card CS to LOW to for the time of SD initialization */
  delay(500);

  if (!SD.begin(SD_ARD_MEGA_CS))
  {
    Serial.println(F("Comm issue with SD Card ctrl. No SD Card available."));
    digitalWrite(SD_ARD_MEGA_CS, HIGH);
    return SD_INIT_NOHW;
  }
  if (!SD.exists("MAIN.CSS")) {
    Serial.println(F("ERROR - Can't find MAIN.CSS file!"));
    digitalWrite(SD_ARD_MEGA_CS, HIGH);
    return SD_INIT_NOFILE;
  } else {
    if (!SD.exists("MAIN.JS")) {
      Serial.println(F("ERROR - Can't find MAIN.JS file!"));
      digitalWrite(SD_ARD_MEGA_CS, HIGH);
      return SD_INIT_NOFILE;
    } else {
      Serial.println(F("SUCCESS - All files present!"));
      digitalWrite(SD_ARD_MEGA_CS, HIGH);
      return SD_INIT_SUCCESS;
    }
  }
#endif
}

static File Settings::SDCardFileOpen(char *filename) {
  File file;
  file = SD.open(filename);
  return file;
}

static void Settings::SDCardFileClose(File file) {
  file.close();
}

static byte Settings::getOneWirePin() {
  return ONE_WIRE_PIN;
}
