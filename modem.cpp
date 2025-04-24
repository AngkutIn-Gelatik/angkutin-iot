#include "modem.h"
#include "config.h"

TinyGsm modem(Serial1);

TinyGsmClient mqttGsmClient(modem, 1);
TinyGsmClient httpGsmClient(modem, 2);

PubSubClient mqttClient(mqttGsmClient);
HttpClient httpClient(httpGsmClient, HTTP_SERVER, HTTP_PORT);

void reconnectModem() {
  Serial.println("Connecting modem...");

  if (!modem.testAT()) {
    Serial.println("Modem unresponsive. Restarting...");
    modem.restart();
  }

  if (!modem.waitForNetwork()) {
    Serial.println("Network connection failed. Retrying...");
    delay(5000);
    reconnectModem();
  }

  if (!modem.gprsConnect(APN, USER, PASS)) {
    Serial.println("GPRS connection failed. Retrying...");
    delay(5000);
    reconnectModem();
  }

  Serial.println("Modem connected successfully.");
}

void reconnectMqtt() {
  while (!mqttClient.connected()) {
    Serial.println("Connecting to MQTT...");
    if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS)) {
      Serial.println("Connected to MQTT!");
    } else {
      Serial.print("Failed, rc=");
      Serial.println(mqttClient.state());
      delay(5000);
    }
  }
}

void modemSetup() {
  Serial1.begin(GSM_BAUD, SERIAL_8N1, RX_PIN, TX_PIN);
  reconnectModem();

  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setKeepAlive(120);
  reconnectMqtt();

  httpClient.setTimeout(10);

  Serial.println("Modem and network initialized.");
}

void ensureModemAndMqtt() {
  if (!modem.isGprsConnected()) {
    Serial.println("GPRS disconnected, attempting to reconnect...");
    reconnectModem();
  }

  if (!mqttClient.connected()) {
    reconnectMqtt();
  }
}

void publishUidMqtt(const String& uid) {
  ensureModemAndMqtt();
  if (mqttClient.publish(MQTT_TOPIC, uid.c_str())) {
    Serial.println("UID sent via MQTT!");
  } else {
    Serial.println("Failed to send UID via MQTT.");
  }
}

void sendUidHttp(const String& uid) {
  ensureModemAndMqtt();

  String url = "/store_uid?uid=" + uid;
  int err = httpClient.get(url);
  if (err != 0) {
    Serial.println("HTTP request failed!");
    return;
  }

  mqttClient.loop();

  Serial.print("HTTP status: ");
  Serial.println(httpClient.responseStatusCode());
  Serial.println("Response: " + httpClient.responseBody());
  httpClient.stop();
}
