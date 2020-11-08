#include <Arduino.h>
#include <Ethernet.h>

#ifndef WEBGUI_H_
#define WEBGUI_H_


#define WEBGUI_HTTP_BUFF 200      /* maximum incoming HTTP message length */

#define HTTP_200_OK     "HTTP/1.1 200 OK\nContent-Type: text/html\nConnnection: close\n\n"
#define HTTP_200_OK_XML "HTTP/1.1 200 OK\nContent-Type: text/xml\nConnnection: keep-alive\n"

/* WebGUI actions that are communicated outside of the library */
#define CMD_WEBGUI_DEREGISTER  0
#define CMD_WEBGUI_NOTHING     1



class WebGUIClass {
  private:
    static EthernetServer WebGUIServer;
    static EthernetClient WebGUIClient;

    static void WebGUIClass::sendXMResponse(EthernetClient cl); 

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
  
};

extern WebGUIClass WebGUI;

#endif
