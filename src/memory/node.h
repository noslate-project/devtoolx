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
private:
  snapshot_parser::SnapshotParser* parser_;
};
}

#endif