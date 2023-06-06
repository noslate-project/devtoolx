#include <node.h>
#include "parser.h"

namespace parser
{
  using snapshot_parser::DOMINATOR_ALGORITHM;
  using snapshot_parser::snapshot_dominate_t;
  using snapshot_parser::snapshot_dominates_t;
  using snapshot_parser::snapshot_retainer_t;
  using v8::Array;
  using v8::Boolean;
  using v8::Function;
  using v8::FunctionTemplate;
  using v8::Local;
  using v8::Number;
  using v8::Object;
  using v8::String;
  using v8::Value;

  Nan::Persistent<Function> Parser::constructor;

  Parser::Parser(char *filename, int filelength)
  {
    filelength_ = filelength;
    filename_ = (char *)malloc(filelength);
    strncpy(filename_, filename, filelength);
  }

  Parser::~Parser() {}

  void Parser::Init(Local<Object> exports)
  {
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
    Nan::SetPrototypeMethod(tpl, "getChildRepeat", GetChildRepeat);
    Nan::SetPrototypeMethod(tpl, "getConsStringName", GetConsStringName);

    constructor.Reset(Nan::GetFunction(tpl).ToLocalChecked());
    Nan::Set(exports, Nan::New<v8::String>("V8Parser").ToLocalChecked(), Nan::GetFunction(tpl).ToLocalChecked());
  }

  void Parser::New(const Nan::FunctionCallbackInfo<Value> &info)
  {
    if (!info[0]->IsString())
    {
      Nan::ThrowTypeError(Nan::New<String>("constrctor arguments 0 must be string!").ToLocalChecked());
      info.GetReturnValue().Set(Nan::Undefined());
      return;
    }
    // new instance
    if (info.IsConstructCall())
    {
      Nan::Utf8String filename(Nan::To<v8::String>(info[0]).ToLocalChecked());
      // last position for "\0"
      int length = static_cast<int>(strlen(*filename)) + 1;
      Parser *parser = new Parser(*filename, length);
      parser->Wrap(info.This());
      info.GetReturnValue().Set(info.This());
    }
    else
    {
      const int argc = 1;
      Local<Value> argv[argc] = {info[0]};
      Local<Function> cons = Nan::New<Function>(constructor);
      info.GetReturnValue().Set((Nan::NewInstance(cons, argc, argv)).ToLocalChecked());
    }
  }

  void Print(Local<Value> cb, std::string type, std::string status, int64_t cost)
  {
    if (cb->IsFunction())
    {
      Local<Object> progress = Nan::New<Object>();
      Nan::Set(progress, Nan::New<v8::String>("type").ToLocalChecked(), Nan::New<String>(type).ToLocalChecked());
      Nan::Set(progress, Nan::New<v8::String>("status").ToLocalChecked(), Nan::New<String>(status).ToLocalChecked());
      Nan::Set(progress, Nan::New<v8::String>("cost").ToLocalChecked(), Nan::New<Number>(cost / 10e5));
      Local<Value> argv[1] = {progress};
      Nan::Call(cb.As<Function>(), Nan::New<Object>(), 1, argv);
    }
  }

