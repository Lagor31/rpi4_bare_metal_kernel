#ifndef DRIVER_H
#define DRIVER_H

#include "Vector.h"

class Driver {
 public:
  virtual void init() = 0;
  virtual const char* getName() = 0;
  virtual void unload() = 0;
};

class DriverManager {
 private:
  static Vector<Driver*>* drivers;

 public:
  static void load(Driver* d);
  static void loadAndStart(Driver* d);
  static void startAll();

  static void init() { drivers = new Vector<Driver*>(); }
  static Vector<Driver*>* getAll();
  static int getDriversCount();
};

#endif