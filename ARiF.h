#include <Arduino.h>

#ifndef ARIF_H_
#define ARIF_H_

/* Shades version 1 with the shade position and tilt */
#define VER_SHD_1   0

/* light version 1 */
#define VER_LGHT_1  1


class ARiFClass {
  private:
    
  public:
    static byte begin(byte version);

    static byte init();

    static byte init(byte ardID, byte raspyID, byte raspyIP, byte mac[]);
};

extern ARiFClass ARiF;

#endif
