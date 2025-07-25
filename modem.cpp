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

void publishGpsMqtt(const String& coor) {
  ensureModemAndMqtt();
  String topic = String(MQTT_TOPIC) + "/" + String(VEHICLE_ID);
  if (!mqttClient.publish(topic.c_str(), coor.c_str())) {
    errorIndicator("Failed to send Coordinate via MQTT.");
  }
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
  infoIndicator("Response Body: " + responseBody);
  infoIndicator("Card Storage Status");

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

  if (statusCode == 201) {
    infoIndicator("Succesfully sent driver id to server", true, true);
    return trip_id;
  }

  errorIndicator("Failed to sent driver id to server", true);
  return "";
}

bool sendUidHttp(const String& tripId, const String& uid) {
  ensureModemAndMqtt();
  turnLight(true);
  int scanType = 0;

  if (uidManager.exists(uid)) {
    scanType = 1;
    infoIndicator("Penumpang turun");
  } else {
    infoIndicator("Penumpang naik");
  }

  String url = "/api/v1/iot/trip/" + tripId + "/rfid-scan";
  String jsonBody = "{\"nik\":\"" + uid + "\", \"scan_type\": " + scanType + "}";

  String responseBody;
  int statusCode = http.post(url, jsonBody, responseBody);
  String parsedStatusCode = String(statusCode);

  infoIndicator("Status: " + parsedStatusCode);
  infoIndicator("Request Body: " + responseBody);
  infoIndicator("Card Storage Status");

  turnLight(false);
  mqttClient.loop();

  if (statusCode == 204) {
    infoIndicator("Succesfully sent uid to server", true, true);
    if (uidManager.exists(uid)) {
      uidManager.remove(uid);
    } else {
      uidManager.add(uid);
    }

    uidManager.printAll();
    return true;
  } else if (statusCode == 500) {
    errorIndicator("Failed to sent data to server (500)", true);
    return true;
  }


  errorIndicator("Failed to sent data to server", true);
  return false;
}
