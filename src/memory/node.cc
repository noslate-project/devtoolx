#include <nan.h>
#include "node.h"
#include "snapshot_parser.h"

namespace snapshot_node {
Node::Node(snapshot_parser::SnapshotParser* parser): parser_(parser) {}

std::string Node::GetConsStringName(int id) {
  json& strings = parser_->strings;
  if(lazy_string_map_.count(id) != 0)
    return lazy_string_map_.at(id);
  // max length
  int* node_stack = new int[128]();
  int length = 1;
  node_stack[0] = id;
  std::string name = "";
  while(length > 0 && name.length() < 256) {
    int index = node_stack[--length];
    if(GetTypeForInt(index) != KCONCATENATED_STRING) {
      std::string cons = strings[static_cast<int>(parser_->nodes[index * parser_->node_field_length + parser_->node_name_offset])];
      name += cons;
      continue;
    }
    int* edges = GetEdges(index);
    int edge_count = GetEdgeCount(index);
    int first_node_index = -1;
    int second_node_index = -1;
    for(int i = 0; i < edge_count; i++) {
      int edge = edges[i];
      int edge_type = parser_->edge_util->GetTypeForInt(edge, true);
      if(edge_type == snapshot_edge::KERNAL) {
        if(first_int_ != -1 && second_int_ != -1) {
          int edge_name = parser_->edge_util->GetNameOrIndexForInt(edge, true);
          if(edge_name == first_int_)
            first_node_index = parser_->edge_util->GetTargetNode(edge, true);
          if(edge_name == second_int_)
            second_node_index = parser_->edge_util->GetTargetNode(edge, true);
        } else {
          std::string edge_name = parser_->edge_util->GetNameOrIndex(edge, true);
          if (edge_name == "first") {
            first_node_index = parser_->edge_util->GetTargetNode(edge, true);
            first_int_ =  parser_->edge_util->GetNameOrIndexForInt(edge, true);
          }
          if (edge_name == "second") {
            second_node_index = parser_->edge_util->GetTargetNode(edge, true);
            second_int_ = parser_->edge_util->GetNameOrIndexForInt(edge, true);
          }
        }
      }
    }
    delete[] edges;
    if(second_node_index != -1)
      node_stack[length++] = second_node_index;
    if(first_node_index != -1)
      node_stack[length++] = first_node_index;
  }
  delete[] node_stack;
  lazy_string_map_.insert(LazyStringMap::value_type(id, name));
  return name;
}

bool Node::CheckOrdinalId(int oridnal) {
  return oridnal < parser_->node_count;
}

int Node::GetNodeId(int source) {
  int node_field_length = parser_->node_field_length;
  if(source %  node_field_length != 0) {
    Nan::ThrowTypeError(Nan::New<v8::String>("node source id is wrong!").ToLocalChecked());
    return -1;
  }
  return static_cast<int>(source / node_field_length);
}

long Node::GetAddress(int id) {
  return static_cast<long>(parser_->nodes[id * parser_->node_field_length + parser_->node_address_offset]);
}

std::string Node::GetType(int id) {
  int type = static_cast<int>(parser_->nodes[id * parser_->node_field_length + parser_->node_type_offset]);
  json types = parser_->node_types;
  // with type "undefined", total 13
  if (type > static_cast<int>(types.size() - 1)) {
    return "undefined";
  }
  return types[type];
}

int Node::GetTypeForInt(int id) {
  return static_cast<int>(parser_->nodes[id * parser_->node_field_length + parser_->node_type_offset]);
}

std::string Node::GetName(int id) {
  return parser_->strings[static_cast<int>(parser_->nodes[id * parser_->node_field_length + parser_->node_name_offset])];
}

int Node::GetNameForInt(int id) {
  return static_cast<int>(parser_->nodes[id * parser_->node_field_length + parser_->node_name_offset]);
}

int* Node::GetEdges(int id) {
  int first_edge_index = parser_->first_edge_indexes[id];
  int next_first_edge_index = 0;
  if(id + 1 >= parser_->node_count) {
    next_first_edge_index = static_cast<int>(parser_->edges.size());
  } else {
    next_first_edge_index = parser_->first_edge_indexes[id + 1];
  }
  int* edges = new int[(next_first_edge_index - first_edge_index) / parser_->edge_field_length];
  for (int i = first_edge_index; i < next_first_edge_index; i += parser_->edge_field_length) {
    edges[(i - first_edge_index) / parser_->edge_field_length] = i;
  }
  return edges;
}

int Node::GetEdgeCount(int id) {
  // edge count may not be larger than 2^31
  return static_cast<int>(parser_->nodes[id * parser_->node_field_length + parser_->node_edge_count_offset]);
}

int Node::GetSelfSize(int id) {
  return static_cast<int>(parser_->nodes[id * parser_->node_field_length + parser_->node_self_size_offset]);
}
}
