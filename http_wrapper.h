#ifndef HTTP_WRAPPER_H
#define TINY_GSM_MODEM_SIM800
#define HTTP_WRAPPER_H

#include <TinyGsmClient.h>

class HttpWrapper {
public:
  HttpWrapper(TinyGsmClient& client);
  int post(const String& url, const String& jsonBody, String& responseBody);

private:
  TinyGsmClient& _client;
};

#endif
