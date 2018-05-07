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
using v8::Boolean;
using snapshot_parser::snapshot_retainer_t;
using snapshot_parser::snapshot_dominates_t;
using snapshot_parser::snapshot_dominate_t;

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
  Nan::SetPrototypeMethod(tpl, "getNodeByAddress", GetNodeByAddress);
  Nan::SetPrototypeMethod(tpl, "getNodeIdByAddress", GetNodeIdByAddress);
  Nan::SetPrototypeMethod(tpl, "getStatistics", GetStatistics);
  Nan::SetPrototypeMethod(tpl, "getDominatorByIDom", GetDominatorByIDom);

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
  parser->snapshot_parser = new snapshot_parser::SnapshotParser(profile);
  if(info[0]->IsObject()) {
    Local<Object> options = info[0]->ToObject();
    Nan::Utf8String mode(options->Get(Nan::New<String>("mode").ToLocalChecked()));
    std::string mode_search = "search";
    if(strcmp(*mode, mode_search.c_str()) == 0) {
      parser->snapshot_parser->CreateAddressMap();
      parser->snapshot_parser->BuildTotalRetainer();
      parser->snapshot_parser->BuildDistances();
      parser->snapshot_parser->BuildDominatorTree();
    }
  }
}

Local<Object> Parser::SetNormalInfo_(int id) {
  Local<Object> node = Nan::New<Object>();
  node->Set(Nan::New<String>("id").ToLocalChecked(), Nan::New<Number>(id));
  std::string type = snapshot_parser->node_util->GetType(id, false);
  node->Set(Nan::New<String>("type").ToLocalChecked(), Nan::New<String>(type).ToLocalChecked());
  std::string name = snapshot_parser->node_util->GetName(id, false);
  node->Set(Nan::New<String>("name").ToLocalChecked(), Nan::New<String>(name).ToLocalChecked());
  std::string address = "@" + std::to_string(snapshot_parser->node_util->GetAddress(id, false));
  node->Set(Nan::New<String>("address").ToLocalChecked(), Nan::New<String>(address).ToLocalChecked());
  int self_size = snapshot_parser->node_util->GetSelfSize(id, false);
  node->Set(Nan::New<String>("self_size").ToLocalChecked(), Nan::New<Number>(self_size));
  int retained_size = snapshot_parser->GetRetainedSize(id);
  node->Set(Nan::New<String>("retained_size").ToLocalChecked(), Nan::New<Number>(retained_size));
  int distance = snapshot_parser->GetDistance(id);
  node->Set(Nan::New<String>("distance").ToLocalChecked(), Nan::New<Number>(distance));
  bool is_gcroot = snapshot_parser->IsGCRoot(id);
  node->Set(Nan::New<String>("is_gcroot").ToLocalChecked(), Nan::New<Number>(is_gcroot));
  return node;
}

