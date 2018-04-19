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
  long GetNodeId(long source );
  long GetAddress(long id, bool source);
  std::string GetType(long id, bool source);
  int GetTypeForInt(long id, bool source);
  std::string GetName(long id, bool source);
  long GetNameForLong(long id, bool source);
  long* GetEdges(long id, bool source);
  int GetEdgeCount(long id, bool source);
  long GetSelfSize(long id, bool source);
private:
  snapshot_parser::SnapshotParser* parser_;
};
}

#endif