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
  node_count = static_cast<int>(nodes.size() / node_field_length);
  edge_count = static_cast<int>(edges.size() / edge_field_length);
  node_types = snapshot["meta"]["node_types"][0];
  edge_types = snapshot["meta"]["edge_types"][0];
  node_type_offset = IndexOf_(node_fields, "type");
  node_name_offset = IndexOf_(node_fields, "name");
  node_address_offset = IndexOf_(node_fields, "id");
  node_self_size_offset = IndexOf_(node_fields, "self_size");
  node_edge_count_offset = IndexOf_(node_fields, "edge_count");
  node_trace_nodeid_offset = IndexOf_(node_fields, "trace_node_id");
  edge_type_offset = IndexOf_(edge_fields, "type");
  edge_name_or_index_offset = IndexOf_(edge_fields, "name_or_index");
  edge_to_node_offset = IndexOf_(edge_fields, "to_node");
  edge_from_node = new int[edge_count]();
  first_edge_indexes = GetFirstEdgeIndexes_();
  node_util = new snapshot_node::Node(this);
  edge_util = new snapshot_edge::Edge(this);
}

int SnapshotParser::IndexOf_(json array, std::string target) {
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

void SnapshotParser::FillArray_(int* array, int length, int fill) {
  for(int i = 0; i < length; i++) {
    *(array + i) = fill;
  }
}

int* SnapshotParser::GetFirstEdgeIndexes_() {
  int* first_edge_indexes = new int[node_count]();
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
    long address = node_util->GetAddress(ordinal, false);
    address_map_.insert(AddressMap::value_type(address, ordinal));
  }
}

void SnapshotParser::ClearAddressMap() {
  address_map_.clear();
}

int SnapshotParser::SearchOrdinalByAddress(long address) {
  int count = address_map_.count(address);
  if(count == 0) {
    return -1;
  }
  int ordinal = address_map_.at(address);
  return ordinal;
}

void SnapshotParser::BuildTotalRetainer() {
  retaining_nodes_ = new int[edge_count]();
  retaining_edges_ = new int[edge_count]();
  first_retainer_index_ = new int[node_count + 1]();
  // every node's retainer count
  for(int to_node_field_index = edge_to_node_offset, l = static_cast<int>(edges.size()); to_node_field_index < l; to_node_field_index += edge_field_length) {
    int to_node_index = static_cast<int>(edges[to_node_field_index]);
    if(to_node_index % node_field_length != 0) {
      Nan::ThrowTypeError(Nan::New<v8::String>("node index id is wrong!").ToLocalChecked());
      return;
    }
    int ordinal_id = to_node_index / node_field_length;
    first_retainer_index_[ordinal_id] += 1;
  }
  // set first retainer index
  for(int i = 0, first_unused_retainer_slot = 0; i < node_count; i++) {
    int retainers_count = first_retainer_index_[i];
    first_retainer_index_[i] = first_unused_retainer_slot;
    retaining_nodes_[first_unused_retainer_slot] = retainers_count;
    first_unused_retainer_slot += retainers_count;
  }
  // for (index ~ index + 1)
  first_retainer_index_[node_count] = edge_count;
  // set retaining slot
  int next_node_first_edge_index = first_edge_indexes[0];
  for(int src_node_ordinal = 0; src_node_ordinal < node_count; src_node_ordinal++) {
    int first_edge_index = next_node_first_edge_index;
    next_node_first_edge_index = first_edge_indexes[src_node_ordinal + 1];
    for(int edge_index = first_edge_index; edge_index < next_node_first_edge_index; edge_index += edge_field_length) {
      int to_node_index = static_cast<int>(edges[edge_index + edge_to_node_offset]);
      if(to_node_index % node_field_length != 0) {
        Nan::ThrowTypeError(Nan::New<v8::String>("to_node id is wrong!").ToLocalChecked());
        return;
      }
      int first_retainer_slot_index = first_retainer_index_[to_node_index / node_field_length];
      int next_unused_retainer_slot_index = first_retainer_slot_index + (--retaining_nodes_[first_retainer_slot_index]);
      // save retainer & edge
      retaining_nodes_[next_unused_retainer_slot_index] = src_node_ordinal;
      retaining_edges_[next_unused_retainer_slot_index] = edge_index;
    }
  }
}