Local<Object> Parser::GetNodeById_(int id, int current, int limit, GetNodeTypes get_node_type) {
  Local<Object> node = SetNormalInfo_(id);
  // get edges
  if(get_node_type == KALL || get_node_type == KEDGES) {
    int* edges_local = snapshot_parser->GetSortedEdges(id);
    int edges_length = snapshot_parser->node_util->GetEdgeCount(id, false);
    int start_edge_index = current;
    int stop_edge_index = current + limit;
    if(start_edge_index >= edges_length) {
      start_edge_index = edges_length;
    }
    if(stop_edge_index >= edges_length) {
      stop_edge_index = edges_length;
    }
    Local<Array> edges = Nan::New<Array>(stop_edge_index - start_edge_index);
    for(int i = start_edge_index; i < stop_edge_index; i++) {
      Local<Object> edge = Nan::New<Object>();
      std::string edge_type = snapshot_parser->edge_util->GetType(edges_local[i], true);
      std::string name_or_index = snapshot_parser->edge_util->GetNameOrIndex(edges_local[i], true);
      int to_node = snapshot_parser->edge_util->GetTargetNode(edges_local[i], true);
      int dominator = snapshot_parser->GetImmediateDominator(to_node);
      bool idomed = dominator == id;
      edge->Set(Nan::New<String>("type").ToLocalChecked(), Nan::New<String>(edge_type).ToLocalChecked());
      edge->Set(Nan::New<String>("name_or_index").ToLocalChecked(), Nan::New<String>(name_or_index).ToLocalChecked());
      edge->Set(Nan::New<String>("to_node").ToLocalChecked(), Nan::New<Number>(to_node));
      edge->Set(Nan::New<String>("idomed").ToLocalChecked(), Nan::New<Boolean>(idomed));
      edges->Set((i - start_edge_index), edge);
    }
    if(stop_edge_index < edges_length) {
      node->Set(Nan::New<String>("edges_end").ToLocalChecked(), Nan::New<Boolean>(false));
      node->Set(Nan::New<String>("edges_current").ToLocalChecked(), Nan::New<Number>(stop_edge_index));
      node->Set(Nan::New<String>("edges_left").ToLocalChecked(), Nan::New<Number>(edges_length - stop_edge_index));
    } else {
      node->Set(Nan::New<String>("edges_end").ToLocalChecked(), Nan::New<Boolean>(true));
    }
    node->Set(Nan::New<String>("edges").ToLocalChecked(), edges);
  }
  // get retainers
  if(get_node_type == KALL || get_node_type == KRETAINERS) {
    snapshot_retainer_t** retainers_local = snapshot_parser->GetRetainers(id);
    int retainers_length = snapshot_parser->GetRetainersCount(id);
    int start_retainer_index = current;
    int stop_retainer_index = current + limit;
    if(start_retainer_index >= retainers_length) {
      start_retainer_index = retainers_length;
    }
    if(stop_retainer_index >= retainers_length) {
      stop_retainer_index = retainers_length;
    }
    Local<Array> retainers = Nan::New<Array>(stop_retainer_index - start_retainer_index);
    for(int i = start_retainer_index; i < stop_retainer_index; i++) {
      snapshot_retainer_t* retainer_local = *(retainers_local + i);
      int node = retainer_local->ordinal;
      int edge = retainer_local->edge;
      Local<Object> retainer = Nan::New<Object>();
      std::string edge_type = snapshot_parser->edge_util->GetType(edge, true);
      std::string name_or_index = snapshot_parser->edge_util->GetNameOrIndex(edge, true);
      retainer->Set(Nan::New<String>("type").ToLocalChecked(), Nan::New<String>(edge_type).ToLocalChecked());
      retainer->Set(Nan::New<String>("name_or_index").ToLocalChecked(), Nan::New<String>(name_or_index).ToLocalChecked());
      retainer->Set(Nan::New<String>("from_node").ToLocalChecked(), Nan::New<Number>(node));
      retainers->Set((i - start_retainer_index), retainer);
    }
    if(stop_retainer_index < retainers_length) {
      node->Set(Nan::New<String>("retainers_end").ToLocalChecked(), Nan::New<Boolean>(false));
      node->Set(Nan::New<String>("retainers_current").ToLocalChecked(), Nan::New<Number>(stop_retainer_index));
      node->Set(Nan::New<String>("retainers_left").ToLocalChecked(), Nan::New<Number>(retainers_length - stop_retainer_index));
    } else {
      node->Set(Nan::New<String>("retainers_end").ToLocalChecked(), Nan::New<Boolean>(true));
    }
    node->Set(Nan::New<String>("retainers").ToLocalChecked(), retainers);
    // do not free snapshot_retainer_t** retainers_local because it'll be cached
    // delete[] retainers_local;
    // retainers_local = nullptr;
  }
  return node;
}

void Parser::GetNodeId(const Nan::FunctionCallbackInfo<Value>& info) {
  if(!info[0]->IsNumber()) {
    Nan::ThrowTypeError(Nan::New<String>("argument must be number!").ToLocalChecked());
    return;
  }
  int source = static_cast<int>(info[0]->ToInteger()->Value());
  Parser* parser = ObjectWrap::Unwrap<Parser>(info.Holder());
  int nodeid = parser->snapshot_parser->node_util->GetNodeId(source);
  info.GetReturnValue().Set(Nan::New<Number>(nodeid));
}

