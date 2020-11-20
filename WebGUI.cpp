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
static byte mode;
static WebShade WebGUIClass::shades[SHADES];
static WebLight WebGUIClass::lights[LIGHTS];

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
      if (deregPressed(buff)) {
        registered = false;
        cmd = CMD_WEBGUI_DEREGISTER;
      }
      client.println(F(HTTP_200_OK_XML));
      sendXMResponse(client);
      client.println();
      client.stop();

    } else if (strstr(buff, "settings")) {
      if (strstr(buff, "types=shades")) {
        if (mode != M_WEBGUI_SHADES) {
          mode = M_WEBGUI_SHADES;
          cmd = CMD_WEBGUI_SET_M_SHADES;
        }
      } else if (strstr(buff, "types=lights")) {
        if (mode != M_WEBGUI_LIGHTS) {
          mode = M_WEBGUI_LIGHTS;
          cmd = CMD_WEBGUI_SET_M_LIGHTS;
        }
      } else {
        /* nothing here */
      }
      client.println(F(HTTP_200_OK));
      sendWebGUIHTML(client);
      client.println();
      client.stop();
    } else {
      /* regular HTTP GET */
      Serial.println("HTTP GET received");
      client.println(F(HTTP_200_OK));
      sendWebGUIHTML(client);
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

static void WebGUIClass::sendWebGUIHTML(EthernetClient client) {
  client.println("<html>");
  client.println("<head>");
  client.println("<title>Velen IoT System</title>");
  client.println("<script type=\"text/javascript\" src=\"http://velen.tech/velen-main.js\"> </script>");
  client.println("<link rel=\"stylesheet\" type=\"text/css\" href=\"http://velen.tech/velen-main.css\" media=\"all\" />");
  client.println("</head>");
  client.println("<body onload=\"GetArduinoIO()\">");
  client.println("<h1>Velen IoT System</h1>");

  /* System Status box */
  client.println("<div class=\"IO_box\">");
  client.println("<h2>System Status</h2>");
  client.println("<div> Version: 1.1.0 </div>");
  client.println("<div id=\"uptime\"> Uptime : --</div>");
  client.println("</div>");

  /* Registration box */
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

  /* System Settings */
  client.println("<div class=\"IO_box\">");
  client.println("<h2>System Settings</h2>");
  client.println("<div> Device Type </div>");
  client.println("<form action=\"/settings\" method=\"get\">");
  client.println("<label for=\"types\">Choose the device type </label>");
  client.println("<select name=\"types\" id=\"types\">");
  if (mode == M_WEBGUI_LIGHTS) {
    client.println("<option value=\"lights\" selected=\"selected\">Lights</option>");
    client.println("<option value=\"shades\">Shades</option>");
  } else if (mode = M_WEBGUI_SHADES) {
    client.println("<option value=\"lights\">Lights</option>");
    client.println("<option value=\"shades\" selected=\"selected\">Shades</option>");
  }
  client.println("</select>");
  client.println("<input type=\"submit\" value=\"Save\">");
  client.println("</form>");
  client.println("</div>");

  /* Device Status and Control */

  if (mode == M_WEBGUI_LIGHTS) {
    for (int i = 0; i < LIGHTS; i++) {
      client.println("<div class=\"IO_box\">");
      client.print("<h2>DevID: ");
      client.print(lights[i].devID);
      client.println(" Data</h2>");

      client.println("<div class=\"device\">");
      client.print("<div id=\"light-status\">Status    : ");
      if (lights[i].status) {
        client.print("ON");
      } else {
        client.print("OFF");
      }
      
      client.println("</div>");
      client.println("</div>");
      client.println("</div>");
    }

  } else if (mode == M_WEBGUI_SHADES) {
    for (int i = 0; i < SHADES; i++) {
      client.println("<div class=\"IO_box\">");
      client.print("<h2>DevID: ");
      client.print(shades[i].devID);
      client.println(" Data</h2>");

      client.println("<div class=\"device\">");
      //client.println("<button type=\"button\" id=\"up\" onclick=\"sendUp()\">Up</button>");
      //client.println("<button type=\"button\" id=\"stop\" onclick=\"sendUp()\">Stop</button>");
      //client.println("<button type=\"button\" id=\"down\" onclick=\"sendUp()\">Down</button><br><br>");
      if (shades[i].sync == S_WEBGUI_UNSYNC) {
        client.println("<div id=\"shade-status\">Status    : unsync</div>");
        client.println("<div id=\"shade-pos\">Pos       : --</div>");
        client.println("<div id=\"shade-tilt\">Tilt      : --</div><br>");
      } else if (shades[i].sync == S_WEBGUI_SYNC ) {
        if (shades[i].direction == S_WEBGUI_UP ) {
          client.println("<div id=\"shade-status\">Status    : UP</div>");
        } else if (shades[i].direction == S_WEBGUI_DOWN ) {
          client.println("<div id=\"shade-status\">Status    : DOWN</div>");
        } else if (shades[i].direction == S_WEBGUI_STOP ) {
          client.println("<div id=\"shade-status\">Status    : STOPPED</div>");
        }
        client.print("<div id=\"shade-pos\">Pos       : ");
        client.print(shades[i].position);
        client.println(" </div>");
        client.print("<div id=\"shade-tilt\">Tilt      : ");
        client.print(shades[i].tilt);
        client.println(" </div><br>");
      }

      //client.println("<div id=\"shade-pos\">Pos       : --</div>");
      //client.println("<div id=\"shade-tilt\">Tilt      : --</div><br>");
      //client.println("<button type=\"button\" id=\"tilt-up\" onclick=\"tiltUp()\">Tilt up</button>");
      //client.println("<button type=\"button\" id=\"tilt-down\" onclick=\"tiltDown()\">Tilt Down</button><br><br>");
      client.println("</div>");
      client.println("</div>");
    }
  }

  client.println("</body>");
  client.println("</html>");
}

static bool WebGUIClass::deregPressed(char *buff) {
  if (strstr(buff, "dereg=1"))  {
    return true;
  } else {
    return false;
  }
}

static void WebGUIClass::setSystemMode(byte m) {
  mode = m;
}

static void WebGUIClass::shadeInit(byte index, byte devID) {
  shades[index].devID = devID;
  shades[index].sync = S_WEBGUI_UNSYNC;
}

static void WebGUIClass::lightInit(byte index, byte devID) {
  lights[index].devID = devID;
  lights[index].status = false;
}

static void WebGUIClass::shadeSetDirection(byte devID, byte direction) {
  for (int i = 0; i < SHADES; i++) {
    if (shades[i].devID == devID) {
      shades[i].direction = direction;
      if (shades[i].sync == S_WEBGUI_UNSYNC) {
        shades[i].sync = S_WEBGUI_SYNC;
      }
    }
  }
}

static void WebGUIClass::shadeSetPosition(byte devID, byte position) {
  for (int i = 0; i < SHADES; i++) {
    if (shades[i].devID == devID) {
      shades[i].position = position;
      if (shades[i].sync == S_WEBGUI_UNSYNC) {
        shades[i].sync = S_WEBGUI_SYNC;
      }
    }
  }
}

static void WebGUIClass::shadeSetTilt(byte devID, byte tilt) {
  for (int i = 0; i < SHADES; i++) {
    if (shades[i].devID == devID) {
      shades[i].tilt = tilt;
      if (shades[i].sync == S_WEBGUI_UNSYNC) {
        shades[i].sync = S_WEBGUI_SYNC;
      }
    }
  }
}

static void WebGUIClass::lightSetON(byte devID) {
  for (int i = 0; i < SHADES; i++) {
    if (lights[i].devID == devID) {
      lights[i].status = true;
    }
  }
}

static void WebGUIClass::lightSetOFF(byte devID) {
  for (int i = 0; i < SHADES; i++) {
    if (lights[i].devID == devID) {
      lights[i].status = false;
    }
  }
}
