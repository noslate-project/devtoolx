#include <node.h>
#include "./parser.h"

namespace parser {

  using v8::Local;
  using v8::Object;
  using v8::String;
  using v8::FunctionTemplate;
  using v8::Function;
  using v8::Value;
  using nlohmann::json;

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

    constructor.Reset(tpl->GetFunction());
    exports->Set(Nan::New("V8Parser").ToLocalChecked(), tpl->GetFunction());
  }

  void Parser::New(const Nan::FunctionCallbackInfo<Value>& info){
    int argumentLength = info.Length();
    if(argumentLength < 1){
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
    json jso;
    jsonfile >> jso;
    json nodes = jso["nodes"];
    printf("array length: %lu\n", nodes.size());
  }

  void Parser::GetFileName(const Nan::FunctionCallbackInfo<Value>& info) {
    Parser* parser = ObjectWrap::Unwrap<Parser>(info.Holder());
    info.GetReturnValue().Set(Nan::New(parser->filename_).ToLocalChecked());
  }
}