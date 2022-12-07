#include <Arduino.h>
#include <Ethernet.h>
#include <EthernetUDP.h>
#include "Settings.h"

#ifndef ARIF_H_
#define ARIF_H_

#define ARiF_INITIAL_MAC   { 0x00, 0xAA, 0xBB, 0x13, 0xF9, 0x45 }

#define ARiF_HTTP_PORT 32302                  /* port used as the listening port for ARiF connections and destination port for outgoing ones */
#define ARiF_HTTP_BUFF 400                    /* maximum readable length of the incoming HTTP message */
#define ARiF_HB_TIMEOUT 10                    /* ARiF HB timeout in seconds */
#define ARiF_BEACON_INT 5                     /* interval which the beacon is sent at (in seconds) */
#define ARiF_BEACON_LENGTH 15
#define ARiF_BEACON_PORT 5007                 /* Destination port where the beacon message is sent to */
#define ARiF_BEACON_STRING "/smarthouse/%d"   /* String that we send in the beacon message */

#define HTTP_200_OK "HTTP/1.1 200 OK\nContent-Type: text/html\nConnnection: close\n\n"
#define HTTP_500_Error "HTTP/1.1 500\nContent-Type: text/html\nConnnection: close\n\n"
#define HTTP_403_Error "HTTP/1.1 403\nContent-Type: text/html\nConnnection: close\n\n"
#define HTTP_404_Error "HTTP/1.1 404\nContent-Type: text/html\nConnnection: close\n\n"

/* Shades version 1 with the shade position and tilt */
#define VER_SHD_1   0

/* light version 1 */
#define VER_LGHT_1  1

/* ARiF messages different CMD codes & values returned by the update() in case CMD is captured */
#define CMD_REGISTER     0
#define CMD_HEARTBEAT    1
#define CMD_LIGHTON      2
#define CMD_LIGHTOFF     3
#define CMD_SHADEPOS     4
#define CMD_SHADETILT    5
#define CMD_SHADEUP      6
#define CMD_SHADEDOWN    7
#define CMD_SHADESTOP    8
#define CMD_LIGHT_TYPE   9
#define CMD_LIGHT_TIMER  10
#define CMD_CTRL_ON      11
#define CMD_CTRL_OFF     12
#define CMD_MODE_LIGHTS  13
#define CMD_MODE_SHADES  14
#define CMD_TIMER_POS    15
#define CMD_TIMER_TILT   16
#define CMD_INPUT_HOLD   17
#define CMD_INPUT_REL    18
#define CMD_DEREGISTER   19
#define CMD_INPUT_OVERRIDE_ON    20
#define CMD_INPUT_OVERRIDE_OFF   21
#define CMD_CTRL_DEV_ON  22
#define CMD_CTRL_DEV_OFF 23
#define CMD_LIGHT_SETTINGS 24
#define CMD_RESTORE      25
#define CMD_UNKNOWN      200

/* values returned by update() other than the CMDs above */
#define U_NOTHING       50
#define U_DISCONNECTED  51
#define U_CONNECTED     60
#define U_REGISTERED    53
#define U_RASPYIPCHGD   54


/* getValue() codes - i. e. what do we want the function to return */
#define DEVID   0
#define ARDID   1
#define RASPYID 2
#define CMD     3
#define VALUE   4
#define VALUE_L 5 /* value long */
#define VALUE_M 6 /* value MAC address */

/* Shade status to control sendShadeStatus() CMD value */
#define VAL_MOVE_UP     2
#define VAL_MOVE_DOWN   1
#define VAL_STOPPED     0
#define VAL_UNSYNC      3
#define VAL_SYNC        4

/* Light status to control sendLightStatus() CMD value */
#define VAL_OFF        0
#define VAL_ON         1

/* shade status dataType values */
#define DT_DIRECTION      0
#define DT_POSITION       1
#define DT_TILT           2
#define DT_SYNC           3
#define DT_DIRECTION_USER 4
#define DT_TILT_USER      5

/* define modes of operations */
#define M_SHADES     1
#define M_LIGHTS     0

/* define centralON status */
#define ARIF_CTRLON_DISABLED  0
#define ARIF_CTRLON_ENABLED   1

/* define factore restore status */
#define ARIF_RESTORE_DISABLED 0
#define ARIF_RESTORE_ENABLED  1

/* INTERNAL sendMessage() messageTypes */
#define _MTYPE_LIGHT_TYPE         0
#define _MTYPE_LIGHT_INPUT_TYPE   1
#define _MTYPE_LIGHT_STATUS_ON    2
#define _MTYPE_LIGHT_STATUS_OFF   3
#define _MTYPE_LIGHT_STATUS_ON_USER  4
#define _MTYPE_LIGHT_STATUS_OFF_USER 5
#define _MTYPE_LIGHT_STATUS_INT_VALUE 6