int SnapshotParser::GetRetainersCount(int id) {
  int first_retainer_index = first_retainer_index_[id];
  int next_retainer_index = first_retainer_index_[id + 1];
  // count may not be larger than 2^31
  return static_cast<int>(next_retainer_index - first_retainer_index);
}

snapshot_retainer_t** SnapshotParser::GetRetainers(int id) {
  if(ordered_retainers_map_.count(id) != 0)
    return ordered_retainers_map_.at(id);
  int first_retainer_index = first_retainer_index_[id];
  int next_retainer_index = first_retainer_index_[id + 1];
  int length = static_cast<int>(next_retainer_index - first_retainer_index);
  snapshot_retainer_t** retainers = new snapshot_retainer_t*[length];
  for(int i = first_retainer_index; i < next_retainer_index; i++) {
    snapshot_retainer_t* retainer = new snapshot_retainer_t;
    retainer->ordinal = retaining_nodes_[i];
    retainer->edge = retaining_edges_[i];
    retainers[i - first_retainer_index] = retainer;
  }
  std::sort(retainers, retainers + length, [this](snapshot_retainer_t* lhs, snapshot_retainer_t* rhs) {
    int lhs_distance = this->node_distances_[lhs->ordinal];
    int rhs_distance = this->node_distances_[rhs->ordinal];
    return lhs_distance < rhs_distance;
  });
  ordered_retainers_map_.insert(OrderedRetainersMap::value_type(id, retainers));
  return retainers;
}

void SnapshotParser::EnqueueNode_(snapshot_distance_t* t) {
  if(t->node_distances_[t->ordinal] != NO_DISTANCE)
    return;
  t->node_distances_[t->ordinal] = t->distance;
  t->node_to_visit[*(t->node_to_visit_length)] = t->ordinal;
  *(t->node_to_visit_length) += 1;
}

bool SnapshotParser::Filter_(int ordinal, int edge) {
  int node_type = node_util->GetTypeForInt(ordinal, false);
  if(node_type == snapshot_node::NodeTypes::KHIDDEN) {
    std::string edge_name = edge_util->GetNameOrIndex(edge, true);
    std::string node_name = node_util->GetName(ordinal, false);
    std::string slow_function_map_name = "sloppy_function_map";
    std::string native_context = "system / NativeContext";
    return (strcmp(edge_name.c_str(), slow_function_map_name.c_str()) != 0) || (strcmp(node_name.c_str(), native_context.c_str()) != 0);
  }
  if(node_type == snapshot_node::NodeTypes::KARRAY) {
    std::string node_name = node_util->GetName(ordinal, false);
    std::string map_descriptors_name = "(map descriptors)";
    if(strcmp(node_name.c_str(), map_descriptors_name.c_str()) != 0)
      return true;
    std::string edge_name = edge_util->GetNameOrIndex(edge, true);
    int index = atoi(edge_name.c_str());
    return index < 2 || (index % 3) != 1;
  }
  return true;
}

