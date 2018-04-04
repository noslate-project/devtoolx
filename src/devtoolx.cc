#include <nan.h>
#include "./memory/heapsnapshot.h"

void Init(v8::Local<v8::Object> exports) {
  HeapSnapshot::Init(exports);
}

NODE_MODULE(devtoolx, Init)