  void Parser::Parse(const Nan::FunctionCallbackInfo<Value> &info)
  {
    Parser *parser = ObjectWrap::Unwrap<Parser>(info.Holder());
    // get json from heapsnapshot
    Print(info[1], "json parse", "start", 0);
    int64_t jsonparse_start = uv_hrtime();
    std::ifstream jsonfile(parser->filename_);
    if (!jsonfile.is_open()) {
      std::cout << "\nfailed to open " << parser->filename_ << '\n';
      std::cerr << "ParseError: " << strerror(errno);
      std::exit(1);
      return;
    }
    json profile;
    jsonfile >> profile;
    jsonfile.close();
    Print(info[1], "json parse", "end", uv_hrtime() - jsonparse_start);
    // get snapshot parser
    Print(info[1], "snapshot parser init", "start", 0);
    int64_t snapshot_parser_init_start = uv_hrtime();
    parser->snapshot_parser = new snapshot_parser::SnapshotParser(profile);
    Print(info[1], "snapshot parser init", "end", uv_hrtime() - snapshot_parser_init_start);
    if (info[0]->IsObject())
    {
      Local<Object> options = Nan::To<v8::Object>(info[0]).ToLocalChecked();
      // chose algorithm
      Local<v8::Value> method = Nan::Get(options, Nan::New<v8::String>("method").ToLocalChecked()).ToLocalChecked();
      Nan::Utf8String method_s(Nan::To<v8::String>(method).ToLocalChecked());
      std::string tarjan = "tarjan";
      std::string data_iteration = "data_iteration";
      if (strcmp(*method_s, tarjan.c_str()) == 0)
      {
        parser->snapshot_parser->SetAlgorithm(DOMINATOR_ALGORITHM::TARJAN);
      }
      if (strcmp(*method_s, data_iteration.c_str()) == 0)
      {
        parser->snapshot_parser->SetAlgorithm(DOMINATOR_ALGORITHM::DATA_ITERATION);
      }
      // get result
      Local<v8::Value> mode = Nan::Get(options, Nan::New<v8::String>("mode").ToLocalChecked()).ToLocalChecked();
      Nan::Utf8String mode_s(Nan::To<v8::String>(mode).ToLocalChecked());
      std::string mode_search = "search";
      if (strcmp(*mode_s, mode_search.c_str()) == 0)
      {
        Print(info[1], "get address map", "start", 0);
        int64_t get_address_map_start = uv_hrtime();
        parser->snapshot_parser->CreateAddressMap();
        Print(info[1], "get address map", "end", uv_hrtime() - get_address_map_start);
        Print(info[1], "build retainer", "start", 0);
        int64_t build_retainer_start = uv_hrtime();
        parser->snapshot_parser->BuildTotalRetainer();
        Print(info[1], "build retainer", "end", uv_hrtime() - build_retainer_start);
        Print(info[1], "build distance", "start", 0);
        int64_t build_distance_start = uv_hrtime();
        parser->snapshot_parser->BuildDistances();
        Print(info[1], "build distance", "end", uv_hrtime() - build_distance_start);
        Print(info[1], "build dominator tree", "start", 0);
        int64_t build_dominator_tree_start = uv_hrtime();
        parser->snapshot_parser->BuildDominatorTree();
        Print(info[1], "build dominator tree", "end", uv_hrtime() - build_dominator_tree_start);
      }
    }
  }

  Local<Object> Parser::SetNormalInfo_(int id)
  {
    Local<Object> node = Nan::New<Object>();
    if (!snapshot_parser->node_util->CheckOrdinalId(id))
      return node;
    Nan::Set(node, Nan::New<v8::String>("id").ToLocalChecked(), Nan::New<Number>(id));
    std::string type = snapshot_parser->node_util->GetType(id);
    Nan::Set(node, Nan::New<v8::String>("type").ToLocalChecked(), Nan::New<String>(type).ToLocalChecked());
    int type_int = snapshot_parser->node_util->GetTypeForInt(id);
    std::string name;
    if (type_int == snapshot_node::NodeTypes::KCONCATENATED_STRING)
      name = snapshot_parser->node_util->GetConsStringName(id);
    else
      name = snapshot_parser->node_util->GetName(id);
    Nan::Set(node, Nan::New<v8::String>("name").ToLocalChecked(), Nan::New<String>(name).ToLocalChecked());
    std::string address = "@" + std::to_string(snapshot_parser->node_util->GetAddress(id));
    Nan::Set(node, Nan::New<v8::String>("address").ToLocalChecked(), Nan::New<String>(address).ToLocalChecked());
    int self_size = snapshot_parser->node_util->GetSelfSize(id);
    Nan::Set(node, Nan::New<v8::String>("self_size").ToLocalChecked(), Nan::New<Number>(self_size));
    int retained_size = snapshot_parser->GetRetainedSize(id);
    Nan::Set(node, Nan::New<v8::String>("retained_size").ToLocalChecked(), Nan::New<Number>(retained_size));
    int distance = snapshot_parser->GetDistance(id);
    Nan::Set(node, Nan::New<v8::String>("distance").ToLocalChecked(), Nan::New<Number>(distance));
    bool is_gcroot = snapshot_parser->IsGCRoot(id);
    Nan::Set(node, Nan::New<v8::String>("is_gcroot").ToLocalChecked(), Nan::New<Number>(is_gcroot));
    snapshot_dominates_t *dominates = snapshot_parser->GetSortedDominates(id);
    Nan::Set(node, Nan::New<v8::String>("dominates_count").ToLocalChecked(), Nan::New<Number>(dominates->length));
    return node;
  }