void SnapshotParser::ForEachRoot_(void (*action)(snapshot_distance_t* t), snapshot_distance_t* user_root, bool user_root_only) {
  std::unordered_map<int, bool> visit_nodes;
  int gc_roots = -1;
  int* edges = node_util->GetEdges(root_index, false);
  int length = node_util->GetEdgeCount(root_index, false);
  for(int i = 0; i < length; i++) {
    int target_node = edge_util->GetTargetNode(*(edges + i), true);
    std::string node_name = node_util->GetName(target_node, false);
    std::string gc_root_name = "(GC roots)";
    if(strcmp(node_name.c_str(), gc_root_name.c_str()) == 0) {
      gc_roots = target_node;
    }
  }
  if(gc_roots == -1)
    return;
  if(user_root_only) {
    // iterator the "true" root, set user root distance 1 -> global
    for(int i = 0; i < length; i++) {
      int target_node = edge_util->GetTargetNode(*(edges + i), true);
      std::string node_name = node_util->GetName(target_node, false);
      int type = node_util->GetTypeForInt(target_node, false);
      // type != synthetic, means user root
      if(type != snapshot_node::NodeTypes::KSYNTHETIC) {
        if(visit_nodes.count(target_node) == 0) {
          user_root->ordinal = target_node;
          action(user_root);
          visit_nodes.insert(std::unordered_map<int, bool>::value_type(target_node, true));
        }
      }
    }
    delete[] edges;
    // set user root gc roots -> synthetic roots -> true roots
    // int* sub_root_edges = node_util->GetEdges(gc_roots, false);
    // int sub_root_edge_length = node_util->GetEdgeCount(gc_roots, false);
    // for(int i = 0; i < sub_root_edge_length; i++) {
    //   int sub_root_ordinal = edge_util->GetTargetNode(*(sub_root_edges + i), true);
    //   int* sub2_root_edges = node_util->GetEdges(sub_root_ordinal, false);
    //   int sub2_root_edge_length = node_util->GetEdgeCount(sub_root_ordinal, false);
    //   for(int j = 0; j < sub2_root_edge_length; j++) {
    //     int sub2_root_ordinal = edge_util->GetTargetNode(*(sub2_root_edges + j), true);
    //     // mark sub sub gc roots
    //     if(visit_nodes.count(sub2_root_ordinal) == 0) {
    //       user_root->ordinal = sub2_root_ordinal;
    //       action(user_root);
    //       visit_nodes.insert(std::unordered_map<int, bool>::value_type(sub2_root_ordinal, true));
    //     }
    //   }
    // }
  } else {
    int* sub_root_edges = node_util->GetEdges(gc_roots, false);
    int sub_root_edge_length = node_util->GetEdgeCount(gc_roots, false);
    for(int i = 0; i < sub_root_edge_length; i++) {
      int sub_root_ordinal = edge_util->GetTargetNode(*(sub_root_edges + i), true);
      int* sub2_root_edges = node_util->GetEdges(sub_root_ordinal, false);
      int sub2_root_edge_length = node_util->GetEdgeCount(sub_root_ordinal, false);
      bool need_add_gc_root = true;
      std::string sub_root_name = node_util->GetName(sub_root_ordinal, false);
      if(sub_root_name.compare("(Internalized strings)") == 0
          || sub_root_name.compare("(External strings)") == 0
          || sub_root_name.compare("(Smi roots)") == 0) {
        need_add_gc_root = false;
      }
      for(int j = 0; j < sub2_root_edge_length; j++) {
        int sub2_root_ordinal = edge_util->GetTargetNode(*(sub2_root_edges + j), true);
        // mark sub sub gc roots
        if(visit_nodes.count(sub2_root_ordinal) == 0) {
          user_root->ordinal = sub2_root_ordinal;
          action(user_root);
          visit_nodes.insert(std::unordered_map<int, bool>::value_type(sub2_root_ordinal, true));
        }
        // add gc root
        if(need_add_gc_root) {
          int sub_to_sub2_edge_type = edge_util->GetTypeForInt(*(sub2_root_edges + j), true);
          if(sub_to_sub2_edge_type != snapshot_edge::EdgeTypes::KWEAK) {
            gcroots++;
            gcroots_map_.insert(GCRootsMap::value_type(sub2_root_ordinal, true));
          }
        }
      }
      delete[] sub2_root_edges;
      // mark sub gc roots
      if(visit_nodes.count(sub_root_ordinal) == 0) {
        user_root->ordinal = sub_root_ordinal;
        action(user_root);
        visit_nodes.insert(std::unordered_map<int, bool>::value_type(sub_root_ordinal, true));
      }
    }
    delete[] sub_root_edges;
    // mark sub roots
    for(int i = 0; i < length; i++) {
      int target_node = edge_util->GetTargetNode(*(edges + i), true);
      if(visit_nodes.count(target_node) == 0) {
        user_root->ordinal = target_node;
        action(user_root);
        visit_nodes.insert(std::unordered_map<int, bool>::value_type(target_node, true));
      }
    }
  }
}

