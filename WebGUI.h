#include <Arduino.h>
#include <Ethernet.h>

#ifndef WEBGUI_H_
#define WEBGUI_H_


#define WEBGUI_HTTP_BUFF 200      /* maximum incoming HTTP message length */

#define HTTP_200_OK     "HTTP/1.1 200 OK\nContent-Type: text/html\nConnnection: close\n\n"
#define HTTP_200_OK_XML "HTTP/1.1 200 OK\nContent-Type: text/xml\nConnnection: keep-alive\n"

/* WebGUI actions that are communicated outside of the library */
#define CMD_WEBGUI_DEREGISTER   0
#define CMD_WEBGUI_NOTHING      1
#define CMD_WEBGUI_SET_V_LIGHTS 2
#define CMD_WEBGUI_SET_V_SHADES 3

/* system variants */
#define V_WEBGUI_LIGHTS        0
#define V_WEBGUI_SHADES        1

class WebGUIClass {
  private:
    static EthernetServer WebGUIServer;
    static EthernetClient WebGUIClient;

    /* function to craft a generic AJAX response */
    static void WebGUIClass::sendXMResponse(EthernetClient cl); 

    /* function to generate static HTML file */
    static void WebGUIClass::sendWebGUIHTML(EthernetClient cl);

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
    static void WebGUIClass::setInfoSystemVariant(byte v);
  
};

extern WebGUIClass WebGUI;

#endif
