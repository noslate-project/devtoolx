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
  std::string GetType(int id, bool source);
  int GetTypeForInt(int id, bool source);
  std::string GetNameOrIndex(int id, bool source);
  int GetTargetNode(int id, bool source);
private:
  snapshot_parser::SnapshotParser* parser_;
};
}

#endif