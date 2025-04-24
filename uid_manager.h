#ifndef UID_MANAGER_H
#define UID_MANAGER_H

#include <Arduino.h>

class UidManager {
public:
  UidManager(int maxUids);

  bool add(const String& uid);
  bool remove(const String& uid);
  bool exists(const String& uid) const;
  void printAll() const;
  int size() const;
  String get(int index) const;

private:
  int _maxUids;
  String* _uids;
  int _count;

  int findIndex(const String& uid) const;
};

#endif