  Local<Object> Parser::GetNodeById_(int id, int current, int limit, GetNodeTypes get_node_type)
  {
    Local<Object> node = SetNormalInfo_(id);
    // get edges
    if (get_node_type == KALL || get_node_type == KEDGES)
    {
      if (!snapshot_parser->node_util->CheckOrdinalId(id))
      {
        Nan::ThrowTypeError(Nan::New<String>("argument 0 is wrong!").ToLocalChecked());
        return node;
      }
      int *edges_local = snapshot_parser->GetSortedEdges(id);
      int edges_length = snapshot_parser->node_util->GetEdgeCount(id);
      int start_edge_index = current;
      int stop_edge_index = current + limit;
      if (start_edge_index >= edges_length)
      {
        start_edge_index = edges_length;
      }
      if (stop_edge_index >= edges_length)
      {
        stop_edge_index = edges_length;
      }
      Local<Array> edges = Nan::New<Array>(stop_edge_index - start_edge_index);
      for (int i = start_edge_index; i < stop_edge_index; i++)
      {
        Local<Object> edge = Nan::New<Object>();
        std::string edge_type = snapshot_parser->edge_util->GetType(edges_local[i], true);
        std::string name_or_index = snapshot_parser->edge_util->GetNameOrIndex(edges_local[i], true);
        int to_node = snapshot_parser->edge_util->GetTargetNode(edges_local[i], true);
        int dominator = snapshot_parser->GetImmediateDominator(to_node);
        bool idomed = dominator == id;
        Nan::Set(edge, Nan::New<v8::String>("type").ToLocalChecked(), Nan::New<String>(edge_type).ToLocalChecked());
        Nan::Set(edge, Nan::New<v8::String>("name_or_index").ToLocalChecked(), Nan::New<String>(name_or_index).ToLocalChecked());
        Nan::Set(edge, Nan::New<v8::String>("to_node").ToLocalChecked(), Nan::New<Number>(to_node));
        Nan::Set(edge, Nan::New<v8::String>("idomed").ToLocalChecked(), Nan::New<Boolean>(idomed));
        Nan::Set(edge, Nan::New<v8::String>("index").ToLocalChecked(), Nan::New<Number>(i));
        Nan::Set(edges, (i - start_edge_index), edge);
      }
      if (stop_edge_index < edges_length)
      {
        Nan::Set(node, Nan::New<v8::String>("edges_end").ToLocalChecked(), Nan::New<Boolean>(false));
        Nan::Set(node, Nan::New<v8::String>("edges_current").ToLocalChecked(), Nan::New<Number>(stop_edge_index));
        Nan::Set(node, Nan::New<v8::String>("edges_left").ToLocalChecked(), Nan::New<Number>(edges_length - stop_edge_index));
      }
      else
      {
        Nan::Set(node, Nan::New<v8::String>("edges_end").ToLocalChecked(), Nan::New<Boolean>(true));
      }
      Nan::Set(node, Nan::New<v8::String>("edges").ToLocalChecked(), edges);
    }
    // get retainers
    if (get_node_type == KALL || get_node_type == KRETAINERS)
    {
      snapshot_retainer_t **retainers_local = snapshot_parser->GetRetainers(id);
      int retainers_length = snapshot_parser->GetRetainersCount(id);
      int start_retainer_index = current;
      int stop_retainer_index = current + limit;
      if (start_retainer_index >= retainers_length)
      {
        start_retainer_index = retainers_length;
      }
      if (stop_retainer_index >= retainers_length)
      {
        stop_retainer_index = retainers_length;
      }
      Local<Array> retainers = Nan::New<Array>(stop_retainer_index - start_retainer_index);
      for (int i = start_retainer_index; i < stop_retainer_index; i++)
      {
        snapshot_retainer_t *retainer_local = *(retainers_local + i);
        int node = retainer_local->ordinal;
        int edge = retainer_local->edge;
        Local<Object> retainer = Nan::New<Object>();
        std::string edge_type = snapshot_parser->edge_util->GetType(edge, true);
        std::string name_or_index = snapshot_parser->edge_util->GetNameOrIndex(edge, true);
        Nan::Set(retainer, Nan::New<v8::String>("type").ToLocalChecked(), Nan::New<String>(edge_type).ToLocalChecked());
        Nan::Set(retainer, Nan::New<v8::String>("name_or_index").ToLocalChecked(), Nan::New<String>(name_or_index).ToLocalChecked());
        Nan::Set(retainer, Nan::New<v8::String>("from_node").ToLocalChecked(), Nan::New<Number>(node));
        Nan::Set(retainer, Nan::New<v8::String>("index").ToLocalChecked(), Nan::New<Number>(i));
        Nan::Set(retainers, (i - start_retainer_index), retainer);
      }
      if (stop_retainer_index < retainers_length)
      {
        Nan::Set(node, Nan::New<v8::String>("retainers_end").ToLocalChecked(), Nan::New<Boolean>(false));
        Nan::Set(node, Nan::New<v8::String>("retainers_current").ToLocalChecked(), Nan::New<Number>(stop_retainer_index));
        Nan::Set(node, Nan::New<v8::String>("retainers_left").ToLocalChecked(), Nan::New<Number>(retainers_length - stop_retainer_index));
      }
      else
      {
        Nan::Set(node, Nan::New<v8::String>("retainers_end").ToLocalChecked(), Nan::New<Boolean>(true));
      }
      Nan::Set(node, Nan::New<v8::String>("retainers").ToLocalChecked(), retainers);
      // do not free snapshot_retainer_t** retainers_local because it'll be cached
      // delete[] retainers_local;
      // retainers_local = nullptr;
    }
    return node;
  }

