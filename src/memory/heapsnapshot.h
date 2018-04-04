#include <nan.h>
#include <iostream>

class HeapSnapshot {
  public:
    static void Init(v8::Local<v8::Object> exports);
  private:
    std::string filename;
};