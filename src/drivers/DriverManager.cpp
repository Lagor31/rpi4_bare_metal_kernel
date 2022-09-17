#include "../include/Console.h"
#include "../include/Driver.h"

Vector<Driver*>* DriverManager::drivers;

void DriverManager::load(Driver* d) {
  drivers->push_back(d);
  Console::print("Loaded driver: %s\n", d->getName());
}
void DriverManager::loadAndStart(Driver* d) {
  load(d);
  d->init();
}

void DriverManager::startAll() {
  for (Driver* d : *drivers) {
    d->init();
  }
}

int DriverManager::getDriversCount() { return drivers->size(); }

Vector<Driver*>* DriverManager::getAll() { return drivers; }