class ARiFClass {
  /* types definitions */

  /* time structure */
  typedef struct t  {
    unsigned long tStart;
    unsigned long tTimeout;
  };

  /* settings data structure */
  typedef struct t_settings  {
    byte major;
    byte minor;
    byte patch;
    byte ctrlON;
    byte mode;
  };
  
  private:
    /* holds the SW version which also can dictate what this particular modules will support, i. e. lights, shades, analog inputs etc.. */
    static byte version;

    /* holds the assigned ardID, if not registered supposed to be 0 */
    static byte ardID;

    /* holds the raspyID that we are talking to */
    static byte raspyID;

    /* holds the MAC address received from raspy */
    static byte raspyMAC[6];

    /* holds the MAC address used on Ethernet */
    static byte mac[];

    /* holds the raspy IP we are talking to */
    static IPAddress raspyIP;

    /* variable to indicate the DHCP failure at the boot. If DHCP failed
    *  at boot then there is no sense in re-trying every 1 sec as it will block
    *  the Hub for at least one second rendering it unresponsive. 
    *  -- Workaround: reboot the Hub once Connctivity/DHCP server is fixed. */
    static bool DHCPFailed;
    static bool initDHCPFailed;

    /* variable indicating if Ethernet should be reloaded with the new MAC */
    static bool restartNewMAC;

    /* stores value how many seconds ago last heartbeat was received */
    static byte lastHeartbeat;

    /* variable to check if beacon was already sent this second */
    static bool beaconSent;

    /* UDP socket object for sending beacons */
    static EthernetUDP UDP;

    /* ARiF multicast IP for sending beacons */
    static IPAddress mip;

    /* holds information if the iot-hub is connected to the iot-gw */
    static bool isConnected;

    /* ARiF REST HTTP Server */
    static EthernetServer ARiFServer;

    /* AriF REST HTTP Client */
    static EthernetClient ARiFClient;

    /* variable used to track if everySec() has been already executed this second */
    static byte oldSec;  

    /* variable holding devID of the last received command */
    static byte lastDevID;

    /* variable holding shade position received by the last shadePOS command */
    static byte lastShadePosition;

    /* variable holding shade tilt received by the last shadeTILT command */
    static byte lastShadeTilt;

    /* variable holding light type received by the last settings command */
    static byte lastLightType;

    /* variable holding light type received by the last settings command */
    static byte lastLightInputType;

    /* variable holding light ctrlON received by the last settings command */
    static byte lastLightCtrlON;

    /* variable holding light timer received by the last settings command */
    static unsigned long lastLightTimer;

    /* variable holding last shade position timer */
    static int lastShadePositionTimer;

    /* variable holding last shade tilt timer */
    static int lastShadeTiltTimer;

    /* holds information if this arduino is registered */
    static bool isRegistered;

    /* variable used to signal if the IP of the raspy has changed */
    static bool signalIPchange;

    /* mode of operations */
    static byte mode;

    /* status of the ctrlON function */
    static byte ctrlON;

    //Tasks and their Schedules.
    static t t_func1;
    static t t_func2;
    static t t_func3;

    static bool timeCheck(struct t *t);

    static void timeRun(struct t *t);

    /* timer struct used to send settings regularly */
    static t_settings settings;

    /* stores indicator if next reboot shall return to factory settings */
    static byte restore;

    
    /*
     * Functions
     */

    /* function to initialize the Ethernet interface and all server and client objects */
    static byte beginEthernet();

    /* get single value from the ARiF URL */
    static long getValue(char *buff, int value);

    /* get MAC address from the ARiF registration URL */
    static void getMAC(char *buff, byte *mac);

    /* return true if the raspy IP address is same as the old one*/
    static bool checkIotGwIP(IPAddress ip);

    /* simple function sending shade status to the raspy 
     * Where status is one of #defined values: 
     * SS_MOVE_UP
     * SS_MOVE_DOWN
     * SS_STOPPED
     */
    static void sendShadeStatus(byte devID, byte dataType, byte value);

    /* get the hex value of two digits and convert them into byte */
    static byte getByteFromHex(const char *string);

    /* convert single hex char to byte */
    static byte nibble(char c);

    /* verify if newly received MAC address from raspy is different then one that is saved.
     * If it is, replace it and restart Ethernet and save the new MAC into EEPROM.
     */
    static byte replaceMAC();

    /* compares two IP addresses. Returns true if they are the same IP, otherwise returns false */
    static bool compareIP(IPAddress ip1, IPAddress ip2);

