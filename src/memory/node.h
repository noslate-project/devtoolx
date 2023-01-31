#include <unordered_map>

#include "../library/json.hpp"

#ifndef _SNAPSHOT_NODE_H_
#define _SNAPSHOT_NODE_H_

namespace snapshot_parser {
class SnapshotParser;
}

namespace snapshot_node {
using nlohmann::json;

enum NodeTypes {
  KHIDDEN,
  KARRAY,
  KSTRING,
  KOBJECT,
  KCODE,
  KCLOSURE,
  KREGEXP,
  KNUMBER,
  KNATIVE,
  KSYNTHETIC,
  KCONCATENATED_STRING,
  KSLICED_STRING
};

typedef std::unordered_map<int, std::string> LazyStringMap;

class Node {
 public:
  explicit Node(snapshot_parser::SnapshotParser* parser);
  ~Node();
  bool CheckOrdinalId(int id);
  int GetNodeId(int source);
  long GetAddress(int id);
  std::string GetType(int id);
  int GetTypeForInt(int id);
  std::string GetName(int id);
  int GetNameForInt(int id);
  int* GetEdges(int id);
  int GetEdgeCount(int id);
  int GetSelfSize(int id);
  std::string GetConsStringName(int id);

 private:
  snapshot_parser::SnapshotParser* parser_;
  LazyStringMap lazy_string_map_;
  int first_int_ = -1;
  int second_int_ = -1;
};
}  // namespace snapshot_node

#endif