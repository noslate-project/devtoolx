#include "snapshot_parser.h"

namespace snapshot_parser {
SnapshotParser::SnapshotParser(json profile) {
  nodes = profile["nodes"];
  edges = profile["edges"];
  strings = profile["strings"];
  snapshot = profile["snapshot"];
  if (snapshot["root_index"] != nullptr) {
    root_index = snapshot["root_index"];
  }
  json node_fields = snapshot["meta"]["node_fields"];
  json edge_fields = snapshot["meta"]["edge_fields"];
  node_field_length = node_fields.size();
  edge_field_length = edge_fields.size();
  node_count = nodes.size() / node_field_length;
  edge_count = edges.size() / edge_field_length;
  node_types = snapshot["meta"]["node_types"][0];
  edge_types = snapshot["meta"]["edge_types"][0];
  node_type_offset = IndexOf(node_fields, "type");
  node_name_offset = IndexOf(node_fields, "name");
  node_address_offset = IndexOf(node_fields, "id");
  node_self_size_offset = IndexOf(node_fields, "self_size");
  node_edge_count_offset = IndexOf(node_fields, "edge_count");
  node_trace_nodeid_offset = IndexOf(node_fields, "trace_node_id");
  edge_type_offset = IndexOf(edge_fields, "type");
  edge_name_or_index_offset = IndexOf(edge_fields, "name_or_index");
  edge_to_node_offset = IndexOf(edge_fields, "to_node");
  edge_from_node = new int[edge_count];
  first_edge_indexes = GetFirstEdgeIndexes();
  node_util = new snapshot_node::Node(this);
  edge_util = new snapshot_edge::Edge(this);
}

int SnapshotParser::IndexOf(json array, std::string target) {
  const char* t = target.c_str();
  int size = array.size();
  for (int i = 0; i < size; i++) {
    std::string str1 = array[i];
    if(strcmp(str1.c_str(), t) == 0) {
      return i;
    }
  }
  return -1;
}

int* SnapshotParser::GetFirstEdgeIndexes() {
  int* first_edge_indexes = new int[this->node_count];
  for(int node_ordinal = 0, edge_index = 0; node_ordinal < this->node_count; node_ordinal++) {
    first_edge_indexes[node_ordinal] = edge_index;
    int offset = static_cast<int>(this->nodes[node_ordinal * this->node_field_length + this->node_edge_count_offset]) * this->edge_field_length;
    for(int i = edge_index; i < offset; i += this->edge_field_length) {
      this->edge_from_node[i / this->edge_field_length] = node_ordinal;
    }
    edge_index += offset;
  }
  return first_edge_indexes;
}
}