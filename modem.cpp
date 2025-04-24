#include "modem.h"
#include "config.h"
#include "http_wrapper.h"
#include "uid_manager.h"

TinyGsm modem(Serial1);

TinyGsmClient mqttGsmClient(modem, 1);
TinyGsmClient httpGsmClient(modem, 2);

PubSubClient mqttClient(mqttGsmClient);
HttpClient httpClient(httpGsmClient, HTTP_SERVER, HTTP_PORT);

HttpWrapper http(httpGsmClient);

UidManager uidManager(20);

void reconnectModem() {
  Serial.println("Connecting modem...");

  if (!modem.testAT()) {
    Serial.println("Modem unresponsive. Restarting...");
    modem.restart();
  }

  if (!modem.waitForNetwork()) {
    Serial.println("Network connection failed. Retrying...");
    delay(2000);
    reconnectModem();
  }

  if (!modem.gprsConnect(APN, USER, PASS)) {
    Serial.println("GPRS connection failed. Retrying...");
    delay(2000);
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
      delay(2000);
    }
  }
}

void modemSetup() {
  Serial1.begin(GSM_BAUD, SERIAL_8N1, RX_PIN, TX_PIN);
  reconnectModem();

  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  mqttClient.setKeepAlive(120);
  reconnectMqtt();

  httpClient.setTimeout(HTTP_TIMEOUT);

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
  int scanType = 0;

  if (uidManager.exists(uid)) {
    uidManager.remove(uid);
    scanType = 1;
  } else {
    uidManager.add(uid);
  }

  String url = "/api/v1/iot/trip/123123123/rfid-scan";
  String jsonBody = "{\"nik\":\"" + uid + "\", \"scan_type\": " + scanType + "}";

  String responseBody;
  int statusCode = http.post(url, jsonBody, responseBody);
  String parsedStatusCode = String(statusCode);

  Serial.println("Status: " + parsedStatusCode);
  Serial.println("Request Body: " + responseBody);
  Serial.println("Card Storage Status");
  uidManager.printAll();

  mqttClient.loop();
}
