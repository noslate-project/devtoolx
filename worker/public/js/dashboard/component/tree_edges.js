(function () {
  var TreeEdges = {
    template: '#tree-template',
    data() {
      return {
        props: { label: 'name', isLeaf: 'exists' },
        node: {},
        type: 'edges',
        loadMoreStatus: { b1: false, b2: false, b3: false },
        limit: Devtoolx.limit
      }
    },
    props: ['rootid', 'nodeData', 'getNode', 'formatSize', 'getEdgeType', 'getNodeId'],
    methods: {
      formatNode(data, edge, raw) {
        raw = raw || {};
        raw.id = data.id;
        raw.key = `${Math.random().toString(36).substr(2)}`;
        if (typeof data.name === 'string' && data.name.length > 54) {
          raw.name = data.name.substr(0, 54);
        } else {
          raw.name = data.name;
        }
        raw.address = data.address;
        raw.additional = `(type: ${data.type}, self_size: ${this.formatSize(data.self_size)}, distance: ${data.distance})`;
        raw.edges = data.edges;
        raw.edgesEnd = data.edges_end;
        raw.edgesCurrent = data.edges_current;
        raw.edgesLeft = data.edges_left;
        if (!edge) {
          raw.expandPaths = [data.address];
        }
        if (edge) {
          if (edge.type === 'property' || edge.type === 'element' || edge.type === 'shortcut') {
            raw.edgeClass = 'property';
          }
          if (edge.type === 'context') {
            raw.edgeClass = 'context';
          }
          raw.fromEdge = `${edge.name_or_index}`;
          raw.edgeType = edge.type;
        }
        return raw;
      },
      formatPaths(parent, child, address) {
        child.expandPaths = parent.expandPaths.concat([address]);
        if (~parent.expandPaths.indexOf(address)) {
          child.exists = true;
          child.class = 'disabled';
        }
      },
      loadNode(node, resolve) {
        var vm = this;
        if (node.level === 0) {
          vm.node = node;
          vm.getNode(`/ordinal/${vm.rootid}?current=0&limit=${Devtoolx.limit}&type=edges`)
            .then(data => resolve([vm.formatNode(data[0])]))
            .catch(err => vm.$message.error(err.message || 'Server Inner Error'));
          return;
        }
        var data = node.data;
        if (node.level > 0) {
          if (data.edges) {
            var ids = data.edges.map(e => e.to_node).join(',');
            if (ids == '') return resolve([])
            vm.getNode(`/ordinal/${ids}/?current=0&limit=${Devtoolx.limit}&type=edges`)
              .then((list) => {
                var result = list.map((r, i) => {
                  var result = vm.formatNode(r, data.edges[i]);
                  vm.formatPaths(data, result, r.address);
                  return result;
                }).filter(r => r);
                if (!data.edgesEnd) {
                  result.push({
                    id: data.id,
                    loadMore: true,
                    edgesCurrent: data.edgesCurrent,
                    exists: true,
                    edgesLeft: data.edgesLeft
                  });
                }
                resolve(result);
              }).catch(err => vm.$message.error(err.message || 'Server Inner Error'));
          }
        }
      },
      loadMore(node, rawdata, number) {
        var vm = this;
        var p = null;
        var setKey = number / Devtoolx.limit === 1 && 'b1' || number / Devtoolx.limit === 2 && 'b2' || 'b3';
        vm.$set(vm.loadMoreStatus, setKey, true);
        vm.getNode(`/ordinal/${rawdata.id}?current=${rawdata.edgesCurrent}&limit=${number}&type=edges`)
          .then(parent => {
            p = parent = parent[0];
            var ids = parent.edges.map(e => e.to_node).join(',');
            if (ids == '') return [];
            return vm.getNode(`/ordinal/${ids}/?current=0&limit=${Devtoolx.limit}&type=edges`)
          }).then(list => {
            list.forEach((r, i) => {
              var data = vm.formatNode(r, p.edges[i]);
              vm.formatPaths(node.parent.data, data, r.address);
              node.parent.insertBefore({ data }, node);
            });
            if (p.edges_end) {
              node.parent.childNodes.pop();
            } else {
              rawdata.edgesCurrent = p.edges_current;
              rawdata.edgesLeft = p.edges_left;
            }
            vm.$set(vm.loadMoreStatus, setKey, false);
          }).catch(err => vm.$message.error(err.message || 'Server Inner Error'));
      }
    },
    watch: {
      rootid() {
        var root = this.node.childNodes[0];
        root.childNodes = [];
        root.expanded = false;
        root.isLeaf = false;
        root.loaded = false;
        this.formatNode(this.nodeData, null, root.data);
      }
    }
  };
  if (typeof Devtoolx === 'undefined') window.Devtoolx = {};
  Devtoolx.TreeEdges = TreeEdges;
})();