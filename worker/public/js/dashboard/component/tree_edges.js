(function () {
  var TreeEdges = {
    template: '#tree-template',
    data() {
      return { props: { label: 'name', isLeaf: 'exists' }, node: {}, type: 'edges' }
    },
    props: ['rootid', 'nodeData', 'getNode', 'formatSize'],
    methods: {
      formatNode(data, edge, raw) {
        raw = raw || {};
        raw.id = data.id;
        raw.key = data.key;
        raw.name = data.name;
        raw.address = data.address;
        raw.additional = `(type: ${data.type}, self_size: ${this.formatSize(data.self_size)})`;
        raw.edges = data.edges;
        raw.edgesEnd = data.edges_end;
        raw.edgesCurrent = data.edgesCurrent;
        if (edge) {
          raw.alive = edge.type === 'property' || edge.type === 'element' || edge.type === 'shortcut';
          raw.fromEdge = edge.name_or_index
        }
        return raw;
      },

      loadNodeEdge(node, resolve) {
        var vm = this;
        if (node.level === 0) {
          vm.node = node;
          if (!vm.node.exists) vm.node.exists = {};
          vm.getNode(`/ordinal/${vm.rootid}?current=0&limit=${Devtoolx.limit}`)
            .then(data => resolve([vm.formatNode(data)]))
            .catch(err => vm.$message.error(err));
          return;
        }
        node.root = vm.node;
        var data = node.data;
        if (node.level > 0) {
          if (data.edges) {
            Promise.all(data.edges.map(e => vm.getNode(`/ordinal/${e.to_node}/?current=0&limit=${Devtoolx.limit}`))).then((list) => {
              var result = list.map((r, i) => {
                var result = vm.formatNode(r, data.edges[i]);
                if (node.root.exists[r.address]) {
                  result.exists = true;
                  result.class = 'disabled';
                } else {
                  node.root.exists[r.address] = true;
                }
                return result;
              }).filter(r => r);
              if (!data.edgesEnd) {
                result.push({ id: data.id, loadMore: true, edgesCurrent: data.edgesCurrent, exists: true });
              }
              resolve(result);
            }).catch(err => vm.$message.error(err));
          }
        }
      },
      loadMoreEdges(node, data) {
        var vm = this;
        var p = null;
        vm.getNode(`/ordinal/${data.id}?current=${data.edgesCurrent}&limit=${Devtoolx.limit}`)
          .then(parent => {
            p = parent;
            return Promise.all(parent.edges.map(e => vm.getNode(`/ordinal/${e.to_node}/?current=0&limit=${Devtoolx.limit}`)));
          })
          .then(list => {
            list.forEach((r, i) => {
              var data = vm.formatNode(r, p.edges[i]);
              node.parent.insertBefore({ data }, node);
            });
            if (p.edges_end) {
              node.parent.childNodes.pop();
            } else {
              data.edgesCurrent = p.edges_current;
            }
          }).catch(err => vm.$message.error(err));
      }
    },
    watch: {
      rootid() {
        var root = this.node.childNodes[0];
        root.childNodes = [];
        root.expanded = false;
        root.isLeaf = false;
        root.loaded = false;
        this.formatNode(this.nodeData, false, root.data);
        this.node.exists = {};
        this.node.exists[this.nodeData.address] = true;
      }
    }
  };
  if (typeof Devtoolx === 'undefined') window.Devtoolx = {};
  Devtoolx.TreeEdges = TreeEdges;
})();