    /* add the URL preamble into the system */
    static void addPreamble(byte devID);
    
  public:

    /*  library initialization functions where version is the following:
    VER_LGHT_1 - IQbutton handling digitIN/digitOUT devices (lights)
    VER_SHD_1 - IQbutton handling digitIN/shades devices (shades)
    This function needs to be called during the setup time, if this IQButton IS registered and the registration values are known.
    They must be stored in EEPROM outside of this library */
    static byte begin(byte version, byte mac[], IPAddress raspyIP, byte ardID, byte raspyID);

    /* This function needs to be called during the setup time, if this IQButton is NOT registered */
    static byte begin(byte version, byte mac[]);

    /*  Function must be executed every loop. It updates all the timers and handles incoming ARiF messages 
    returns:
        U_NOTHING - if no action need to be taken
        CMD_REGISTER - if this IQbutton has been registered (this means that the ardID, raspyID etc.. needs to be saved)
          This means that the values resulting from registration must be asked using other methods and stored somewhere outside of this API
        U_DISCONNECTED - if there is a timeout and raspy got disconnected
        U_CONNECTED - if the raspy got connected back
        CMD_SHADEPOS - if the shade CMD indicating position was received
        CMD_SHADETILT - if the shade CMD indicating tilt was received (digitOUT)

    */
    static byte update();

    /* send the shade moving UP indication towards the raspy */
    static void sendShadeUp(byte devID);
    static void sendUserShadeUp(byte devID);

    /* send the shade moving DOWN indication towards the raspy */
    static void sendShadeDown(byte devID);
    static void sendUserShadeDown(byte devID);

    /* send the shade stopped indication towards the raspy */
    static void sendShadeStop(byte devID);
    static void sendUserShadeStop(byte devID);

    /* send the shade current position towards the raspy */
    static void sendShadePosition(byte devID, byte position);

    /* send the shade current tilt towards the raspy */
    static void sendShadeTilt(byte devID, byte tilt);
    static void sendUserShadeTilt(byte devID, byte tilt);

    /* get the currently assigned raspberry IP address */
    static IPAddress getRaspyIP();

    /* get the raspyID value of the raspy currently controlling this arduino */
    static byte getRaspyID();

    /* get the currently assigned ardID by the controlling raspy */
    static byte getArdID();

    /* get the devID of the last received command */
    static byte getLastDevID();

    /* get the position of the last received shadePOS command */
    static byte getLastShadePosition();

    /* get the tilt of the last received shadeTILT command */
    static byte getLastShadeTilt();

    /* get the type of the last received lightType command */
    static byte getLastLightType();

    /* get the timer of the last received lightTimer command */
    unsigned long getLastLightTimer();

    /* get the shade Position Timer received through shadePTimer command */
    int getLastShadePositionTimer();

    /* get the shade Tilt Timer received through shadeTTimer command */
    int getLastShadeTiltTimer();

    /* send the shade sync information */
    static void sendShadeSynced(byte devID);

    /* send information that the shade is not synced */
    static void sendShadeUnsynced(byte devID);

    /* send system settings and status */
    static void sendSettings();

    /* send light specific settings */
    static void sendLightSettings(byte devID, unsigned long timer, byte type, byte inputType, byte ctrlON);

    /* send shade position timer */
    static void sendShadeSettings(byte devID, int positionTimer, int tiltTimer);

    /* send a generic message to the raspy */
    static void sendMessage(byte devID, byte messageType, unsigned int value);

    /* deregister this arduino */
    static void deregister();

    /* send the generic light status message */
    static void ARiFClass::sendLightStatus(byte devID, byte value);

    /* send the light ON status message */
    static void ARiFClass::sendLightON(byte devID);

    /* send the light ON status message with a physical input triggered indication */
    static void ARiFClass::sendUserLightON(byte devID);

    /* send the light OFF status message */
    static void ARiFClass::sendLightOFF(byte devID);

    /* send the light OFF status message with a physical input triggered indication */
    static void ARiFClass::sendUserLightOFF(byte devID);

    /* send the temperature value */
    static void ARiFClass::sendTempStatus(byte devID, float value);

    /* set the arduino mode */
    static void ARiFClass::setMode(byte m);

    /* set the ctrlON mode */
    static void ARiFClass::setCtrlON(byte c);

    /* print MAC address in XX:XX:XX... format */
    static void printMAC(const byte *mac);

    /* send counter value */
    static void sendCounter(byte devID, unsigned int counter);

    /* returns true if ARiF is in connected state */
    static bool isARiFConnected();
};

extern ARiFClass ARiF;

#endif
