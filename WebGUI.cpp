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
static byte WebGUIClass::SDStatus;

static void WebGUIClass::begin() {
  Serial.println(F("Starting WebGUI server"));
  WebGUIServer.begin();
}

static byte WebGUIClass::update() {
  cmd = CMD_WEBGUI_NOTHING;
  EthernetClient client = WebGUIServer.available();
  if (client) {
    Serial.print(F("WebGUI: HTTP Request received from: "));
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
    } else if (strstr(buff, "main.css")) {
      Serial.println(F("HTTP GET for main.css received"));
      if (getSDStatus() == SD_WEBGUI_AVAIL) {
        
        client.println(F(HTTP_200_OK_CSS));
        sendWebGUICSS(client);
      } else {
        Serial.println(F("HTTP GET for main.css unavailable"));
        client.println(F(HTTP_404_NF));
      }
      client.println();
      client.stop();
    } else if (strstr(buff, "main.js")) {
      Serial.println(F("HTTP GET for main.js received"));
      if (getSDStatus() == SD_WEBGUI_AVAIL) {
        
        client.println(F(HTTP_200_OK_JS));
        sendWebGUIJS(client);
      } else {
        Serial.println(F("HTTP GET for main.js unavailable"));
        client.println(F(HTTP_404_NF));
      }
      client.println();
      client.stop();
    } else {
      /* regular HTTP GET */
      Serial.println(F("HTTP GET for main page received"));
      client.println(F(HTTP_200_OK));
      if (getSDStatus() == SD_WEBGUI_AVAIL) {
        
        sendWebGUIHTML(client);
      } else {
        Serial.println(F("HTTP GET - sending stub HTML"));
        sendWebGUIHTMLStub(client);
      }
      client.println();
      client.stop();
    }

    delay(1);      // give the web browser time to receive the data
    client.stop(); // close the connection
  }

  return cmd;
}

static void WebGUIClass::setInfoRegistered(byte aID, byte rID, IPAddress rIP) {
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
  client.println(F("<html>"));
  client.println(F("<head>"));
  client.println(F("<title>Velen IoT System</title>"));
  //client.println(F("<script type=\"text/javascript\" src=\"http://37.46.83.239/velen-main.js\"> </script>"));
  //client.println(F("<link rel=\"stylesheet\" type=\"text/css\" href=\"http://37.46.83.239/velen-main.css\" media=\"all\" />"));
  client.println(F("<script type=\"text/javascript\" src=\"velen-main.js\"> </script>"));
  client.println(F("<link rel=\"stylesheet\" type=\"text/css\" href=\"velen-main.css\" media=\"all\" />"));
  client.println(F("</head>"));
  client.println(F("<body onload=\"GetArduinoIO()\">"));
  client.println(F("<h1>Velen IoT System</h1>"));

  /* System Status box */
  client.println(F("<div class=\"IO_box\">"));
  client.println(F("<h2>System Status</h2>"));
  client.println(F("<div> Version: 1.1.0 </div>"));
  client.println(F("<div id=\"uptime\"> Uptime : --</div>"));
  client.println(F("</div>"));

  /* Registration box */
  client.println(F("<div class=\"IO_box\">"));
  client.println(F("<h2>Registration Status</h2>"));
  client.println(F("<br>"));
  client.println(F("<div id=\"reg-status\">Registration Status: --</div>"));
  client.println(F("<div id=\"ardid\">ArdID    : --</div>"));
  client.println(F("<div id=\"raspyid\">RaspyID  : --</div>"));
  client.println(F("<div id=\"raspyip\">Raspy IP : --</div>"));
  client.println(F("<br>"));
  client.println(F("<button type=\"button\" id=\"deregister\" onclick=\"GetButton1()\">Deregister</button><br /><br />"));
  client.println(F("</div>"));

  /* System Settings */
  client.println(F("<div class=\"IO_box\">"));
  client.println(F("<h2>System Settings</h2>"));
  client.println(F("<div> Device Type </div>"));
  client.println(F("<form action=\"/settings\" method=\"get\">"));
  client.println(F("<label for=\"types\">Choose the device type </label>"));
  client.println(F("<select name=\"types\" id=\"types\">"));
  if (mode == M_WEBGUI_LIGHTS) {
    client.println(F("<option value=\"lights\" selected=\"selected\">Lights</option>"));
    client.println(F("<option value=\"shades\">Shades</option>"));
  } else if (mode = M_WEBGUI_SHADES) {
    client.println(F("<option value=\"lights\">Lights</option>"));
    client.println(F("<option value=\"shades\" selected=\"selected\">Shades</option>"));
  }
  client.println(F("</select>"));
  client.println(F("<input type=\"submit\" value=\"Save\">"));
  client.println(F("</form>"));
  client.println(F("</div>"));

  /* Device Status and Control */

  if (mode == M_WEBGUI_LIGHTS) {
    for (int i = 0; i < LIGHTS; i++) {
      client.println(F("<div class=\"IO_box\">"));
      client.print(F("<h2>DevID: "));
      client.print(lights[i].devID);
      client.println(F(" Data</h2>"));

      client.println(F("<div class=\"device\">"));

      /* light status DIV */
      client.print(F("<div id=\"light-status\">Status    : "));
      if (lights[i].status) {
        client.print(F("ON"));
      } else {
        client.print(F("OFF"));
      }
      client.println(F("</div>"));

      /* light type timer DIVs */
      if (lights[i].type == S_WEBGUI_L_TIMER) {
        client.print(F("<div>Timer    : "));
        client.print(lights[i].timer);
        client.println(F("ms </div>"));
      }
         
      client.println(F("</div>"));
      client.println(F("</div>"));
    }

  } else if (mode == M_WEBGUI_SHADES) {
    for (int i = 0; i < SHADES; i++) {
      client.println(F("<div class=\"IO_box\">"));
      client.print(F("<h2>DevID: "));
      client.print(shades[i].devID);
      client.println(F(" Data</h2>"));

      client.println(F("<div class=\"device\">"));
      if (shades[i].sync == S_WEBGUI_UNSYNC) {
        client.println(F("<div id=\"shade-status\">Status    : unsync</div>"));
        client.println(F("<div id=\"shade-pos\">Pos       : --</div>"));
        client.println(F("<div id=\"shade-tilt\">Tilt      : --</div><br>"));
      } else if (shades[i].sync == S_WEBGUI_SYNC ) {
        if (shades[i].direction == S_WEBGUI_UP ) {
          client.println(F("<div id=\"shade-status\">Status    : UP</div>"));
        } else if (shades[i].direction == S_WEBGUI_DOWN ) {
          client.println(F("<div id=\"shade-status\">Status    : DOWN</div>"));
        } else if (shades[i].direction == S_WEBGUI_STOP ) {
          client.println(F("<div id=\"shade-status\">Status    : STOPPED</div>"));
        }
        client.print(F("<div id=\"shade-pos\">Pos       : "));
        client.print(shades[i].position);
        client.println(F(" </div>"));
        client.print(F("<div id=\"shade-tilt\">Tilt      : "));
        client.print(shades[i].tilt);
        client.println(F(" </div><br>"));
      }
      client.println(F("</div>"));
      client.println(F("</div>"));
    }
  }

  client.println(F("</body>"));
  client.println(F("</html>"));
}

