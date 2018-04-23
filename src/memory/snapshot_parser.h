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

typedef struct  {
  int distance;
  long ordinal;
  long* node_to_visit;
  int* node_to_visit_length;
  int* node_distances_;
} snapshot_distance_t;

typedef std::unordered_map<long, long> AddressMap;
typedef std::unordered_map<long, bool> GCRootsMap;

const int NO_DISTANCE = -5;
const int BASE_SYSTEMDISTANCE = 100000000;

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
  void BuildDistances();
  int GetDistance(long id);
  int IsGCRoot(long id);
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
  int gcroots;

private:
  long* GetFirstEdgeIndexes();
  static void EnqueueNode_(snapshot_distance_t* t);
  void ForEachRoot_(void (*action)(snapshot_distance_t* t), snapshot_distance_t* user_root, bool user_root_only);
  void BFS_(long* node_to_visit, int node_to_visit_length);
  bool Filter(long ordinal, long edge);
  // address -> node ordinal id
  AddressMap address_map_;
  // ordinal id -> bool
  GCRootsMap gcroots_map_;
  // total retainers
  long* retaining_nodes_;
  long* retaining_edges_;
  long* first_retainer_index_;
  int* node_distances_;
};
}

#endif