void Parser::GetNodeByOrdinalId(const Nan::FunctionCallbackInfo<Value>& info) {
  if(!info[0]->IsArray()) {
    Nan::ThrowTypeError(Nan::New<String>("argument 0 must be array!").ToLocalChecked());
    return;
  }
  Local<Object> list = info[0]->ToObject();
  Parser* parser = ObjectWrap::Unwrap<Parser>(info.Holder());
  int length = list->Get(Nan::New<String>("length").ToLocalChecked())->ToInteger()->Value();
  Local<Array> nodes = Nan::New<Array>(length);
  int current = 0;
  if(info[1]->IsNumber()) {
    current = static_cast<int>(info[1]->ToInteger()->Value());
  }
  int limit = 0;
  if(info[2]->IsNumber()) {
    limit = static_cast<int>(info[2]->ToInteger()->Value());
  }
  GetNodeTypes type = KALL;
  if(!info[3]->IsUndefined() && !info[3]->IsNull()) {
    Local<Object> options = info[3]->ToObject();
    Local<String> raw_option = options->Get(Nan::New<String>("type").ToLocalChecked())->ToString();
    std::string type_edges = "edges";
    std::string type_retainers = "retainers";
    Nan::Utf8String option_type(raw_option);
    if(strcmp(*option_type, type_edges.c_str()) == 0) {
      type = KEDGES;
    }
    if(strcmp(*option_type, type_retainers.c_str()) == 0) {
      type = KRETAINERS;
    }
  }
  int error_id_count = 0;
  for(int i = 0; i < length; i++) {
    int id = static_cast<int>(list->Get(Nan::New<Number>(i))->ToInteger()->Value());
    if(id >= parser->snapshot_parser->node_count) {
      error_id_count++;
      break;
    }
    if(!info[2]->IsNumber()) {
      limit = parser->snapshot_parser->node_util->GetEdgeCount(id, false);
    }
    nodes->Set(i, parser->GetNodeById_(id, current, limit, type));
  }
  if(error_id_count > 0) {
    Nan::ThrowTypeError(Nan::New<String>("node ordinal id wrong!").ToLocalChecked());
    return;
  }
  info.GetReturnValue().Set(nodes);
}

void Parser::GetNodeByAddress(const Nan::FunctionCallbackInfo<Value>& info) {
  if(!info[0]->IsString()) {
    Nan::ThrowTypeError(Nan::New<String>("argument 0 must be string!").ToLocalChecked());
    return;
  }
  Parser* parser = ObjectWrap::Unwrap<Parser>(info.Holder());
  Nan::Utf8String addr(info[0]->ToString());
  char start = '@';
  if(strncmp(*addr, &start, 1) != 0) {
    Nan::ThrowTypeError(Nan::New<String>("argument 0 must be startwith \"@\"!").ToLocalChecked());
    return;
  }
  int id = parser->snapshot_parser->SearchOrdinalByAddress(atoi((*addr) + 1));
  if(id == -1) {
    std::string addrs = *addr;
    std::string error = "address \"" + addrs + "\" is wrong!";
    Nan::ThrowTypeError(Nan::New<String>(error).ToLocalChecked());
    return;
  }
  int current = 0;
  if(info[1]->IsNumber()) {
    current = static_cast<int>(info[1]->ToInteger()->Value());
  }
  int limit = parser->snapshot_parser->node_util->GetEdgeCount(id, false);
  if(info[2]->IsNumber()) {
    limit = static_cast<int>(info[2]->ToInteger()->Value());
  }
  Local<Object> node = parser->GetNodeById_(id, current, limit, KALL);
  info.GetReturnValue().Set(node);
}

void Parser::GetNodeIdByAddress(const Nan::FunctionCallbackInfo<Value>& info) {
  if(!info[0]->IsString()) {
    Nan::ThrowTypeError(Nan::New<String>("argument must be string!").ToLocalChecked());
    return;
  }
  Nan::Utf8String addr(info[0]->ToString());
  char start = '@';
  if(strncmp(*addr, &start, 1) != 0) {
    Nan::ThrowTypeError(Nan::New<String>("argument 0 must be startwith \"@\"!").ToLocalChecked());
    return;
  }
  Parser* parser = ObjectWrap::Unwrap<Parser>(info.Holder());
  int id = parser->snapshot_parser->SearchOrdinalByAddress(atoi((*addr) + 1));
  if(id == -1) {
    std::string addrs = *addr;
    std::string error = "address \"" + addrs + "\" is wrong!";
    Nan::ThrowTypeError(Nan::New<String>(error).ToLocalChecked());
    return;
  }
  info.GetReturnValue().Set(Nan::New<Number>(id));
}

