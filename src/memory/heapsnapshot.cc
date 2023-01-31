#include "parser.h"
#include "heapsnapshot.h"

using v8::Local;
using v8::Object;
void GetName(const Nan::FunctionCallbackInfo<v8::Value> &info)
{
  Local<v8::String> name = Nan::New("HeapTools").ToLocalChecked();
  info.GetReturnValue().Set(name);
}

void HeapSnapshot::Init(Local<Object> exports)
{
  Local<Object> heapTools = Nan::New<Object>();
  Nan::Set(heapTools, Nan::New<v8::String>("getName").ToLocalChecked(), Nan::GetFunction(Nan::New<v8::FunctionTemplate>(GetName)).ToLocalChecked());
  parser::Parser::Init(heapTools);
  Nan::Set(exports, Nan::New<v8::String>("heapTools").ToLocalChecked(), heapTools);
}