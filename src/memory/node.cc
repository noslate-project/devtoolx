#include <nan.h>
#include "node.h"
#include "snapshot_parser.h"

namespace snapshot_node {
Node::Node(snapshot_parser::SnapshotParser* parser): parser_(parser) {}

int Node::GetNodeId(int source) {
  int node_field_length = parser_->node_field_length;
  if(source %  node_field_length != 0) {
    Nan::ThrowTypeError(Nan::New<v8::String>("node source id is wrong!").ToLocalChecked());
    return -1;
  }
  return static_cast<int>(source / node_field_length);
}

long Node::GetAddress(int id, bool source) {
  int node_field_length = parser_->node_field_length;
  if(source && id % node_field_length != 0) {
    Nan::ThrowTypeError(Nan::New<v8::String>("node source id is wrong!").ToLocalChecked());
    return -1;
  }
  int node_source_index = source ? id : id * node_field_length;
  if(node_source_index / node_field_length >= parser_->node_count) {
    Nan::ThrowTypeError(Nan::New<v8::String>("node id larger than nodes.length!").ToLocalChecked());
    return -1;
  }
  return static_cast<long>(parser_->nodes[node_source_index + parser_->node_address_offset]);
}

std::string Node::GetType(int id, bool source) {
  int node_field_length = parser_->node_field_length;
  if(source && id % node_field_length != 0) {
    Nan::ThrowTypeError(Nan::New<v8::String>("node source id is wrong!").ToLocalChecked());
    return "error";
  }
  int node_source_index = source ? id : id * node_field_length;
  if(node_source_index / node_field_length >= parser_->node_count) {
    Nan::ThrowTypeError(Nan::New<v8::String>("node id larger than nodes.length!").ToLocalChecked());
    return "error";
  }
  int type = static_cast<int>(parser_->nodes[node_source_index + parser_->node_type_offset]);
  json types = parser_->node_types;
  // with type "undefined", total 13
  if (type > static_cast<int>(types.size() - 1)) {
    return "undefined";
  }
  return types[type];
}

int Node::GetTypeForInt(int id, bool source) {
  int node_field_length = parser_->node_field_length;
  if(source && id % node_field_length != 0) {
    Nan::ThrowTypeError(Nan::New<v8::String>("node source id is wrong!").ToLocalChecked());
    return -1;
  }
  int node_source_index = source ? id : id * node_field_length;
  if(node_source_index / node_field_length >= parser_->node_count) {
    Nan::ThrowTypeError(Nan::New<v8::String>("node id larger than nodes.length!").ToLocalChecked());
    return -1;
  }
  return static_cast<int>(parser_->nodes[node_source_index + parser_->node_type_offset]);
}

std::string Node::GetName(int id, bool source) {
  if(id == parser_->root_index) {
    return "SYNTTETICROOT";
  }
  int node_field_length = parser_->node_field_length;
  if(source && id % node_field_length != 0) {
    Nan::ThrowTypeError(Nan::New<v8::String>("node source id is wrong!").ToLocalChecked());
    return "error";
  }
  int node_source_index = source ? id : id * node_field_length;
  if(node_source_index / node_field_length >= parser_->node_count) {
    Nan::ThrowTypeError(Nan::New<v8::String>("node id larger than nodes.length!").ToLocalChecked());
    return "error";
  }
  json strings = parser_->strings;
  int name = static_cast<int>(parser_->nodes[node_source_index + parser_->node_name_offset]);
  return strings[name];
}

int Node::GetNameForInt(int id, bool source) {
  int node_field_length = parser_->node_field_length;
  if(source && id % node_field_length != 0) {
    Nan::ThrowTypeError(Nan::New<v8::String>("node source id is wrong!").ToLocalChecked());
    return -1;
  }
  int node_source_index = source ? id : id * node_field_length;
  if(node_source_index / node_field_length >= parser_->node_count) {
    Nan::ThrowTypeError(Nan::New<v8::String>("node id larger than nodes.length!").ToLocalChecked());
    return -1;
  }
  return static_cast<int>(parser_->nodes[node_source_index + parser_->node_name_offset]);
}

int* Node::GetEdges(int id, bool source) {
  int node_field_length = parser_->node_field_length;
  if(source && id % node_field_length != 0) {
    Nan::ThrowTypeError(Nan::New<v8::String>("node source id is wrong!").ToLocalChecked());
    return new int[0];
  }
  int node_ordinal_index = source ?  id / node_field_length : id;
  if(node_ordinal_index >= parser_->node_count) {
    Nan::ThrowTypeError(Nan::New<v8::String>("node id larger than nodes.length!").ToLocalChecked());
    return new int[0];
  }
  int first_edge_index = parser_->first_edge_indexes[node_ordinal_index];
  int next_first_edge_index = 0;
  if(node_ordinal_index + 1 >= parser_->node_count) {
    next_first_edge_index = static_cast<int>(parser_->edges.size());
  } else {
    next_first_edge_index = parser_->first_edge_indexes[node_ordinal_index + 1];
  }
  int* edges = new int[(next_first_edge_index - first_edge_index) / parser_->edge_field_length];
  for (int i = first_edge_index; i < next_first_edge_index; i += parser_->edge_field_length) {
    edges[(i - first_edge_index) / parser_->edge_field_length] = i;
  }
  return edges;
}

int Node::GetEdgeCount(int id, bool source) {
  int node_field_length = parser_->node_field_length;
  if(source && id % node_field_length != 0) {
    Nan::ThrowTypeError(Nan::New<v8::String>("node source id is wrong!").ToLocalChecked());
    return -1;
  }
  int node_source_index = source ? id : id * node_field_length;
  if(node_source_index / node_field_length >= parser_->node_count) {
    Nan::ThrowTypeError(Nan::New<v8::String>("node id larger than nodes.length!").ToLocalChecked());
    return -1;
  }
  // edge count may not be larger than 2^31
  return static_cast<int>(parser_->nodes[node_source_index + parser_->node_edge_count_offset]);
}

int Node::GetSelfSize(int id, bool source) {
  int node_field_length = parser_->node_field_length;
  if(source && id % node_field_length != 0) {
    Nan::ThrowTypeError(Nan::New<v8::String>("node source id is wrong!").ToLocalChecked());
    return -1;
  }
  int node_source_index = source ? id : id * node_field_length;
  if(node_source_index / node_field_length >= parser_->node_count) {
    Nan::ThrowTypeError(Nan::New<v8::String>("node id larger than nodes.length!").ToLocalChecked());
    return -1;
  }
  return static_cast<int>(parser_->nodes[node_source_index + parser_->node_self_size_offset]);
}
}