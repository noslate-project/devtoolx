#include <nan.h>
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
  int ordinal;
  int* node_to_visit;
  int* node_to_visit_length;
  int* node_distances_;
} snapshot_distance_t;

typedef struct {
  int ordinal;
  int edge;
} snapshot_retainer_t;

typedef std::unordered_map<long, int> AddressMap;
typedef std::unordered_map<int, bool> GCRootsMap;
typedef std::unordered_map<int, snapshot_retainer_t**> OrderedRetainersMap;

const int NO_DISTANCE = -5;
const int BASE_SYSTEMDISTANCE = 100000000;

class SnapshotParser {
public:
  explicit SnapshotParser(json profile);
  ~SnapshotParser();
  void CreateAddressMap();
  void ClearAddressMap();
  int SearchOrdinalByAddress(long address);
  void BuildTotalRetainer();
  int GetRetainersCount(int id);
  snapshot_retainer_t** GetRetainers(int id);
  void BuildDistances();
  int GetDistance(int id);
  int IsGCRoot(int id);
  static int IndexOf(json array, std::string target);
  json nodes;
  json edges;
  json strings;
  json snapshot;
  int root_index = 0;
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
  int gcroots;

private:
  int* GetFirstEdgeIndexes_();
  static void EnqueueNode_(snapshot_distance_t* t);
  void ForEachRoot_(void (*action)(snapshot_distance_t* t), snapshot_distance_t* user_root, bool user_root_only);
  void BFS_(int* node_to_visit, int node_to_visit_length);
  bool Filter_(int ordinal, int edge);
  // address -> node ordinal id
  AddressMap address_map_;
  // ordinal id -> bool
  GCRootsMap gcroots_map_;
  // ordinal id -> int* (ordered retainers)
  OrderedRetainersMap ordered_retainers_map_;
  // total retainers
  int* retaining_nodes_;
  int* retaining_edges_;
  int* first_retainer_index_;
  int* node_distances_;
};
}

#endif