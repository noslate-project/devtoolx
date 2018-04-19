#include "../library/json.hpp"

#ifndef _SNAPSHOT_EDGE_H_
#define _SNAPSHOT_EDGE_H_

namespace snapshot_parser {
class SnapshotParser;
}

namespace snapshot_edge {
using nlohmann::json;

enum EdgeTypes {
  KCONTEXTVARIABLE,
  KELEMENT,
  KPROPERTY,
  KERNAL,
  KHIDDEN,
  KSHORTCUT,
  KWEAK
};

class Edge {
public:
  explicit Edge(snapshot_parser::SnapshotParser* parser);
  ~Edge();
  std::string GetType(long id, bool source);
  int GetTypeForInt(long id, bool source);
  std::string GetNameOrIndex(long id, bool source);
  long GetTargetNode(long id, bool source);
private:
  snapshot_parser::SnapshotParser* parser_;
};
}

#endif