  void Parser::GetNodeId(const Nan::FunctionCallbackInfo<Value> &info)
  {
    if (!info[0]->IsNumber())
    {
      Nan::ThrowTypeError(Nan::New<String>("argument must be number!").ToLocalChecked());
      return;
    }
    int source = static_cast<int>(Nan::To<uint32_t>(info[0]).ToChecked());
    Parser *parser = ObjectWrap::Unwrap<Parser>(info.Holder());
    int nodeid = parser->snapshot_parser->node_util->GetNodeId(source);
    info.GetReturnValue().Set(Nan::New<Number>(nodeid));
  }

  void Parser::GetNodeByOrdinalId(const Nan::FunctionCallbackInfo<Value> &info)
  {
    if (!info[0]->IsArray())
    {
      Nan::ThrowTypeError(Nan::New<String>("argument 0 must be array!").ToLocalChecked());
      return;
    }
    Local<Object> list = Nan::To<v8::Object>(info[0]).ToLocalChecked();
    Parser *parser = ObjectWrap::Unwrap<Parser>(info.Holder());

    int length = static_cast<int>(Nan::To<uint32_t>(Nan::Get(list, Nan::New<v8::String>("length").ToLocalChecked()).ToLocalChecked()).ToChecked());
    Local<Array> nodes = Nan::New<Array>(length);
    int current = 0;
    if (info[1]->IsNumber())
    {
      current = static_cast<int>(Nan::To<uint32_t>(info[1]).ToChecked());
    }
    int limit = 0;
    if (info[2]->IsNumber())
    {
      limit = static_cast<int>(Nan::To<uint32_t>(info[2]).ToChecked());
    }
    GetNodeTypes type = KALL;
    if (!info[3]->IsUndefined() && !info[3]->IsNull())
    {
      Local<Object> options = Nan::To<v8::Object>(info[3]).ToLocalChecked();
      Local<String> raw_option = Nan::To<v8::String>(Nan::Get(options, Nan::New<v8::String>("type").ToLocalChecked()).ToLocalChecked()).ToLocalChecked();
      std::string type_edges = "edges";
      std::string type_retainers = "retainers";
      Nan::Utf8String option_type(raw_option);
      if (strcmp(*option_type, type_edges.c_str()) == 0)
      {
        type = KEDGES;
      }
      if (strcmp(*option_type, type_retainers.c_str()) == 0)
      {
        type = KRETAINERS;
      }
    }
    int error_id_count = 0;
    for (int i = 0; i < length; i++)
    {
      int id = static_cast<int>(Nan::To<uint32_t>(Nan::Get(list, Nan::New<v8::Number>(i)).ToLocalChecked()).ToChecked());
      if (id >= parser->snapshot_parser->node_count)
      {
        error_id_count++;
        break;
      }
      if (!info[2]->IsNumber())
      {
        limit = parser->snapshot_parser->node_util->GetEdgeCount(id);
      }
      Nan::Set(nodes, i, parser->GetNodeById_(id, current, limit, type));
    }
    if (error_id_count > 0)
    {
      Nan::ThrowTypeError(Nan::New<String>("node ordinal id wrong!").ToLocalChecked());
      return;
    }
    info.GetReturnValue().Set(nodes);
  }

