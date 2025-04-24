#include "modem.h"
#include "rfid.h"
#include "config.h"

void setup() {
  Serial.begin(115200);
  while (!Serial);

  pinMode(LED_BUILTIN_PIN, OUTPUT);
  digitalWrite(LED_BUILTIN_PIN, HIGH);

  modemSetup();
  rfidInit();
  Serial.println("Scan RFID to send data...");
}

void loop() {
  mqttClient.loop();

  if (rfidAvailable()) {
    digitalWrite(LED_BUILTIN_PIN, LOW);
    String uid = rfidGetUid();
    Serial.println("UID: " + uid);

    publishUidMqtt(uid);
    sendUidHttp(uid);

    delay(2000);
    digitalWrite(LED_BUILTIN_PIN, HIGH);
  }
}
