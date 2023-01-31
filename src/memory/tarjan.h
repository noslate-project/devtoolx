#include <vector>
#include <unordered_map>

#ifndef __TARJAN_H_
#define __TARJAN_H_

namespace tarjan {

const int ERROR_VALUE = -999;

typedef std::unordered_map<int, std::vector<int>> BoundListMap;

typedef struct TarjanBoundList {
  BoundListMap inbounds;
  BoundListMap outbounds;
  int root = 0;
  int count;
  ~TarjanBoundList() {
    this->inbounds.clear();
    this->outbounds.clear();
    this->root = 0;
    this->count = 0;
  }
} tarjan_bound_list_t;

class TarJan {
public:
  explicit TarJan(tarjan_bound_list_t *data);
  ~TarJan();
  int length;
  int root;
  // middle variables
  int *dfs;
  int *vertex;
  int *semi;
  int *parent;
  BoundListMap bucket;
  int *dom;
  int *ancestor;
  int *label;
  int *size;
  int *child;
  BoundListMap inbounds;
  BoundListMap outbounds;
  // calculate
  void Compute();
  void ClearMiddleVariables();
  // results
  BoundListMap dominators;
  int *idominator;

private:
  void Enumerate_();
  void Build_();
  int Evaluate_(int v);
  void Compress_(int v);
  void Link_(int v, int w);
};
} // namespace tarjan

#endif