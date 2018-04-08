#include <nan.h>
#include <node.h>
#include <node_object_wrap.h>
#include <iostream>
#include <fstream>
#include "../library/json.hpp"
#include "snapshot_parser.h"

#ifndef _PARSER_H
#define _PARSER_H

namespace parser {
using nlohmann::json;

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
  static void GetNodeId(const Nan::FunctionCallbackInfo<v8::Value>& args);
  static void GetNodeByOrdinalId(const Nan::FunctionCallbackInfo<v8::Value>& args);
  static Nan::Persistent<v8::Function> constructor;
  // snapshot info
  int filelength_;
  char* filename_;
  snapshot_parser::SnapshotParser* snapshotParser;
};
}

#endif