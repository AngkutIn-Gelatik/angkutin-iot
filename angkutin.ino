#include "modem.h"
#include "rfid.h"
#include "config.h"
#include "utils.h"
#include "angkot.h"

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

  String tripId = setupDriver();
  infoIndicator("Scan RFID to send data...");
  setupLight(false);
}

void loop() {
  mqttClient.loop();

  if (rfidAvailable()) {
    turnLight(true);
    String uid = rfidGetUid();
    infoIndicator("UID: " + uid);

    publishUidMqtt(uid);
    sendUidHttp(uid);

    turnLight(false);
    delay(2000);
  }
}
