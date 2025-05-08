#ifndef MODEM_H
#define TINY_GSM_MODEM_SIM900
#define MODEM_H

#include <TinyGsmClient.h>
#include <PubSubClient.h>
#include <ArduinoHttpClient.h>

extern TinyGsm modem;

extern TinyGsmClient mqttGsmClient;
extern TinyGsmClient httpGsmClient;

extern PubSubClient mqttClient;
extern HttpClient httpClient;

void modemSetup();
void mqttReconnect();
String sendDriverUid(const String& uid);
bool sendUidHttp(const String& tripId, const String& uid);
void publishUidMqtt(const String& tripId, const String& uid);

#endif
