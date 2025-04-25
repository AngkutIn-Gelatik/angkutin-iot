#include "angkot.h"

#include <string.h>
#include "utils.h"
#include "http_wrapper.h"
#include "modem.h"
#include "rfid.h"

String driverUid;
String tripId;

String setupDriver() {
  infoIndicator("Tap RFID to setup driver", true, true);
  bool lightOn = false;

  while (driverUid == "" || tripId == "") {
    lightOn = !lightOn;
    turnLight(lightOn);

    if (rfidAvailable()) {
      String uid = rfidGetUid();
      infoIndicator("Driver UID: " + uid);

      publishUidMqtt(uid);
      sendUidHttp(uid);

      driverUid = uid;
      tripId = "123123123";

      turnLight(false);
      delay(2000);
      break;
    }

    delay(500);
  }

  return tripId;
}
