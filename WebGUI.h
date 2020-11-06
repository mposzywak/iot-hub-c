#include <Arduino.h>
#include <Ethernet.h>

#ifndef WEBGUI_H_
#define WEBGUI_H_

#define WEBGUI_HTTP_BUFF 200

#define HTTP_200_OK     "HTTP/1.1 200 OK\nContent-Type: text/html\nConnnection: close\n\n"
#define HTTP_200_OK_XML "HTTP/1.1 200 OK\nContent-Type: text/xml\nConnnection: keep-alive\n"

class WebGUIClass {
  private:
    static EthernetServer WebGUIServer;
    static EthernetClient WebGUIClient;

    static void WebGUIClass::sendXMResponse(EthernetClient cl); 

    /* detect if the deregistration button has been pressed on the WebGUI */
    static bool WebGUIClass::deregPressed(char *buff);

  public:

    static void WebGUIClass::begin();

    static byte WebGUIClass::update();

    /* set local registration variables */
    static void WebGUIClass::setInfoRegistered(byte aID, byte rID, IPAddress rIP);

    /* set local registration variables to deregistered */
    static void WebGUIClass::setInfoDeregistered();
  
};

extern WebGUIClass WebGUI;

#endif