  void Parser::GetNodeByAddress(const Nan::FunctionCallbackInfo<Value> &info)
  {
    if (!info[0]->IsString())
    {
      Nan::ThrowTypeError(Nan::New<String>("argument 0 must be string!").ToLocalChecked());
      return;
    }
    Parser *parser = ObjectWrap::Unwrap<Parser>(info.Holder());
    Nan::Utf8String addr(Nan::To<v8::String>(info[0]).ToLocalChecked());
    char start = '@';
    if (strncmp(*addr, &start, 1) != 0)
    {
      Nan::ThrowTypeError(Nan::New<String>("argument 0 must be startwith \"@\"!").ToLocalChecked());
      return;
    }
    int id = parser->snapshot_parser->SearchOrdinalByAddress(atoi((*addr) + 1));
    if (id == -1)
    {
      std::string addrs = *addr;
      std::string error = "address \"" + addrs + "\" is wrong!";
      Nan::ThrowTypeError(Nan::New<String>(error).ToLocalChecked());
      return;
    }
    int current = 0;
    if (info[1]->IsNumber())
    {
      current = static_cast<int>(Nan::To<uint32_t>(info[1]).ToChecked());
    }
    int limit = parser->snapshot_parser->node_util->GetEdgeCount(id);
    if (info[2]->IsNumber())
    {
      limit = static_cast<int>(Nan::To<uint32_t>(info[2]).ToChecked());
    }
    Local<Object> node = parser->GetNodeById_(id, current, limit, KALL);
    info.GetReturnValue().Set(node);
  }

  void Parser::GetNodeIdByAddress(const Nan::FunctionCallbackInfo<Value> &info)
  {
    if (!info[0]->IsString())
    {
      Nan::ThrowTypeError(Nan::New<String>("argument must be string!").ToLocalChecked());
      return;
    }
    Nan::Utf8String addr(Nan::To<v8::String>(info[0]).ToLocalChecked());
    char start = '@';
    if (strncmp(*addr, &start, 1) != 0)
    {
      Nan::ThrowTypeError(Nan::New<String>("argument 0 must be startwith \"@\"!").ToLocalChecked());
      return;
    }
    Parser *parser = ObjectWrap::Unwrap<Parser>(info.Holder());
    int id = parser->snapshot_parser->SearchOrdinalByAddress(atoi((*addr) + 1));
    if (id == -1)
    {
      std::string addrs = *addr;
      std::string error = "address \"" + addrs + "\" is wrong!";
      Nan::ThrowTypeError(Nan::New<String>(error).ToLocalChecked());
      return;
    }
    info.GetReturnValue().Set(Nan::New<Number>(id));
  }

  void Parser::GetFileName(const Nan::FunctionCallbackInfo<Value> &info)
  {
    Parser *parser = ObjectWrap::Unwrap<Parser>(info.Holder());
    info.GetReturnValue().Set(Nan::New(parser->filename_).ToLocalChecked());
  }

