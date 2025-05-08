#include "modem.h"
#include "rfid.h"
#include "config.h"
#include "utils.h"
#include "angkot.h"
#include "gps_module.h"

String tripId;

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;

  pinMode(RGB_PIN, OUTPUT);

  setupIndicators(RGB_PIN, 18);
  infoIndicator("successfully setup indicators", true, true);

  setupLight(true);
  modemSetup();
  rfidInit();

  while (tripId == "") {
    tripId = setupDriver();
  }

  GPSModule::begin();

  infoIndicator("Scan RFID to send data...");
  setupLight(false);
}

unsigned long lastGpsCheck = 0;
const unsigned long gpsInterval = 2000;

void loop() {
  mqttClient.loop();

  if (millis() - lastGpsCheck >= gpsInterval) {
    lastGpsCheck = millis();

    Coordinate coord = GPSModule::getCoordinate();
    if (coord.valid) {
      String coor = "{\"lat\": \"" + String(coord.latitude, 6) + 
                    "\",\n\"long\": \"" + String(coord.longitude, 6) + "\"}";

      publishGpsMqtt(coor);
    } else {
      errorIndicator("Failed to get valid GPS coordinates.");
    }
  }

  if (rfidAvailable()) {
    turnLight(true);
    String uid = rfidGetUid();
    infoIndicator("UID: " + uid);

    while (!sendUidHttp(tripId, uid)) {
      delay(500);
    }

    turnLight(false);
    delay(1000);
  }
}