void SnapshotParser::BFS_(int* node_to_visit, int node_to_visit_length) {
  int index = 0;
  int temp = 0;
  while(index < node_to_visit_length) {
    int ordinal = node_to_visit[index++];
    int distance = node_distances_[ordinal] + 1;
    int* edges = node_util->GetEdges(ordinal, false);
    int edge_length = node_util->GetEdgeCount(ordinal, false);
    for(int i = 0; i < edge_length; i++) {
      int edge_type = edge_util->GetTypeForInt(*(edges + i), true);
      // ignore weak edge
      if(edge_type == snapshot_edge::EdgeTypes::KWEAK)
        continue;
      int child_ordinal = edge_util->GetTargetNode(*(edges + i), true);
      if(node_distances_[child_ordinal] != NO_DISTANCE)
        continue;
      // need optimized filter
      // if(!Filter_(ordinal, *(edges + i)))
      // continue;
      node_distances_[child_ordinal] = distance;
      node_to_visit[node_to_visit_length++] = child_ordinal;
      temp++;
    }
    delete[] edges;
  }
  if (node_to_visit_length > node_count) {
    std::string error = "BFS failed. Nodes to visit (" + std::to_string(node_to_visit_length)
                        + ") is more than nodes count (" + std::to_string(node_count) + ")";
    Nan::ThrowTypeError(Nan::New<v8::String>(error).ToLocalChecked());
  }
}

void SnapshotParser::BuildDistances() {
  node_distances_ = new int[node_count];
  for(int i = 0; i < node_count; i++) {
    *(node_distances_ + i) = NO_DISTANCE;
  }
  int* node_to_visit = new int[node_count]();
  int node_to_visit_length = 0;
  // add user root
  snapshot_distance_t* user_root = new snapshot_distance_t;
  user_root->distance = 1;
  user_root->node_to_visit = node_to_visit;
  user_root->node_to_visit_length = &node_to_visit_length;
  user_root->node_distances_ = node_distances_;
  ForEachRoot_(EnqueueNode_, user_root, true);
  BFS_(node_to_visit, node_to_visit_length);
  // add rest
  node_to_visit_length = 0;
  user_root->distance = BASE_SYSTEMDISTANCE;
  ForEachRoot_(EnqueueNode_, user_root, false);
  BFS_(node_to_visit, node_to_visit_length);
  user_root = nullptr;
}

int SnapshotParser::GetRetainedSize(int id) {
  return retained_sizes_[id];
}

int SnapshotParser::GetDistance(int id) {
  return node_distances_[id];
}

int SnapshotParser::IsGCRoot(int id) {
  int count = gcroots_map_.count(id);
  if(count == 0)
    return 0;
  return 1;
}

void SnapshotParser::BuildDominatorTree() {
  CalculateFlags_();
  snapshot_post_order_t* ptr = BuildPostOrderIndex_();
  BuildDominatorTree_(ptr);
  CalculateRetainedSizes_(ptr);
  // free memory
  delete[] ptr->post_order_index_to_ordinal;
  delete[] ptr->ordinal_to_post_order_index;
  delete ptr;
}

bool SnapshotParser::IsEssentialEdge_(int ordinal, int type) {
  return type != snapshot_edge::EdgeTypes::KWEAK &&
         (type != snapshot_edge::EdgeTypes::KSHORTCUT || ordinal == root_index);
}

bool SnapshotParser::HasOnlyWeakRetainers_(int ordinal) {
  int begin_retainer_index = first_retainer_index_[ordinal];
  int end_retainer_index = first_retainer_index_[ordinal + 1];
  for(int retainer_index = begin_retainer_index; retainer_index < end_retainer_index; ++retainer_index) {
    int retainer_edge_index = retaining_edges_[retainer_index];
    int retainer_edge_type = edge_util->GetTypeForInt(retainer_edge_index, true);
    if (retainer_edge_type != snapshot_edge::EdgeTypes::KWEAK
        && retainer_edge_type != snapshot_edge::EdgeTypes::KSHORTCUT)
      return false;
  }
  return true;
}

