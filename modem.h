#ifndef MODEM_H
#define TINY_GSM_MODEM_SIM800
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
void sendUidHttp(const String& uid);
void publishUidMqtt(const String& uid);

#endif
