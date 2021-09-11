#include <Ethernet.h>

#ifndef REST_H_
#define REST_H_

#define REST_PORT 8080
#define REST_HTTP_BUFF 200

class RESTClass {
  private:

    static EthernetServer RESTServer;
    static EthernetClient RESTClient;

  public:

  /* Class constructor */
  static byte RESTClass::begin();

  /* processing function for the main loop */
  static byte RESTClass::update();
};

extern RESTClass REST;

#endif
