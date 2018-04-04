#include <nan.h>
#include <node.h>
#include <node_object_wrap.h>
#include <iostream>
#include <fstream>
#include "../library/json.hpp"

namespace parser {
  class Parser : public node::ObjectWrap {
   public:
    static void Init(v8::Local<v8::Object> exports);
    static void NewInstance(const Nan::FunctionCallbackInfo<v8::Value>& args);
 
   private:
    explicit Parser(char* filename, int filelength);
    ~Parser();
 
    static void New(const Nan::FunctionCallbackInfo<v8::Value>& args);
    static void GetFileName(const Nan::FunctionCallbackInfo<v8::Value>& args);
    static void Parse(const Nan::FunctionCallbackInfo<v8::Value>& args);
    static Nan::Persistent<v8::Function> constructor;
    int filelength_;
    char* filename_;
  };
}