#include <Arduino.h>
#include <Ethernet.h>
#include "Settings.h"

#ifndef WEBGUI_H_
#define WEBGUI_H_


#define WEBGUI_HTTP_BUFF 200      /* maximum incoming HTTP message length */

#define HTTP_200_OK     "HTTP/1.1 200 OK\nContent-Type: text/html\nConnnection: close\n\n"
#define HTTP_200_OK_XML "HTTP/1.1 200 OK\nContent-Type: text/xml\nConnnection: keep-alive\n"
#define HTTP_200_OK_JS  "HTTP/1.1 200 OK\nContent-Type: application/javascript\nConnnection: close\n\n"
#define HTTP_200_OK_CSS "HTTP/1.1 200 OK\nContent-Type: text/css\nConnnection: close\n\n"
#define HTTP_404_NF     "HTTP/1.1 404\nContent-Type: text/html\nConnnection: close\n\n"

/* WebGUI actions that are communicated outside of the library */
#define CMD_WEBGUI_DEREGISTER   0
#define CMD_WEBGUI_NOTHING      1
#define CMD_WEBGUI_SET_M_LIGHTS 2
#define CMD_WEBGUI_SET_M_SHADES 3

/* system variants */
#define M_WEBGUI_LIGHTS        0
#define M_WEBGUI_SHADES        1


/* shade states */
#define S_WEBGUI_UP      0
#define S_WEBGUI_DOWN    1
#define S_WEBGUI_STOP    2
#define S_WEBGUI_0       3
#define S_WEBGUI_25      4
#define S_WEBGUI_50      5
#define S_WEBGUI_75      6
#define S_WEBGUI_100     7
#define S_WEBGUI_T_0     8
#define S_WEBGUI_T_45    9
#define S_WEBGUI_T_90   10
#define S_WEBGUI_SYNC   11
#define S_WEBGUI_UNSYNC 12

/* light types */
#define S_WEBGUI_L_ONOFF  0
#define S_WEBGUI_L_TIMER 1

/* SD Card active state */
#define SD_WEBGUI_UNAVAIL  0
#define SD_WEBGUI_AVAIL    1

/* shade tracking object */
typedef struct WebShade {
  byte devID;
  byte direction;
  byte position;
  byte tilt;
  byte sync;
};

/* light tracking object */
typedef struct WebLight {
  byte devID;
  byte status;
  byte type;
  unsigned long timer;
};

class WebGUIClass {

  private:

    /* SD Card status */
    static byte SDStatus;

    /* array for holding all shades state */
    static WebShade shades[SHADES];

    /* array for holding all lights state */
    static WebLight lights[LIGHTS];

    static EthernetServer WebGUIServer;
    static EthernetClient WebGUIClient;

    /* function to craft a generic AJAX response */
    static void WebGUIClass::sendXMResponse(EthernetClient cl);

    /* function to generate static HTML file */
    static void WebGUIClass::sendWebGUIHTML(EthernetClient cl);

    /* function to generate static HTML file when SD Card HTML contents are not available */
    static void WebGUIClass::sendWebGUIHTMLStub(EthernetClient client);

    /* send the requested CSS File */
    static void WebGUIClass::sendWebGUICSS(EthernetClient client);

    /* send the requested JS File */
    static void WebGUIClass::sendWebGUIJS(EthernetClient client);

    /* detect if the deregistration button has been pressed on the WebGUI */
    static bool WebGUIClass::deregPressed(char *buff);

  public:
    /* initialization function (to be executed once in setup()) */
    static void WebGUIClass::begin();

    /* function to be executed in every loop() execution */
    static byte WebGUIClass::update();

    /* set local registration variables */
    static void WebGUIClass::setInfoRegistered(byte aID, byte rID, IPAddress rIP);

    /* set local registration variables to deregistered */
    static void WebGUIClass::setInfoDeregistered();

    /* function to set the variant of the GUI */
    static void WebGUIClass::setSystemMode(byte m);

    /* initialize shade object */
    static void WebGUIClass::shadeInit(byte index, byte devID);

    /* set shade direction */
    static void WebGUIClass::shadeSetDirection(byte devID, byte direction);

    /* set shade position */
    static void WebGUIClass::shadeSetPosition(byte devID, byte position);

    /* set shade tilt */
    static void WebGUIClass::shadeSetTilt(byte devID, byte tilt);

    /* initialize light object */
    static void WebGUIClass::lightInit(byte index, byte devID, byte type);

    /* set light to on */
    static void WebGUIClass::lightSetON(byte devID);

    /* set light to off */
    static void WebGUIClass::lightSetOFF(byte devID);

    /* set light type */
    static void WebGUIClass::lightSetType(byte devID, byte type);

    /* set light timer */
    static void WebGUIClass::lightSetTimer(byte devID, unsigned long timer);

    /* set SD Card status as Available */
    static void WebGUIClass::setSDStatusAvailable();

    /* set SD Card status as Unavailable */
    static void WebGUIClass::setSDStatusUnavailable();

    /* return the SD Card status */
    static byte WebGUIClass::getSDStatus();

};

extern WebGUIClass WebGUI;

#endif
