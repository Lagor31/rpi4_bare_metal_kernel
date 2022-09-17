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
  void gpio_useAsAlt5(unsigned int pin_number);
  void gpio_useAsAlt3(unsigned int pin_number);
  unsigned int read(long reg);
  unsigned int gpio_call(unsigned int pin_number, unsigned int value,
                         unsigned long base, unsigned int field_size,
                         unsigned int field_max);
  unsigned int gpio_set(unsigned int pin_number, unsigned int value);
  unsigned int gpio_clear(unsigned int pin_number, unsigned int value);
  unsigned int gpio_pull(unsigned int pin_number, unsigned int value);
  unsigned int gpio_function(unsigned int pin_number, unsigned int value);
  void gpio_initOutputPinWithPullNone(unsigned int pin_number);
  void gpio_setPinOutputBool(unsigned int pin_number, unsigned int onOrOff);

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