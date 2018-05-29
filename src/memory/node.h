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
  int GetNodeId(int source );
  long GetAddress(int id, bool source);
  std::string GetType(int id, bool source);
  int GetTypeForInt(int id, bool source);
  std::string GetName(int id, bool source);
  int GetNameForInt(int id, bool source);
  int* GetEdges(int id, bool source);
  int GetEdgeCount(int id, bool source);
  int GetSelfSize(int id, bool source);
  std::string GetConsStringName(int id);
private:
  snapshot_parser::SnapshotParser* parser_;
  LazyStringMap lazy_string_map_;
  int first_int_ = -1;
  int second_int_ = -1;
};
}

#endif