void SnapshotParser::CalculateFlags_() {
  flags_ = new int[node_count]();
  int* node_to_visit = new int[node_count]();
  int node_to_visit_length = 0;
  for(int edge_index = first_edge_indexes[root_index],
      end_edge_index = first_edge_indexes[root_index + 1];
      edge_index < end_edge_index; edge_index += edge_field_length) {
    int target_node = edge_util->GetTargetNode(edge_index, true);
    int edge_type = edge_util->GetTypeForInt(edge_index, true);
    if(edge_type == snapshot_edge::EdgeTypes::KELEMENT) {
      int node_type = node_util->GetTypeForInt(target_node, false);
      std::string node_name = node_util->GetName(target_node, false);
      if(!(node_type == snapshot_node::NodeTypes::KSYNTHETIC
           && node_name.compare("(Document DOM trees)") ==  0))
        continue;
    } else if(edge_type != snapshot_edge::EdgeTypes::KSHORTCUT)
      continue;
    node_to_visit[node_to_visit_length++] = target_node;
    flags_[target_node] |= page_object_flag_;
  }
  // mark object from global/window/dom
  while(node_to_visit_length) {
    int ordinal = node_to_visit[--node_to_visit_length];
    int begin_edge_index = first_edge_indexes[ordinal];
    int end_edge_index = first_edge_indexes[ordinal + 1];
    for(int edge_index = begin_edge_index;
        edge_index < end_edge_index; edge_index += edge_field_length) {
      int child_ordinal = edge_util->GetTargetNode(edge_index, true);
      // has been marked
      if(flags_[child_ordinal] & page_object_flag_)
        continue;
      int child_type = edge_util->GetTypeForInt(edge_index, true);
      if(child_type == snapshot_edge::EdgeTypes::KWEAK)
        continue;
      node_to_visit[node_to_visit_length++] = child_ordinal;
      flags_[child_ordinal] |= page_object_flag_;
    }
  }
}

snapshot_post_order_t* SnapshotParser::BuildPostOrderIndex_() {
  int* stack_nodes = new int[node_count]();
  int* stack_current_edge = new int[node_count]();
  int* post_order_index_to_ordinal = new int[node_count]();
  int* ordinal_to_post_order_index = new int[node_count]();
  int* visited = new int[node_count]();
  int post_order_index = 0;
  // set stack
  int stack_top = 0;
  stack_nodes[0] = root_index;
  stack_current_edge[0] = first_edge_indexes[root_index];
  visited[root_index] = 1;
  int iteration = 0;
  while(true) {
    ++iteration;
    // dfs
    while(stack_top >= 0) {
      int ordinal = stack_nodes[stack_top];
      int edge_index = stack_current_edge[stack_top];
      int end_edge_index = first_edge_indexes[ordinal + 1];
      if(edge_index < end_edge_index ) {
        // stack edge current offset to next edge
        stack_current_edge[stack_top] += edge_field_length;
        int edge_type = edge_util->GetTypeForInt(edge_index, true);
        if(!IsEssentialEdge_(ordinal, edge_type))
          continue;
        int target_node = edge_util->GetTargetNode(edge_index, true);
        if(visited[target_node] == 1)
          continue;
        int node_flag = flags_[ordinal] & page_object_flag_;
        int child_node_flag = flags_[target_node] & page_object_flag_;
        if(ordinal != root_index && child_node_flag != 0 &&  node_flag == 0)
          continue;
        ++stack_top;
        stack_nodes[stack_top] = target_node;
        stack_current_edge[stack_top] = first_edge_indexes[target_node];
        visited[target_node] = 1;
      } else {
        ordinal_to_post_order_index[ordinal] = post_order_index;
        post_order_index_to_ordinal[post_order_index++] = ordinal;
        --stack_top;
      }
    }
    if (post_order_index == node_count || iteration > 1)
      break;
    // may be have some unreachable object fromm root_index, can give warnings
    --post_order_index;
    stack_top = 0;
    stack_nodes[0] = root_index;
    stack_current_edge[0] = first_edge_indexes[root_index + 1];
    for (int i = 0; i < node_count; ++i) {
      if (visited[i] == 1 || !HasOnlyWeakRetainers_(i))
        continue;
      stack_nodes[++stack_top] = i;
      stack_current_edge[stack_top] = first_edge_indexes[i];
      visited[i] = 1;
    }
  }
  if (post_order_index != node_count) {
    --post_order_index;
    for (int i = 0; i < node_count; ++i) {
      if (visited[i] == 1)
        continue;
      ordinal_to_post_order_index[i] = post_order_index;
      post_order_index_to_ordinal[post_order_index++] = i;
    }
    ordinal_to_post_order_index[root_index] = post_order_index;
    post_order_index_to_ordinal[post_order_index++] = root_index;
  }
  // return struct
  snapshot_post_order_t* ptr = new snapshot_post_order_t;
  ptr->ordinal_to_post_order_index = ordinal_to_post_order_index;
  ptr->post_order_index_to_ordinal = post_order_index_to_ordinal;
  return ptr;
}