void Parser::GetFileName(const Nan::FunctionCallbackInfo<Value>& info) {
  Parser* parser = ObjectWrap::Unwrap<Parser>(info.Holder());
  info.GetReturnValue().Set(Nan::New(parser->filename_).ToLocalChecked());
}

void Parser::GetStatistics(const Nan::FunctionCallbackInfo<Value>& info) {
  Parser* parser = ObjectWrap::Unwrap<Parser>(info.Holder());
  Local<Object> statistics = Nan::New<Object>();
  statistics->Set(Nan::New<String>("node_count").ToLocalChecked(), Nan::New<Number>(parser->snapshot_parser->node_count));
  statistics->Set(Nan::New<String>("edge_count").ToLocalChecked(), Nan::New<Number>(parser->snapshot_parser->edge_count));
  statistics->Set(Nan::New<String>("gcroots").ToLocalChecked(), Nan::New<Number>(parser->snapshot_parser->gcroots));
  statistics->Set(Nan::New<String>("total_size").ToLocalChecked(), Nan::New<Number>(parser->snapshot_parser->GetRetainedSize(parser->snapshot_parser->root_index)));
  info.GetReturnValue().Set(statistics);
}

void Parser::GetDominatorByIDom(const Nan::FunctionCallbackInfo<v8::Value>& info) {
  if(!info[0]->IsNumber()) {
    Nan::ThrowTypeError(Nan::New<String>("argument 0 must be number!").ToLocalChecked());
    return;
  }
  Parser* parser = ObjectWrap::Unwrap<Parser>(info.Holder());
  snapshot_dominates_t* ptr =
    parser->snapshot_parser->GetSortedDominates(info[0]->ToInteger()->Value());
  int current = 0;
  if(info[1]->IsNumber())
    current = static_cast<int>(info[1]->ToInteger()->Value());
  int limit = 0;
  if(info[2]->IsNumber())
    limit = static_cast<int>(info[2]->ToInteger()->Value());
  else
    limit = ptr->length;
  if(current >= ptr->length)
    current = ptr->length;
  int end = current + limit;
  if(end >= ptr->length)
    end = ptr->length;
  Local<Array> dominates = Nan::New<Array>(end - current);
  for(int i = current; i < end; i++) {
    snapshot_dominate_t* sdom = *(ptr->dominates + i);
    int id = sdom->dominate;
    Local<Object> node = parser->SetNormalInfo_(id);
    if(sdom->edge != -1) {
      std::string edge_name_or_index = parser->snapshot_parser->edge_util->GetNameOrIndex(sdom->edge, true);
      node->Set(Nan::New<String>("edge_name_or_index").ToLocalChecked(), Nan::New<String>(edge_name_or_index).ToLocalChecked());
      std::string edge_type = parser->snapshot_parser->edge_util->GetType(sdom->edge, true);
      node->Set(Nan::New<String>("edge_type").ToLocalChecked(), Nan::New<String>(edge_type).ToLocalChecked());
    }
    dominates->Set((i - current), node);
  }
  Local<Object> result = Nan::New<Object>();
  result->Set(Nan::New<String>("dominates").ToLocalChecked(), dominates);
  if(end < ptr->length) {
    result->Set(Nan::New<String>("dominates_end").ToLocalChecked(), Nan::New<Boolean>(false));
    result->Set(Nan::New<String>("dominates_current").ToLocalChecked(), Nan::New<Number>(end));
    result->Set(Nan::New<String>("dominates_left").ToLocalChecked(), Nan::New<Number>(ptr->length - end));
  } else
    result->Set(Nan::New<String>("dominates_end").ToLocalChecked(), Nan::New<Boolean>(true));
  info.GetReturnValue().Set(result);
}
}