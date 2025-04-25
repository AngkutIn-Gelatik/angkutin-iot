#include "modem.h"
#include "rfid.h"
#include "config.h"
#include "utils.h"

void setup() {
  Serial.begin(115200);
  while (!Serial);

  pinMode(RGB_PIN, OUTPUT);

  setupIndicators(RGB_PIN, 18);
  infoIndicator("successfully setup indicators", true, true);

  turnLight(true);
  modemSetup();
  rfidInit();
  infoIndicator("Scan RFID to send data...");
}

void loop() {
  mqttClient.loop();

  if (rfidAvailable()) {
    turnLight(true);
    String uid = rfidGetUid();
    infoIndicator("UID: " + uid);

    publishUidMqtt(uid);
    sendUidHttp(uid);

    delay(2000);
  }
}
