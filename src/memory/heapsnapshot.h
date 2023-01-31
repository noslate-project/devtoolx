#include <nan.h>

#include <iostream>

#ifndef _HEAPSNAPSHOT_H_
#define _HEAPSNAPSHOT_H_

class HeapSnapshot {
 public:
  static void Init(v8::Local<v8::Object> exports);
};

#endif