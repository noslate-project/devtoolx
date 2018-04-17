#include<nan.h>
#include <iostream>
#include <unordered_map>
#include "node.h"
#include "edge.h"
#include "../library/json.hpp"

#ifndef __SNAPSHOT_PARSER_H_
#define __SNAPSHOT_PARSER_H_

namespace snapshot_parser {
using nlohmann::json;

typedef std::unordered_map<long, long> AddressMap;

class SnapshotParser {
public:
  explicit SnapshotParser(json profile);
  ~SnapshotParser();
  void CreateAddressMap();
  void ClearAddressMap();
  long SearchOrdinalByAddress(long address);
  void BuildTotalRetainer();
  int GetRetainersCount(long id);
  long* GetRetainers(long id);
  static int IndexOf(json array, std::string target);
  json nodes;
  json edges;
  json strings;
  json snapshot;
  int root_index = 0;
  int node_field_length;
  int edge_field_length;
  long node_count;
  long edge_count;
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
  long* edge_from_node;
  long* first_edge_indexes;
  snapshot_node::Node* node_util;
  snapshot_edge::Edge* edge_util;

private:
  long* GetFirstEdgeIndexes();
  // address -> node ordinal id
  AddressMap address_map_;
  // total retainers
  long* retaining_nodes_;
  long* retaining_edges_;
  long* first_retainer_index_;
};
}

#endif