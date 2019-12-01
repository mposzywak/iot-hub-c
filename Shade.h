#include <Arduino.h>

/*
 * Class 
 */
class Shade {
  private:
    byte shadeID;

    /* pin numbers that are related to defined hardware */
    byte outPinUp;
    byte outPinDown;
    byte inPinUp;
    byte inPinDown;

    /* variable holding the left running time */
    byte runTimer;

    /* variables holding states of input and output pins */
    byte outPinUpState;
    byte outPinDownState;
    byte inPinUpState;
    byte inPinDownState;

    /* variables holding information if the in Pins were pressed in previous iteration */
    bool inPinUpPressed;
    bool inPinDownPressed;

    static byte low;
    static byte high;

  public:
  /*
   * Contructor takes shadesID where the value cannot exceed the available in/out pins devided by 2.
   */
  Shade(byte shadeID);

  void init();

  bool isUpPressed();

  void isDownPressed();

  void Up();

  void Down();

  void Stop();
  
};
