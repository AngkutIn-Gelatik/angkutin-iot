#include "uid_manager.h"

UidManager::UidManager(int maxUids) {
  _maxUids = maxUids;
  _uids = new String[_maxUids];
  _count = 0;
}

bool UidManager::add(const String& uid) {
  if (_count >= _maxUids) {
    Serial.println("UID list full. Cannot add more.");
    return false;
  }

  if (exists(uid)) {
    Serial.println("UID already scanned.");
    return false;
  }

  _uids[_count++] = uid;
  Serial.println("UID added: " + uid);
  return true;
}

bool UidManager::remove(const String& uid) {
  int index = findIndex(uid);
  if (index == -1) {
    Serial.println("UID not found. Cannot remove.");
    return false;
  }

  for (int i = index; i < _count - 1; ++i) {
    _uids[i] = _uids[i + 1];
  }

  _count--;
  _uids[_count] = "";
  Serial.println("UID removed: " + uid);
  return true;
}

bool UidManager::exists(const String& uid) const {
  return findIndex(uid) != -1;
}

int UidManager::findIndex(const String& uid) const {
  for (int i = 0; i < _count; ++i) {
    if (_uids[i] == uid) {
      return i;
    }
  }
  return -1;
}

void UidManager::printAll() const {
  Serial.println("Scanned UIDs:");
  for (int i = 0; i < _count; ++i) {
    Serial.println("[" + String(i) + "] " + _uids[i]);
  }
}

int UidManager::size() const {
  return _count;
}

String UidManager::get(int index) const {
  if (index >= 0 && index < _count) {
    return _uids[index];
  }
  return "";
}
