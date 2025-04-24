#include "http_wrapper.h"
#include "config.h"

HttpWrapper::HttpWrapper(TinyGsmClient& client) : _client(client) {}

void HttpWrapper::post(const String& url, const String& jsonBody) {
  String request =
    "POST " + url + " HTTP/1.1\r\n" +
    "Host: " + String(HTTP_SERVER) + "\r\n" +
    "Content-Type: application/json\r\n" +
    "Connection: close\r\n" +
    "Content-Length: " + String(jsonBody.length()) + "\r\n\r\n" +
    jsonBody;

  if (!_client.connect(HTTP_SERVER, HTTP_PORT)) {
    Serial.println("Connection to HTTP server failed!");
    return;
  }

  _client.print(request);

  unsigned long timeout = millis();
  while (!_client.available()) {
    if (millis() - timeout > (HTTP_TIMEOUT * 1000)) {
      Serial.println(">>> HTTP Response Timeout!");
      _client.stop();
      return;
    }
    delay(10);
  }

  while (_client.available()) {
    String line = _client.readStringUntil('\n');
    Serial.println(line);
  }

  _client.stop();
  Serial.println("HTTP POST sent with JSON.");
}
