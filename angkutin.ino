#include "modem.h"
#include "rfid.h"
#include "config.h"
#include "utils.h"
#include "angkot.h"

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

  while(tripId == "") {
    tripId = setupDriver();
  }
  
  infoIndicator("Scan RFID to send data...");
  setupLight(false);
}

void loop() {
  mqttClient.loop();

  if (rfidAvailable()) {
    turnLight(true);
    String uid = rfidGetUid();
    infoIndicator("UID: " + uid);

    publishUidMqtt(tripId, uid);
    while (!sendUidHttp(tripId, uid)) {
      delay(500);
    };

    turnLight(false);
    delay(2000);
  }
}
