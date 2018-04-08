#include <node.h>
#include "parser.h"

namespace parser {
using v8::Local;
using v8::Object;
using v8::String;
using v8::FunctionTemplate;
using v8::Function;
using v8::Value;
using v8::Array;
using v8::Number;

Nan::Persistent<Function> Parser::constructor;

Parser::Parser(char* filename, int filelength) {
  filelength_ = filelength;
  filename_ = (char*)malloc(filelength);
  strncpy(filename_, filename, filelength);
}

Parser::~Parser() {}

void Parser::Init(Local<Object> exports) {
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  tpl->SetClassName(Nan::New<String>("V8Parser").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetPrototypeMethod(tpl, "getFileName", GetFileName);
  Nan::SetPrototypeMethod(tpl, "parse", Parse);
  Nan::SetPrototypeMethod(tpl, "getNodeId", GetNodeId);
  Nan::SetPrototypeMethod(tpl, "getNodeByOrdinalId", GetNodeByOrdinalId);

  constructor.Reset(tpl->GetFunction());
  exports->Set(Nan::New("V8Parser").ToLocalChecked(), tpl->GetFunction());
}

void Parser::New(const Nan::FunctionCallbackInfo<Value>& info) {
  int argumentLength = info.Length();
  if(argumentLength < 1) {
    printf("constrctor arguments must >= 1");
    info.GetReturnValue().Set(Nan::Undefined());
    return;
  }
  // new instance
  if (info.IsConstructCall()) {
    Nan::Utf8String filename(info[0]->ToString());
    // last position for "\0"
    int length = strlen(*filename) + 1;
    Parser* parser = new Parser(*filename, length);
    parser->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
  } else {
    const int argc = 1;
    Local<Value> argv[argc] = { info[0] };
    Local<Function> cons = Nan::New<Function>(constructor);
    info.GetReturnValue().Set((Nan::NewInstance(cons, argc, argv)).ToLocalChecked());
  }
}

void Parser::Parse(const Nan::FunctionCallbackInfo<Value>& info) {
  Parser* parser = ObjectWrap::Unwrap<Parser>(info.Holder());
  // get json from heapsnapshot
  std::ifstream jsonfile(parser->filename_);
  json profile;
  jsonfile >> profile;
  jsonfile.close();
  // get snapshot parser
  parser->snapshotParser = new snapshot_parser::SnapshotParser(profile);
}

void Parser::GetNodeId(const Nan::FunctionCallbackInfo<Value>& info) {
  if(!info[0]->IsNumber()) {
    Nan::ThrowTypeError(Nan::New<String>("argument must be number!").ToLocalChecked());
  }
  int source = info[0]->ToInteger()->Value();
  Parser* parser = ObjectWrap::Unwrap<Parser>(info.Holder());
  int nodeid = parser->snapshotParser->node_util->GetNodeId(source);
  info.GetReturnValue().Set(Nan::New(nodeid));
}

void Parser::GetNodeByOrdinalId(const Nan::FunctionCallbackInfo<Value>& info) {
  if(!info[0]->IsNumber()) {
    Nan::ThrowTypeError(Nan::New<String>("argument must be number!").ToLocalChecked());
  }
  int id = info[0]->ToInteger()->Value();
  Parser* parser = ObjectWrap::Unwrap<Parser>(info.Holder());
  Local<Object> node = Nan::New<Object>();
  std::string type = parser->snapshotParser->node_util->GetType(id, false);
  std::string name = parser->snapshotParser->node_util->GetName(id, false);
  std::string address = "@" + std::to_string(parser->snapshotParser->node_util->GetAddress(id, false));
  int self_size = parser->snapshotParser->node_util->GetSelfSize(id, false);
  int* edges_local = parser->snapshotParser->node_util->GetEdges(id, false);
  int edges_length = parser->snapshotParser->node_util->GetEdgeCount(id, false);
  Local<Array> edges = Nan::New<Array>(edges_length);
  for(int i = 0; i < edges_length; i++) {
    Local<Object> edge = Nan::New<Object>();
    std::string edge_type = parser->snapshotParser->edge_util->GetType(edges_local[i], true);
    std::string name_or_index = parser->snapshotParser->edge_util->GetNameOrIndex(edges_local[i], true);
    int to_node = parser->snapshotParser->edge_util->GetTargetNode(edges_local[i], true);
    edge->Set(Nan::New<String>("type").ToLocalChecked(), Nan::New<String>(edge_type).ToLocalChecked());
    edge->Set(Nan::New<String>("name_or_index").ToLocalChecked(), Nan::New<String>(name_or_index).ToLocalChecked());
    edge->Set(Nan::New<String>("to_node").ToLocalChecked(), Nan::New<Number>(to_node));
    edges->Set(i, edge);
  }
  node->Set(Nan::New<String>("type").ToLocalChecked(), Nan::New<String>(type).ToLocalChecked());
  node->Set(Nan::New<String>("name").ToLocalChecked(), Nan::New<String>(name).ToLocalChecked());
  node->Set(Nan::New<String>("address").ToLocalChecked(), Nan::New<String>(address).ToLocalChecked());
  node->Set(Nan::New<String>("self_size").ToLocalChecked(), Nan::New<Number>(self_size));
  node->Set(Nan::New<String>("edges").ToLocalChecked(), edges);
  info.GetReturnValue().Set(node);
}

void Parser::GetFileName(const Nan::FunctionCallbackInfo<Value>& info) {
  Parser* parser = ObjectWrap::Unwrap<Parser>(info.Holder());
  info.GetReturnValue().Set(Nan::New(parser->filename_).ToLocalChecked());
}
}