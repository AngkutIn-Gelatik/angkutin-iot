#include "Arduino.h"
#include "modem.h"
#include "config.h"
#include "http_wrapper.h"
#include "uid_manager.h"
#include "utils.h"
#include <ArduinoJson.h>

TinyGsm modem(Serial1);

TinyGsmClient mqttGsmClient(modem, 1);
TinyGsmClient httpGsmClient(modem, 2);

PubSubClient mqttClient(mqttGsmClient);
HttpClient httpClient(httpGsmClient, HTTP_SERVER, HTTP_PORT);

HttpWrapper http(httpGsmClient);

UidManager uidManager(20);

void reconnectModem() {
  setupLight(true);
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
  setupLight(false);
}

void reconnectMqtt() {
  setupLight(true);
  while (!mqttClient.connected()) {
    infoIndicator("Connecting to MQTT...");
    if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASS)) {
      infoIndicator("Connected to MQTT!");
    } else {
      errorIndicator("Failed, rc=" + mqttClient.state(), true);
      delay(2000);
    }
  }
  setupLight(false);
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

void publishUidMqtt(const String& tripId, const String& uid) {
  turnLight(true);
  ensureModemAndMqtt();
  if (mqttClient.publish(MQTT_TOPIC, uid.c_str())) {
    infoIndicator("UID sent via MQTT!");
  } else {
    errorIndicator("Failed to send UID via MQTT.");
  }
  turnLight(false);
}

String sendDriverUid(const String& uid) {
  ensureModemAndMqtt();
  turnLight(true);

  String url = "/api/v1/iot/trip";
  String jsonBody = "{\"driver_id\":\"" + uid + "\", \"vehicle_id\": \"" + VEHICLE_ID + "\"}";

  String responseBody;
  int statusCode = http.post(url, jsonBody, responseBody);
  String parsedStatusCode = String(statusCode);

  infoIndicator("Status: " + parsedStatusCode);
  infoIndicator("Request Body: " + responseBody);
  infoIndicator("Card Storage Status");

  if (statusCode == 201) {
    infoIndicator("Succesfully sent driver id to server", true, true);
  } else {
    errorIndicator("Failed to sent driver id to server", true);
  }

  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, responseBody);

  if (error) {
    Serial.print("JSON parsing failed: ");
    Serial.println(error.c_str());
    return "";
  }

  const char* trip_id = doc["trip_id"];

  turnLight(false);
  mqttClient.loop();

  return trip_id;
}

void sendUidHttp(const String& tripId, const String& uid) {
  ensureModemAndMqtt();
  turnLight(true);
  int scanType = 0;

  if (uidManager.exists(uid)) {
    uidManager.remove(uid);
    scanType = 1;
  } else {
    uidManager.add(uid);
  }

  String url = "/api/v1/iot/trip/" + tripId + "/rfid-scan";
  String jsonBody = "{\"nik\":\"" + uid + "\", \"scan_type\": " + scanType + "}";

  String responseBody;
  int statusCode = http.post(url, jsonBody, responseBody);
  String parsedStatusCode = String(statusCode);

  infoIndicator("Status: " + parsedStatusCode);
  infoIndicator("Request Body: " + responseBody);
  infoIndicator("Card Storage Status");
  uidManager.printAll();

  if (statusCode == 204) {
    infoIndicator("Succesfully sent uid to server", true, true);
  } else {
    errorIndicator("Failed to sent data to server", true);
  }

  turnLight(false);
  mqttClient.loop();
}
