#include "gps_module.h"
#include "utils.h"

#define RXD2 4
#define TXD2 5
#define GPS_BAUD 9600

TinyGPSPlus gps;
HardwareSerial gpsSerial(2);

void GPSModule::begin() {
  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, RXD2, TXD2);
  infoIndicator("GPS module started");
}

Coordinate GPSModule::getCoordinate(unsigned long timeout) {
  unsigned long start = millis();
  Coordinate coord = {0.0, 0.0, false};

  while (millis() - start < timeout) {
    while (gpsSerial.available()) {
      gps.encode(gpsSerial.read());
    }
    if (gps.location.isUpdated()) {
      coord.latitude = gps.location.lat();
      coord.longitude = gps.location.lng();
      coord.valid = true;
      break;
    }
  }

  return coord;
}