  void Parser::GetStatistics(const Nan::FunctionCallbackInfo<Value> &info)
  {
    Parser *parser = ObjectWrap::Unwrap<Parser>(info.Holder());
    Local<Object> statistics = Nan::New<Object>();
    Nan::Set(statistics, Nan::New<v8::String>("node_count").ToLocalChecked(), Nan::New<Number>(parser->snapshot_parser->node_count));
    Nan::Set(statistics, Nan::New<v8::String>("edge_count").ToLocalChecked(), Nan::New<Number>(parser->snapshot_parser->edge_count));
    Nan::Set(statistics, Nan::New<v8::String>("gcroots").ToLocalChecked(), Nan::New<Number>(parser->snapshot_parser->gcroots));
    Nan::Set(statistics, Nan::New<v8::String>("total_size").ToLocalChecked(), Nan::New<Number>(parser->snapshot_parser->GetRetainedSize(parser->snapshot_parser->root_index)));
    info.GetReturnValue().Set(statistics);
  }

  void Parser::GetDominatorByIDom(const Nan::FunctionCallbackInfo<v8::Value> &info)
  {
    if (!info[0]->IsNumber())
    {
      Nan::ThrowTypeError(Nan::New<String>("argument 0 must be number!").ToLocalChecked());
      return;
    }
    Parser *parser = ObjectWrap::Unwrap<Parser>(info.Holder());
    snapshot_dominates_t *ptr =
        parser->snapshot_parser->GetSortedDominates(static_cast<int>(Nan::To<uint32_t>(info[0]).ToChecked()));
    int current = 0;
    if (info[1]->IsNumber())
      current = static_cast<int>(Nan::To<uint32_t>(info[1]).ToChecked());
    int limit = 0;
    if (info[2]->IsNumber())
      limit = static_cast<int>(Nan::To<uint32_t>(info[2]).ToChecked());
    else
      limit = ptr->length;
    if (current >= ptr->length)
      current = ptr->length;
    int end = current + limit;
    if (end >= ptr->length)
      end = ptr->length;
    Local<Array> dominates = Nan::New<Array>(end - current);
    for (int i = current; i < end; i++)
    {
      snapshot_dominate_t *sdom = *(ptr->dominates + i);
      int id = sdom->dominate;
      Local<Object> node = parser->SetNormalInfo_(id);
      if (sdom->edge != -1)
      {
        std::string edge_name_or_index = parser->snapshot_parser->edge_util->GetNameOrIndex(sdom->edge, true);
        Nan::Set(node, Nan::New<v8::String>("edge_name_or_index").ToLocalChecked(), Nan::New<String>(edge_name_or_index).ToLocalChecked());
        std::string edge_type = parser->snapshot_parser->edge_util->GetType(sdom->edge, true);
        Nan::Set(node, Nan::New<v8::String>("edge_type").ToLocalChecked(), Nan::New<String>(edge_type).ToLocalChecked());
      }
      Nan::Set(node, Nan::New<v8::String>("index").ToLocalChecked(), Nan::New<Number>(i));
      Nan::Set(dominates, (i - current), node);
    }
    Local<Object> result = Nan::New<Object>();
    Nan::Set(result, Nan::New<v8::String>("dominates").ToLocalChecked(), dominates);
    if (end < ptr->length)
    {
      Nan::Set(result, Nan::New<v8::String>("dominates_end").ToLocalChecked(), Nan::New<Boolean>(false));
      Nan::Set(result, Nan::New<v8::String>("dominates_current").ToLocalChecked(), Nan::New<Number>(end));
      Nan::Set(result, Nan::New<v8::String>("dominates_left").ToLocalChecked(), Nan::New<Number>(ptr->length - end));
    }
    else
      Nan::Set(result, Nan::New<v8::String>("dominates_end").ToLocalChecked(), Nan::New<Boolean>(true));
    info.GetReturnValue().Set(result);
  }

