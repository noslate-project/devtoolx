#include "tarjan.h"

namespace tarjan
{
  TarJan::TarJan(tarjan_bound_list_t *data)
  {
    int count = data->count;
    inbounds = data->inbounds;
    outbounds = data->outbounds;
    length = count + 1;
    dom = new int[length];
    root = data->root;
    dfs = new int[length];
    vertex = new int[length];
    semi = new int[length];
    parent = new int[length];
    // bucket = new int[count + 1];
    ancestor = new int[length];
    label = new int[length];
    size = new int[length];
    child = new int[length];
    // init
    for (int i = 0; i < length; ++i)
    {
      dfs[i] = 0;
      vertex[i] = ERROR_VALUE;
      semi[i] = i;
      parent[i] = 0;
      // bucket[i] = [];
      ancestor[i] = 0;
      label[i] = i;
      size[i] = 1;
      child[i] = 0;
      dom[i] = 0;
    }

    size[0] = 0;
  }

  TarJan::~TarJan()
  {
    ClearMiddleVariables();
    dominators.clear();
    delete idominator;
  }

  void TarJan::Compute()
  {
    Enumerate_();
    Build_();
    idominator = new int[length];
    for (int i = 0; i < length; ++i)
    {
      idominator[i] = ERROR_VALUE;
    }
    for (int i = 1; i < length; ++i)
    {
      if (dom[i] == 0)
        continue;
      int dominator = vertex[dom[i]];
      int block = vertex[i];
      dominators[dominator].emplace_back(block);
      idominator[block] = dominator;
    }
  }

  void TarJan::Enumerate_()
  {
    std::vector<int> queue = {root, 0};
    int dfs = 1;
    while (!queue.empty())
    {
      int parent = queue.back();
      queue.pop_back();
      int block = queue.back();
      queue.pop_back();
      if (this->dfs[block] != 0)
        continue;
      this->dfs[block] = dfs;
      this->parent[dfs] = parent;
      vertex[dfs] = block;
      if (outbounds.count(block) != 0)
      {
        std::vector<int> successors = outbounds.at(block);
        for (int i = static_cast<int>(successors.size()) - 1; i >= 0; i--)
        {
          int succ = successors[i];
          queue.emplace_back(succ);
          queue.emplace_back(dfs);
        }
      }
      ++dfs;
    }
  }

  void TarJan::Build_()
  {
    for (int w = length - 1; w >= 2; w--)
    {
      if (inbounds.count(vertex[w]) != 0)
      {
        for (auto predecessor : inbounds.at(vertex[w]))
        {
          int v = dfs[predecessor];
          int u = Evaluate_(v);
          if (semi[u] < semi[w])
            semi[w] = semi[u];
        }
      }
      bucket[semi[w]].emplace_back(w);
      Link_(parent[w], w);
      std::vector<int> &parent_bucket = bucket[parent[w]];
      while (!parent_bucket.empty())
      {
        int v = parent_bucket.back();
        parent_bucket.pop_back();
        int u = Evaluate_(v);
        if (semi[u] < semi[v])
          dom[v] = u;
        else
          dom[v] = parent[w];
      }
    }

    for (int w = 2; w < length; ++w)
      if (dom[w] != semi[w])
        dom[w] = dom[dom[w]];
    dom[1] = 0;
  }

  int TarJan::Evaluate_(int v)
  {
    if (ancestor[v] == 0)
      return label[v];
    Compress_(v);
    if (semi[label[ancestor[v]]] >= semi[label[v]])
      return label[v];
    else
      return label[ancestor[v]];
  }

  void TarJan::Link_(int v, int w)
  {
    int s = w;
    while (semi[label[w]] < semi[label[child[s]]])
    {
      if (size[s] + size[child[child[s]]] >= 2 * size[child[s]])
      {
        ancestor[child[s]] = s;
        child[s] = child[child[s]];
      }
      else
      {
        size[child[s]] = size[s];
        ancestor[s] = child[s];
        s = ancestor[s];
      }
    }
    label[s] = label[w];
    size[v] += size[w];
    if (size[v] < 2 * size[w])
    {
      int t = s;
      s = child[v];
      child[v] = t;
    }
    while (s != 0)
    {
      ancestor[s] = v;
      s = child[s];
    }
  }

  void TarJan::Compress_(int v)
  {
    if (ancestor[ancestor[v]] == 0)
      return;
    Compress_(ancestor[v]);
    if (semi[label[ancestor[v]]] < semi[label[v]])
      label[v] = label[ancestor[v]];
    ancestor[v] = ancestor[ancestor[v]];
  }

  void TarJan::ClearMiddleVariables()
  {
    length = 0;
    root = 0;
    delete dfs;
    delete vertex;
    delete semi;
    delete parent;
    bucket.clear();
    delete dom;
    delete ancestor;
    delete label;
    delete size;
    delete child;
    inbounds.clear();
    outbounds.clear();
  }

} // namespace tarjan