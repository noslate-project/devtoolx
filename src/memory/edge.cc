#include <nan.h>
#include "node.h"
#include "snapshot_parser.h"

namespace snapshot_edge {
Edge::Edge(snapshot_parser::SnapshotParser* parser): parser_(parser) {}

std::string Edge::GetType(long id, bool source) {
  int edge_field_length = parser_->edge_field_length;
  if(source && id % edge_field_length != 0) {
    Nan::ThrowTypeError(Nan::New<v8::String>("edge source id is wrong!").ToLocalChecked());
    return "error";
  }
  long edge_source_index = source ? id : id * edge_field_length;
  if(edge_source_index / edge_field_length >= parser_->edge_count) {
    Nan::ThrowTypeError(Nan::New<v8::String>("edge id larger than edges.length!").ToLocalChecked());
    return "error";
  }
  int type = static_cast<int>(parser_->edges[edge_source_index + parser_->edge_type_offset]);
  json types = parser_->edge_types;
  return types[type];
}

int Edge::GetTypeForInt(long id, bool source) {
  int edge_field_length = parser_->edge_field_length;
  if(source && id % edge_field_length != 0) {
    Nan::ThrowTypeError(Nan::New<v8::String>("edge source id is wrong!").ToLocalChecked());
    return -1;
  }
  long edge_source_index = source ? id : id * edge_field_length;
  if(edge_source_index / edge_field_length >= parser_->edge_count) {
    Nan::ThrowTypeError(Nan::New<v8::String>("edge id larger than edges.length!").ToLocalChecked());
    return -1;
  }
  return static_cast<int>(parser_->edges[edge_source_index + parser_->edge_type_offset]);
}

std::string Edge::GetNameOrIndex(long id, bool source) {
  int edge_field_length = parser_->edge_field_length;
  if(source && id % edge_field_length != 0) {
    Nan::ThrowTypeError(Nan::New<v8::String>("edge source id is wrong!").ToLocalChecked());
    return "error";
  }
  long edge_source_index = source ? id : id * edge_field_length;
  if(edge_source_index / edge_field_length >= parser_->edge_count) {
    Nan::ThrowTypeError(Nan::New<v8::String>("edge id larger than edges.length!").ToLocalChecked());
    return "error";
  }
  int type = static_cast<int>(parser_->edges[edge_source_index + parser_->edge_type_offset]);
  long name_or_index = static_cast<long>(parser_->edges[edge_source_index + parser_->edge_name_or_index_offset]);
  if(type == KELEMENT) {
    return "[" + std::to_string(name_or_index) + "]";
  } else if(type == KHIDDEN) {
    return std::to_string(name_or_index);
  } else {
    return parser_->strings[name_or_index];
  };
}

long Edge::GetTargetNode(long id, bool source) {
  int edge_field_length = parser_->edge_field_length;
  if(source && id % edge_field_length != 0) {
    Nan::ThrowTypeError(Nan::New<v8::String>("edge source id is wrong!").ToLocalChecked());
    return -1;
  }
  long edge_source_index = source ? id : id * edge_field_length;
  if(edge_source_index / edge_field_length >= parser_->edge_count) {
    Nan::ThrowTypeError(Nan::New<v8::String>("edge id larger than edges.length!").ToLocalChecked());
    return -1;
  }
  long target_node_source_id = static_cast<long>(parser_->edges[edge_source_index + parser_->edge_to_node_offset]);
  int node_field_length = parser_->node_field_length;
  if(target_node_source_id % node_field_length != 0) {
    Nan::ThrowTypeError(Nan::New<v8::String>("target node source id is wrong!").ToLocalChecked());
    return -1;
  }
  return static_cast<long>(target_node_source_id / node_field_length);
}
}