#ifndef DRIVER_H
#define DRIVER_H

class Driver {
 public:
  virtual void init() = 0;
  virtual const char* getName() = 0;
  virtual void unload() = 0;
};

class DriverManager {
 private:
  static int loaded_drivers;
  static const int MAX_DRIVERS = 10;
  static Driver* drivers[MAX_DRIVERS];

 public:
  static void load(Driver* d);
  static void init() { loaded_drivers = 0; }
  static Driver** getAll();
  static int getDriversCount();
};

#endif