#include "angkot.h"

#include <string.h>
#include "utils.h"
#include "http_wrapper.h"
#include "modem.h"
#include "rfid.h"

String driverUid;
String tripIdResponse;

String setupDriver() {
  infoIndicator("Tap RFID to setup driver", true, true);
  bool lightOn = false;

  while (driverUid == "" || tripIdResponse == "") {
    lightOn = !lightOn;
    turnLight(lightOn);

    if (rfidAvailable()) {
      String uid = rfidGetUid();
      infoIndicator("Driver UID: " + uid);

      tripIdResponse = sendDriverUid(uid);

      turnLight(false);
      delay(2000);
      break;
    }

    delay(500);
  }

  return tripIdResponse;
}
