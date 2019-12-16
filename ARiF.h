#include <Arduino.h>
#include <Ethernet.h>
#include <EthernetUDP.h>
#include <Controllino.h>

#ifndef ARIF_H_
#define ARIF_H_

#define ARiF_HTTP_PORT 32302
#define ARiF_HTTP_BUFF 150
#define ARiF_BEACON_INT 5
#define ARiF_BEACON_LENGTH 15
#define ARiF_BEACON_PORT 5007
#define ARiF_BEACON_STRING "/smarthouse/%d"

#define HTTP_200_OK "HTTP/1.1 200 OK\nContent-Type: text/html\nConnnection: close\n\n"
#define HTTP_500_Error "HTTP/1.1 500\nContent-Type: text/html\nConnnection: close\n\n"


/* Shades version 1 with the shade position and tilt */
#define VER_SHD_1   0

/* light version 1 */
#define VER_LGHT_1  1

/* ARiF messages different CMD codes & values returned by the update() in case CMD is captured */
#define CMD_REGISTER  0
#define CMD_HEARTBEAT 1
#define CMD_LIGHTON   2
#define CMD_LIGHTOFF  3
#define CMD_SHADEPOS  4
#define CMD_SHADETILT 5
#define CMD_UNKNOWN   10

/* values returned by update() other than the CMDs above */
#define U_NOTHING       50
#define U_DISCONNECTED  51
#define U_CONNECTED     52
#define U_REGISTERED    53
#define U_RASPYIPCHGD   54


/* getValue() codes - i. e. what do we want the function to return */
#define DEVID   0
#define ARDID   1
#define RASPYID 2
#define CMD     3

/* Shade status to control sendShadeStatus() CMD value */
#define SS_MOVE   1
#define SS_STOP   0

class ARiFClass {
  private:
    /* holds the SW version which also can dictate what this particular modules will support, i. e. lights, shades, analog inputs etc.. */
    static byte version;

    /* holds the assigned ardID, if not registered supposed to be 0 */
    static byte ardID;

    /* holds the raspyID that we are talking to */
    static byte raspyID;

    /* holds the MAC address */
    static byte mac[6];

    /* holds the raspy IP we are talking to */
    static IPAddress raspyIP;

    /* variable to indicate the DHCP failure at the boot. If DHCP failed
    *  at boot then there is no sense in re-trying every 1 sec as it will block
    *  the Hub for at least one second rendering it unresponsive. 
    *  -- Workaround: reboot the Hub once Connctivity/DHCP server is fixed. */
    static bool DHCPFailed;

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

    /* holds information if this arduino is registered */
    static bool isRegistered;

    /* variable used to signal if the IP of the raspy has changed */
    static bool signalIPchange;

    /*
     * Functions
     */

    /* function to initialize the Ethernet interface and all server and client objects */
    static byte beginEthernet(byte mac[]);

    /* get single value from the ARiF URL */
    static int getValue(char *buff, int value);

    /* return true if the raspy IP address is same as the old one*/
    static bool checkIotGwIP(IPAddress ip);

    /* simple function sending shade status to the raspy */
    static void sendShadeStatus(byte devID, byte status);
    
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
    returns - 0 - if no action need to be taken
        1 - if this IQbutton has been registered (this means that the ardID, raspyID etc.. needs to be saved)
          This means that the values resulting from registration must be asked using other methods and stored somewhere outside of this API
        2 - if there is a timeout and raspy got disconnected
        3 - if the raspy got connected back
        4 - if the shade CMD was received
        5 - if the light CMD was received (digitOUT)

    */
    static byte update();

    static void sendShadeUp(byte devID);

    static void sendShadeDown(byte devID);

    static void sendShadeStop(byte devID);

};

extern ARiFClass ARiF;

#endif