void SnapshotParser::BuildDominatorTree_(snapshot_post_order_t* ptr) {
  int root_post_ordered_index = node_count - 1;
  int no_entry = node_count;
  int* dominators = new int[node_count];
  for (int i = 0; i < root_post_ordered_index; ++i)
    dominators[i] = no_entry;
  dominators[root_post_ordered_index] = root_post_ordered_index;
  int* affected = new int[node_count]();
  int ordinal;
  {
    ordinal = root_index;
    int end_edge_index = first_edge_indexes[ordinal + 1];
    for(int edge_index = first_edge_indexes[ordinal];
        edge_index < end_edge_index; edge_index += edge_field_length) {
      int edge_type = edge_util->GetTypeForInt(edge_index, true);
      if(!IsEssentialEdge_(root_index, edge_type))
        continue;
      int child_ordinal = edge_util->GetTargetNode(edge_index, true);
      affected[ptr->ordinal_to_post_order_index[child_ordinal]] = 1;
    }
  }
  bool changed = true;
  while(changed) {
    changed = false;
    for (int post_order_index = root_post_ordered_index - 1;
         post_order_index >= 0; --post_order_index) {
      if (affected[post_order_index] == 0)
        continue;
      affected[post_order_index] = 0;
      if (dominators[post_order_index] == root_post_ordered_index)
        continue;
      ordinal = ptr->post_order_index_to_ordinal[post_order_index];
      int node_flag = flags_[ordinal] & page_object_flag_;
      int new_dominator_index = no_entry;
      int begin_retainer_index = first_retainer_index_[ordinal];
      int end_retainer_index = first_retainer_index_[ordinal + 1];
      bool orphan_node = true;
      for (int retainer_index = begin_retainer_index;
           retainer_index < end_retainer_index; ++retainer_index) {
        int retainer_edge_index = retaining_edges_[retainer_index];
        int retainer_edge_type = edge_util->GetTypeForInt(retainer_edge_index, true);
        int retainer_node_ordinal = retaining_nodes_[retainer_index];
        if (!IsEssentialEdge_(retainer_node_ordinal, retainer_edge_type))
          continue;
        orphan_node = false;
        int retainer_node_flag = flags_[retainer_node_ordinal] & page_object_flag_;
        if (retainer_node_ordinal != root_index && node_flag != 0 && retainer_node_flag == 0)
          continue;
        int retaner_post_order_index = ptr->ordinal_to_post_order_index[retainer_node_ordinal];
        if (dominators[retaner_post_order_index] != no_entry) {
          if (new_dominator_index == no_entry) {
            new_dominator_index = retaner_post_order_index;
          } else {
            while (retaner_post_order_index != new_dominator_index) {
              while (retaner_post_order_index < new_dominator_index)
                retaner_post_order_index = dominators[retaner_post_order_index];
              while (new_dominator_index < retaner_post_order_index)
                new_dominator_index = dominators[new_dominator_index];
            }
          }
          if (new_dominator_index == root_post_ordered_index)
            break;
        }
      }
      if (orphan_node)
        new_dominator_index = root_post_ordered_index;
      if (new_dominator_index != no_entry && dominators[post_order_index] != new_dominator_index) {
        dominators[post_order_index] = new_dominator_index;
        changed = true;
        ordinal = ptr->post_order_index_to_ordinal[post_order_index];
        int begin_edge_index = first_edge_indexes[ordinal];
        int end_edge_index = first_edge_indexes[ordinal + 1];
        for (int edge_index = begin_edge_index;
             edge_index < end_edge_index; edge_index += edge_field_length) {
          int child_ordinal = edge_util->GetTargetNode(edge_index, true);
          affected[ptr->ordinal_to_post_order_index[child_ordinal]] = 1;
        }
      }
    }
  }
  dominators_tree_ = new int[node_count]();
  for (int post_order_index = 0, l = node_count; post_order_index < l; ++post_order_index) {
    ordinal = ptr->post_order_index_to_ordinal[post_order_index];
    dominators_tree_[ordinal] = ptr->post_order_index_to_ordinal[dominators[post_order_index]];
  }
}

