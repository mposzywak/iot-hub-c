#include "WebGUI.h"
#include "ARiF.h"

static EthernetServer WebGUIClass::WebGUIServer(80);
static EthernetClient WebGUIClass::WebGUIClient;

/* state information */
static bool registered;
static byte ardID;
static byte raspyID;
static IPAddress raspyIP;
static byte cmd;

static void WebGUIClass::begin() {
  Serial.println("Starting WebGUI server");
  WebGUIServer.begin();
}

static byte WebGUIClass::update() {
  cmd = CMD_WEBGUI_NOTHING;
  EthernetClient client = WebGUIServer.available();
  if (client) {
    Serial.print("WebGUI: HTTP Request received from: ");
    Serial.println(client.remoteIP());
    char buff[WEBGUI_HTTP_BUFF];
    int index = 0;
    while (client.available()) {
      char c = client.read();
      //Serial.print(c);
      buff[index] = c;
      index++;
      if (c == '/n' or c == '/r' or index >= WEBGUI_HTTP_BUFF) break;
    }

    if (strstr(buff, "ajax_inputs")) {
      /* AJAX query */
      Serial.println("AJAX call received");
      if (deregPressed(buff)) {
        registered = false;
        cmd = CMD_WEBGUI_DEREGISTER;
      }
      client.println(F(HTTP_200_OK_XML));
      sendXMResponse(client);
      client.println();
      client.stop();

    } else {
      /* regular HTTP GET */
      Serial.println("HTTP GET received");
      client.println(F(HTTP_200_OK));

      // send web page
      /*webFile = SD.open("index.htm");        // open web page file
        if (webFile) {
          while(webFile.available()) {
              client.write(webFile.read()); // send web page to client
          }
          webFile.close();
        }*/

      client.println("<html>");
      client.println("<head>");
      client.println("<title>Velen IoT System</title>");
      client.println("<script type=\"text/javascript\" src=\"http://velen.tech/ajax-test.js\"> </script>");
      client.println("<link rel=\"stylesheet\" type=\"text/css\" href=\"http://velen.tech/ajax-test.css\" media=\"all\" />");
      client.println("</head>");
      client.println("<body onload=\"GetArduinoIO()\">");
      client.println("<h1>Velen IoT System</h1>");
      client.println("<div class=\"IO_box\">");
      client.println("<h2>System Status</h2>");
      client.println("<div id=\"uptime\"> Uptime : --</div>");
      client.println("</div>");
      client.println("<div class=\"IO_box\">");
      client.println("<h2>Registration Status</h2>");
      client.println("<br>");
      client.println("<div id=\"reg-status\">Registration Status: --</div>");
      client.println("<div id=\"ardid\">ArdID    : --</div>");
      client.println("<div id=\"raspyid\">RaspyID  : --</div>");
      client.println("<div id=\"raspyip\">Raspy IP : --</div>");
      client.println("<br>");
      client.println("<button type=\"button\" id=\"deregister\" onclick=\"GetButton1()\">Deregister</button><br /><br />");
      client.println("</div>");
      client.println("</body>");
      client.println("</html>");
      client.println();
      client.stop();

    }

    delay(1);      // give the web browser time to receive the data
    client.stop(); // close the connection
  }

  return cmd;
}

static void WebGUIClass::setInfoRegistered(byte aID, byte rID, IPAddress rIP) {
  Serial.println("Registration logged: ");
  Serial.print("raspyID: ");
  Serial.println(raspyID);
  registered = true;
  ardID = aID;
  raspyID = rID;
  raspyIP = rIP;
}

static void WebGUIClass::setInfoDeregistered() {
  registered = false;
}

static void WebGUIClass::sendXMResponse(EthernetClient cl) {
  cl.print("<?xml version = \"1.0\" ?>");
  cl.print("<inputs>");

  /* registration */
  cl.print("<registered>");
  if (registered) {
    cl.print("true");
  }
  else {
    cl.print("false");
  }
  cl.println("</registered>");

  /* uptime */
  cl.print("<uptime>");
  cl.print(millis());
  cl.print("</uptime>");

  if (registered) {
    /* ardID */
    cl.print("<ardID>");
    cl.print(ardID);
    cl.println("</ardID>");

    /* raspyID */
    cl.print("<raspyIP>");
    cl.print(raspyIP);
    cl.println("</raspyIP>");

    /* raspyIP */
    cl.print("<raspyID>");
    cl.print(raspyID);
    cl.println("</raspyID>");
  }
  cl.print("</inputs>");
}

static bool WebGUIClass::deregPressed(char *buff) {
  if (strstr(buff, "dereg=1"))  {
    return true;
  } else {
    return false;
  }
}
