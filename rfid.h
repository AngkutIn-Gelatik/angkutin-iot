#ifndef RFID_H
#define RFID_H

#include <MFRC522v2.h>
#include <MFRC522DriverSPI.h>
#include <MFRC522DriverPinSimple.h>

void rfidInit();
bool rfidAvailable();
String rfidGetUid();

#endif
