#include "REST.h"

static EthernetServer RESTClass::RESTServer(REST_PORT);
static EthernetClient RESTClass::RESTClient;

static byte RESTClass::begin() {
  Serial.println(F("Starting REST server"));
  RESTServer.begin();
  return 0;
}

static byte RESTClass::update() {
  EthernetClient client = RESTServer.available();
  if (client) {
    Serial.print(F("REST: HTTP Request received from: "));
    Serial.println(client.remoteIP());
    char buff[REST_HTTP_BUFF];
    int index = 0;
    while (client.available()) {
      char c = client.read();
      //Serial.print(c);
      buff[index] = c;
      index++;
      if (c == '/n' or c == '/r' or index >= REST_HTTP_BUFF) break;
    }
  }
}
