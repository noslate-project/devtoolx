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
  int* first_edge_indexes = new int[node_count];
  for(int node_ordinal = 0, edge_index = 0; node_ordinal < node_count; node_ordinal++) {
    first_edge_indexes[node_ordinal] = edge_index;
    int offset = static_cast<int>(nodes[node_ordinal * node_field_length + node_edge_count_offset]) * edge_field_length;
    for(int i = edge_index; i < offset; i += edge_field_length) {
      edge_from_node[i / edge_field_length] = node_ordinal;
    }
    edge_index += offset;
  }
  return first_edge_indexes;
}

void SnapshotParser::CreateAddressMap() {
  for(int ordinal = 0; ordinal < node_count; ordinal++) {
    int address = node_util->GetAddress(ordinal, false);
    address_map_.insert(AddressMap::value_type(address, ordinal));
  }
}

void SnapshotParser::ClearAddressMap() {
  address_map_.clear();
}

int SnapshotParser::SearchOrdinalByAddress(int address) {
  int count = address_map_.count(address);
  if(count == 0) {
    return -1;
  }
  int ordinal = address_map_.at(address);
  return ordinal;
}

void SnapshotParser::BuildTotalRetainer() {

}
}