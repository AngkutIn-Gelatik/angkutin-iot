#ifndef HTTP_WRAPPER_H
#define TINY_GSM_MODEM_SIM900
#define HTTP_WRAPPER_H

#include <TinyGsmClient.h>

class HttpWrapper {
public:
  HttpWrapper(TinyGsmClient& client);
  void post(const String& url, const String& jsonBody);

private:
  TinyGsmClient& _client;
};

#endif
