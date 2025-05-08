#ifndef GPS_MODULE_H
#define GPS_MODULE_H

#include <Arduino.h>
#include <TinyGPS++.h>

struct Coordinate {
  double latitude;
  double longitude;
  bool valid;
};

namespace GPSModule {
  void begin();
  Coordinate getCoordinate(unsigned long timeout = 1000);
}

#endif
