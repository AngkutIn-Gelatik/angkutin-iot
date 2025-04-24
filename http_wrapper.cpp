#include <string.h>
#include "http_wrapper.h"
#include "config.h"

HttpWrapper::HttpWrapper(TinyGsmClient& client) : _client(client) {}

int HttpWrapper::post(const String& url, const String& jsonBody, String& responseBody) {
  String request =
    "POST " + url + " HTTP/1.1\r\n" +
    "Host: " + String(HTTP_SERVER) + "\r\n" +
    "Content-Type: application/json\r\n" +
    "Connection: close\r\n" +
    "Content-Length: " + String(jsonBody.length()) + "\r\n\r\n" +
    jsonBody;

  if (!_client.connect(HTTP_SERVER, HTTP_PORT)) {
    Serial.println("Connection to HTTP server failed!");
    return -1;
  }

  _client.print(request);

  unsigned long timeout = millis();
  while (!_client.available()) {
    if (millis() - timeout > (HTTP_TIMEOUT * 1000)) {
      Serial.println(">>> HTTP Response Timeout!");
      _client.stop();
      return -2;
    }
    delay(10);
  }

  String response = "";
  while (_client.available()) {
    response += (char)_client.read();
  }

  int statusCode = -1;
  int statusIndex = response.indexOf("HTTP/1.1 ");
  if (statusIndex != -1) {
    String statusLine = response.substring(statusIndex, response.indexOf("\r\n", statusIndex));
    statusCode = statusLine.substring(9, 12).toInt();
  }

  int bodyStartIndex = response.indexOf("\r\n\r\n");
  if (bodyStartIndex != -1) {
    responseBody = response.substring(bodyStartIndex + 4);
  }

  _client.stop();
  
  return statusCode;
}