static void WebGUIClass::sendWebGUIHTMLStub(EthernetClient client) {
  client.println(F("<html>"));
  client.println(F("<head>"));
  client.println(F("<title>Velen IoT System</title>"));
  client.println(F("</head>"));
  client.println(F("<body"));
  client.println(F("<h1>Velen IoT System</h1>"));
  client.println(F("<h3>HTML files not available on SD Card</h3>"));
  client.println(F("</body>"));
  client.println(F("</html>"));
}

static void WebGUIClass::sendWebGUICSS(EthernetClient client) {
  if (getSDStatus() == SD_WEBGUI_AVAIL) {
    File dataFile = Platform.SDCardFileOpen("MAIN.CSS");
    if (dataFile) {
      while (dataFile.available()) {
        //Serial.write(dataFile.read());
        client.write(dataFile.read());
      }
      dataFile.close();
    }
  } else {
    
  }
}

static void WebGUIClass::sendWebGUIJS(EthernetClient client) {
  if (getSDStatus() == SD_WEBGUI_AVAIL) {
    File dataFile = Platform.SDCardFileOpen("MAIN.JS");
    if (dataFile) {
      while (dataFile.available()) {
        //Serial.write(dataFile.read());
        client.write(dataFile.read());
      }
      dataFile.close();
    }
  } else {
    
  }
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

static void WebGUIClass::lightInit(byte index, byte devID, byte type) {
  lights[index].devID = devID;
  lights[index].status = false;
  lights[index].type = type;
  if (type == S_WEBGUI_L_ONOFF) {
    lights[index].timer = 0;
  }
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
  for (int i = 0; i < LIGHTS; i++) {
    if (lights[i].devID == devID) {
      lights[i].status = true;
    }
  }
}

static void WebGUIClass::lightSetOFF(byte devID) {
  for (int i = 0; i < LIGHTS; i++) {
    if (lights[i].devID == devID) {
      lights[i].status = false;
    }
  }
}

static void WebGUIClass::lightSetType(byte devID, byte type) {
  for (int i = 0; i < LIGHTS; i++) {
    if (lights[i].devID == devID) {
      lights[i].type = type;
    }
  }
}

static void WebGUIClass::lightSetTimer(byte devID, unsigned long timer) {
  for (int i = 0; i < LIGHTS; i++) {
    if (lights[i].devID == devID) {
      lights[i].timer = timer;
    }
  }
}

static void WebGUIClass::setSDStatusAvailable() {
  SDStatus = SD_WEBGUI_AVAIL;
}

static void WebGUIClass::setSDStatusUnavailable() {
  SDStatus = SD_WEBGUI_UNAVAIL;
}

static byte WebGUIClass::getSDStatus() {
  return SDStatus;
}
