
#include <Arduino.h>
#include <EEPROM.h>
#include <Ethernet.h>
#include <SPI.h>
#include <SD.h>

#ifndef SETTINGS_H_
#define SETTINGS_H_

#if defined(CONTROLLINO_MEGA)
#include <Controllino.h>
#define IN_PINS  21
#define OUT_PINS 21
#define SHADES   10
#define LIGHTS   21
#elif defined(CONTROLLINO_MAXI)
#include <Controllino.h>
#define IN_PINS  12
#define OUT_PINS 12
#define SHADES   6
#define LIGHTS   12
#elif defined(ARDUINO_AVR_MEGA2560)
#define IN_PINS  28
#define OUT_PINS 28
#define SHADES   14
#define LIGHTS   28
#endif

/* types of physical button press */
#define PHY_NO_PRESS                          0
#define PHY_MOMENTARY_PRESS                   1
#define PHY_PRESS_MORE_THAN_2SEC              2
#define PHY_CENTRAL_CTRL_MOMENTARY_PRESS      3
#define PHY_CENTRAL_CTRL_PRESS_MORE_THAN_2SEC 4

/* indexes for EEPROM information holding */
#define EEPROM_IDX_ARDID    0  // length 1
#define EEPROM_IDX_RASPYID  1  // length 1
#define EEPROM_IDX_REG      2  // length 1
#define EEPROM_IDX_RASPYIP  3  // length 6
#define EEPROM_IDX_MODE     9  // length 1
#define EEPROM_IDX_LIGHTS   10 // length 210 -> 30 (lights) x 6 (1 status + 1 type + 4 timer)  + 30 (buffer for future use)
#define EEPROM_IDX_CENT_CTRL 220 // length
#define EEPROM_IDX_SHADES   221 // length 150 -> 15 (shades) x 8 (4 status + 1 type + 1 pos timer + 2 tilt timer) + 30 (buffer for future use)

/* flags */
#define EEPROM_FLG_SHADE_SYNC     1
#define EEPROM_FLG_SHADE_INMOTION 2

/* functional modes of the entire device */
#define MODE_LIGHTS 0
#define MODE_SHADES 1
#define MODE_FAIL   100 /* returned by EEPROMGetMode() if couldn't load mode from EEPROM */


/* SD Card variables */
#define SD_ARD_MEGA_CS  4

#define SD_INIT_NOHW        0
#define SD_INIT_SUCCESS     1
#define SD_INIT_NOFILE      2



class Settings {
  private:

    /* The input pin array */
    static byte digitIN[IN_PINS];

    /* The output pin array */
    static byte digitOUT[OUT_PINS];

    /* read a long value from EEPROM's given address */
    static unsigned long EEPROMReadlong(unsigned long address);

    /* write a long vlaue into EEPROM at a given address */
    static void EEPROMWritelong(int address, unsigned long value);

    /* read an int value from EEPROM's given address */
    static int Settings::EEPROMReadInt(int address);

    /* write an int vlaue into EEPROM at a given address */
    static void Settings::EEPROMWriteInt(int address, int value);

  public:

    /* The input pin devID array */
    static byte digitINdevID[IN_PINS];

    /* the output pin devID array */
    static byte digitOUTdevID[OUT_PINS];

    /* the shadeID array */
    static byte shadeIDs[SHADES];

    /* the LightID array */
    static byte lightIDs[LIGHTS];

    /* variable sets how much time the shade motor should be running (in seconds) */
    static const byte runTimer = 10;

    /* variable controlling if the relays are set to NC or NO
     *  NC - Normally Closed - the digitOUT is by default in LOW state, LightON -> HIGH state (true)
     *  NO - Normally Open   - the digitOUT is by default in HIGH state, LightON -> LOW state (false)
     */
    static const bool relaysNC = true;
    static bool getLow();
    static bool getHigh();

    /* Get the Shade PINs based on the shade ID */
    static byte getShadeOutPinUp(byte shadeID);
    static byte getShadeInPinUp(byte shadeID);
    static byte getShadeOutPinDown(byte shadeID);
    static byte getShadeInPinDown(byte shadeID);

    /* Get the Light PINs based on the shade ID */
    static byte Settings::getLightOutPin(byte lightID);
    static byte Settings::getLightInPin(byte lightID);

    /* set input pin mode (platform independent) */
    static void Settings::setInPinMode(uint8_t pin);

    /* get input pin value (platform independent) */
    static int Settings::getInputPinValue(uint8_t pin);

    /* set output pin value (platform independent) */
    static void Settings::setOutputPinValue(uint8_t pin, uint8_t value);

    /* initialize the platform */
    static void Settings::initPlatform();

    /*
     * EEPROM Storage related functions
     */

    /* Get from the EEPROM the ARiF registration status*/
    static bool Settings::EEPROMIsRegistered();

    /* Get from the EEPROM the ardID value*/
    static byte EEPROMGetArdID();

    /* Get from the EEPROM the raspy value*/
    static byte Settings::EEPROMGetRaspyID();

    /* Get from the EEPROM the IP address of the raspy where this arduino is registered */
    static IPAddress Settings::EEPROMGetRaspyIP();

    /* Write 'deregister' value into the EEPROM */
    static void Settings::EEPROMDeregister();

    /* Write Raspy IP address into the EEPROM */
    static void Settings::EEPROMSetRaspyIP(IPAddress addr);

    /* Write all registration data into the EEPROM */
    static void Settings::EEPROMRegister(byte ardID, byte raspyID, IPAddress addr);

    /* Write System mode into the EEPROM */
    static void Settings::EEPROMSetMode(byte mode);
    
    /* Get from the EEPROM System mode */
    static byte Settings::EEPROMGetMode();

    /* Write individual Light settings in the EEPROM */
    static void EEPROMSetLightConfig(byte devID, byte type, unsigned long timer);

    /* Get from the EEPROM Light type */
    static byte EEPROMGetLightType(byte devID);

    /* Write light type into the EEPROM */
    static void EEPROMSetLightType(byte devID, byte type);

    /* Get from the EEPROM Light timer */
    static unsigned long EEPROMGetLightTimer(byte devID);

    /* Write light timer into the EEPROM */
    static void EEPROMSetLightTimer(byte devID, unsigned long timer);

    /* Write light status into the EEPROM */
    static void EEPROMSetLightStatus(byte devID, byte status);

    /* get last devID from the array of lightIDs */
    static byte getLastLightDevID();

    /* Write Central Control mode to the EEPROM */
    static void EEPROMSetLightCentral(byte mode);

    /* Read Central Control mode from the EEPROM */
    static byte EEPROMGetLightCentral();

    /* Write Shade type into memory */
    static void EEPROMSetShadeType(byte devID, byte type);

    /* Write Shade tilt timer into memory */
    static void EEPROMSetShadeTiltTimer(byte devID, int timer);

    /* Write Shade position timer into memory */
    static void EEPROMSetShadePosTimer(byte devID, byte timer);

    /* Read Shade type from memory */
    static byte EEPROMGetShadeType(byte devID);

    /* Read Shade tilt timer from memory */
    static int EEPROMGetShadeTiltTimer(byte devID);

    /* Read Shade position timer from memory */
    static byte EEPROMGetShadePosTimer(byte devID);

    /*
     * SD Card related functions
     */

    /* initialize the SD Card */
    static byte SDCardInit();

    /* Open selected file */
    static File SDCardFileOpen(char *filename);

    /* Close the file */
    static void SDCardFileClose(File file);
};

extern Settings Platform;

#endif