  void Parser::GetChildRepeat(const Nan::FunctionCallbackInfo<v8::Value> &info)
  {
    if (!info[0]->IsNumber() || !info[1]->IsNumber())
    {
      Nan::ThrowTypeError(Nan::New<String>("argument 0 & 1 must be number!").ToLocalChecked());
      return;
    }
    Parser *parser = ObjectWrap::Unwrap<Parser>(info.Holder());
    int parent_id = static_cast<int>(Nan::To<uint32_t>(info[0]).ToChecked());
    int child_id = static_cast<int>(Nan::To<uint32_t>(info[1]).ToChecked());
    if (!parser->snapshot_parser->node_util->CheckOrdinalId(child_id) ||
        !parser->snapshot_parser->node_util->CheckOrdinalId(parent_id))
    {
      Nan::ThrowTypeError(Nan::New<String>("argument 0 is wrong!").ToLocalChecked());
      return;
    }
    int child_name = parser->snapshot_parser->node_util->GetNameForInt(child_id);
    int child_self_size = parser->snapshot_parser->node_util->GetSelfSize(child_id);
    int child_distance = parser->snapshot_parser->GetDistance(child_id);
    // search
    int count = 0;
    int total_retained_size = 0;
    bool done = false;
    if (info[2]->IsString())
    {
      Nan::Utf8String type(Nan::To<v8::String>(info[2]).ToLocalChecked());
      std::string dominates_type = "dominates";
      if (strcmp(dominates_type.c_str(), *type) == 0)
      {
        done = true;
        snapshot_dominates_t *ptr = parser->snapshot_parser->GetSortedDominates(parent_id);
        int childs_length = ptr->length;
        snapshot_dominate_t **childs = ptr->dominates;
        for (int i = 0; i < childs_length; i++)
        {
          snapshot_dominate_t *child = *(childs + i);
          int target_node = child->dominate;
          if (!parser->snapshot_parser->node_util->CheckOrdinalId(target_node))
            continue;
          int name = parser->snapshot_parser->node_util->GetNameForInt(target_node);
          int self_size = parser->snapshot_parser->node_util->GetSelfSize(target_node);
          int distance = parser->snapshot_parser->GetDistance(target_node);
          if (name == child_name && self_size == child_self_size && child_distance == distance)
          {
            count++;
            total_retained_size += parser->snapshot_parser->GetRetainedSize(target_node);
          }
        }
      }
    }
    if (!done)
    {
      int childs_length = parser->snapshot_parser->node_util->GetEdgeCount(parent_id);
      int *childs = parser->snapshot_parser->GetSortedEdges(parent_id);
      for (int i = 0; i < childs_length; i++)
      {
        int target_node = parser->snapshot_parser->edge_util->GetTargetNode(*(childs + i), true);
        if (!parser->snapshot_parser->node_util->CheckOrdinalId(target_node))
          continue;
        int name = parser->snapshot_parser->node_util->GetNameForInt(target_node);
        int self_size = parser->snapshot_parser->node_util->GetSelfSize(target_node);
        int distance = parser->snapshot_parser->GetDistance(target_node);
        if (name == child_name && self_size == child_self_size && child_distance == distance)
        {
          count++;
          total_retained_size += parser->snapshot_parser->GetRetainedSize(target_node);
        }
      }
    }
    Local<Object> result = Nan::New<Object>();
    Nan::Set(result, Nan::New<v8::String>("count").ToLocalChecked(), Nan::New<Number>(count));
    Nan::Set(result, Nan::New<v8::String>("total_retained_size").ToLocalChecked(), Nan::New<Number>(total_retained_size));
    info.GetReturnValue().Set(result);
  }

  void Parser::GetConsStringName(const Nan::FunctionCallbackInfo<v8::Value> &info)
  {
    if (!info[0]->IsNumber())
    {
      Nan::ThrowTypeError(Nan::New<String>("argument 0 must be number!").ToLocalChecked());
      return;
    }
    Parser *parser = ObjectWrap::Unwrap<Parser>(info.Holder());
    int id = static_cast<int>(Nan::To<uint32_t>(info[0]).ToChecked());
    if (!parser->snapshot_parser->node_util->CheckOrdinalId(id))
    {
      Nan::ThrowTypeError(Nan::New<String>("argument 0 is wrong!").ToLocalChecked());
      return;
    }
    Local<String> cons_name = Nan::New<String>(parser->snapshot_parser->node_util
                                                   ->GetConsStringName(id))
                                  .ToLocalChecked();
    info.GetReturnValue().Set(cons_name);
  }
}