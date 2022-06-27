#include "Driver.h"
#include "../Console.h"

Driver* DriverManager::drivers[];

int DriverManager::loaded_drivers;

void DriverManager::
load(Driver* d) {
  if (loaded_drivers < MAX_DRIVERS) {
    drivers[loaded_drivers] = d;
    ++loaded_drivers;
    Console::print("Loaded driver: %s", d->getName());
  }
}

int DriverManager::getDriversCount(){
    return loaded_drivers;
}
Driver** DriverManager::getAll() { return DriverManager::drivers; }