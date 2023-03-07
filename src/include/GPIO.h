#ifndef GPIO_H
#define GPIO_H
#include "Driver.h"

class GPIO : public Driver {
 public:
  GPIO();
  void init();
  const char* getName();
  void unload();
  
  void write(long reg, unsigned int val);
  void useAsAlt5(unsigned int pin_number);
  void useAsAlt3(unsigned int pin_number);
  unsigned int read(long reg);
  unsigned int call(unsigned int pin_number, unsigned int value,
                         unsigned long base, unsigned int field_size,
                         unsigned int field_max);
  unsigned int set(unsigned int pin_number, unsigned int value);
  unsigned int clear(unsigned int pin_number, unsigned int value);
  unsigned int pull(unsigned int pin_number, unsigned int value);
  unsigned int function(unsigned int pin_number, unsigned int value);
  void initOutputPinWithPullNone(unsigned int pin_number);
  void setPinOutputBool(unsigned int pin_number, unsigned int onOrOff);

  enum {
    GPIO_MAX_PIN = 53,
    GPIO_FUNCTION_OUT = 1,
    GPIO_FUNCTION_ALT5 = 2,
    GPIO_FUNCTION_ALT3 = 7
  };

  enum {
    Pull_None = 0,
    Pull_Down = 1,  // Are down and up the right way around?
    Pull_Up = 2
  };
};

#endif