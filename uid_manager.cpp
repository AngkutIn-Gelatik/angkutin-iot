#include "uid_manager.h"
#include "utils.h"

UidManager::UidManager(int maxUids) {
  _maxUids = maxUids;
  _uids = new String[_maxUids];
  _count = 0;
}

bool UidManager::add(const String& uid) {
  if (_count >= _maxUids) {
    errorIndicator("UID list full. Cannot add more.", true);
    return false;
  }

  if (exists(uid)) {
    errorIndicator("UID already scanned.", true);
    return false;
  }

  _uids[_count++] = uid;
  infoIndicator("UID added: " + uid);
  return true;
}

bool UidManager::remove(const String& uid) {
  int index = findIndex(uid);
  if (index == -1) {
    errorIndicator("UID not found. Cannot remove.");
    return false;
  }

  for (int i = index; i < _count - 1; ++i) {
    _uids[i] = _uids[i + 1];
  }

  _count--;
  _uids[_count] = "";
  infoIndicator("UID removed: " + uid);
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
  infoIndicator("Scanned UIDs:");
  for (int i = 0; i < _count; ++i) {
    infoIndicator("[" + String(i) + "] " + _uids[i]);
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
