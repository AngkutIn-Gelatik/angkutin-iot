#include "modem.h"
#include "config.h"
#include "http_wrapper.h"
#include "uid_manager.h"
#include "utils.h"

TinyGsm modem(Serial1);

TinyGsmClient mqttGsmClient(modem, 1);
TinyGsmClient httpGsmClient(modem, 2);

PubSubClient mqttClient(mqttGsmClient);
HttpClient httpClient(httpGsmClient, HTTP_SERVER, HTTP_PORT);

HttpWrapper http(httpGsmClient);

UidManager uidManager(20);

void reconnectModem() {
  turnLight(true);
  infoIndicator("Connecting modem...");

  if (!modem.testAT()) {
    errorIndicator("Modem unresponsive. Restarting...", true);
    modem.restart();
  }

  if (!modem.waitForNetwork()) {
    errorIndicator("Network connection failed. Retrying...", true);
    delay(2000);
    reconnectModem();
  }

  if (!modem.gprsConnect(APN, USER, PASS)) {
    errorIndicator("GPRS connection failed. Retrying...", true);
    delay(2000);
    reconnectModem();
  }

  infoIndicator("Modem connected successfully.");
  turnLight(false);
}

void reconnectMqtt() {
  while (!mqttClient.connected()) {
    turnLight(true);
    infoIndicator("Connecting to MQTT...");
    if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS)) {
      infoIndicator("Connected to MQTT!");
      turnLight(false);
    } else {
      errorIndicator("Failed, rc=" + mqttClient.state(), true);
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

  infoIndicator("Modem and network initialized.", true, true);
}

void ensureModemAndMqtt() {
  if (!modem.isGprsConnected()) {
    errorIndicator("GPRS disconnected, attempting to reconnect...", true);
    reconnectModem();
  }

  if (!mqttClient.connected()) {
    reconnectMqtt();
  }
}

void publishUidMqtt(const String& uid) {
  ensureModemAndMqtt();
  if (mqttClient.publish(MQTT_TOPIC, uid.c_str())) {
    infoIndicator("UID sent via MQTT!");
  } else {
    errorIndicator("Failed to send UID via MQTT.");
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

  infoIndicator("Status: " + parsedStatusCode);
  Serial.println("Request Body: " + responseBody);
  Serial.println("Card Storage Status");
  uidManager.printAll();

  if (statusCode == 204) {
    infoIndicator("Succesfully sent uid to server", true, true);
  } else {
    errorIndicator("Failed to sent data to server", true);
  }

  mqttClient.loop();
}
