#include "parser.h"
#include "heapsnapshot.h"

using v8::Local;
using v8::Object;
void GetName(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  Local<v8::String> name = Nan::New("HeapTools").ToLocalChecked();
  info.GetReturnValue().Set(name);
}

void HeapSnapshot::Init(Local<Object> exports) {
  Local<Object> heapTools = Nan::New<Object>();
  heapTools->Set(Nan::New("getName").ToLocalChecked(),
                 Nan::New<v8::FunctionTemplate>(GetName)->GetFunction());

  parser::Parser::Init(heapTools);

  exports->Set(Nan::New("heapTools").ToLocalChecked(), heapTools);
}