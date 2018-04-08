#include <iostream>
#include "../library/json.hpp"
#include "node.h"
#include "edge.h"

#ifndef __SNAPSHOT_PARSER_H_
#define __SNAPSHOT_PARSER_H_

namespace snapshot_parser {
using nlohmann::json;

class SnapshotParser {
public:
  explicit SnapshotParser(json profile);
  ~SnapshotParser();
  static int IndexOf(json array, std::string target);
  json nodes;
  json edges;
  json strings;
  json snapshot;
  int root_index;
  int node_field_length;
  int edge_field_length;
  int node_count;
  int edge_count;
  json node_types;
  json edge_types;
  int node_type_offset;
  int node_name_offset;
  int node_address_offset;
  int node_self_size_offset;
  int node_edge_count_offset;
  int node_trace_nodeid_offset;
  int edge_type_offset;
  int edge_name_or_index_offset;
  int edge_to_node_offset;
  int* edge_from_node;
  int* first_edge_indexes;
  snapshot_node::Node* node_util;
  snapshot_edge::Edge* edge_util;
private:
  int* GetFirstEdgeIndexes();
};
}

#endif