void SnapshotParser::CalculateRetainedSizes_(snapshot_post_order_t* ptr) {
  retained_sizes_ = new int[node_count]();
  for (int ordinal = 0; ordinal < node_count; ++ordinal)
    retained_sizes_[ordinal] = node_util->GetSelfSize(ordinal, false);
  for (int post_order_index = 0; post_order_index < node_count - 1; ++post_order_index) {
    int ordinal = ptr->post_order_index_to_ordinal[post_order_index];
    // dominator_ordinal immediately dominated ordinal
    int dominator_ordinal = dominators_tree_[ordinal];
    retained_sizes_[dominator_ordinal] += retained_sizes_[ordinal];
  }
}

int* SnapshotParser::GetSortedEdges(int id) {
  if(ordered_edges_map_.count(id) != 0)
    return ordered_edges_map_.at(id);
  int* edges = node_util->GetEdges(id, false);
  int length = node_util->GetEdgeCount(id, false);
  std::sort(edges, edges + length, [this](int lhs, int rhs) {
    int lhs_ordinal = this->edge_util->GetTargetNode(lhs, true);
    int lhs_retained_size = this->retained_sizes_[lhs_ordinal];
    int rhs_ordinal = this->edge_util->GetTargetNode(rhs, true);
    int rhs_retained_size = this->retained_sizes_[rhs_ordinal];
    return lhs_retained_size > rhs_retained_size;
  });
  ordered_edges_map_.insert(OrderedEdgesMap::value_type(id, edges));
  return edges;
}

snapshot_dominates_t* SnapshotParser::GetSortedDominates(int id) {
  if(ordered_dominates_map_.count(id) != 0) {
    return ordered_dominates_map_.at(id);
  }
  snapshot_dominates_t* doms = new snapshot_dominates_t;
  int length = 0;
  for(int ordinal = 0; ordinal < node_count; ordinal++) {
    if(ordinal != root_index &&  dominators_tree_[ordinal] == id)
      length++;
  }
  doms->length = length;
  int* dominates = new int[length];
  for(int ordinal = 0; ordinal < node_count; ordinal++) {
    if(ordinal != root_index &&  dominators_tree_[ordinal] == id)
      dominates[--length] = ordinal;
  }
  std::sort(dominates, dominates + doms->length, [this](int lhs, int rhs) {
    int lhs_retained_size = this->retained_sizes_[lhs];
    int rhs_retained_size = this->retained_sizes_[rhs];
    return lhs_retained_size > rhs_retained_size;
  });
  doms->dominates = dominates;
  ordered_dominates_map_.insert(OrderedDominatesMap::value_type(id, doms));
  return doms;
}
}