(function () {
  var TreeEdges = {
    template: '#tree-template',
    data() {
      return {
        props: { label: 'name', isLeaf: 'exists' },
        node: {},
        type: 'edges',
        loadMoreStatus: { b1: false, b2: false, b3: false, b4: false },
        limit: Devtoolx.limit,
        tooltipType: 'normal',
        customizeStart: 0,
        showCustomizeStart: false
      }
    },
    props: ['rootid', 'nodeData', 'getNode', 'formatSize', 'getEdgeType', 'getTitle',
      'getAdditional', 'contextmenu', 'tooltipStyle', 'nodeClick', 'tooltipData'],
    methods: {
      formatNode(data, edge, raw) {
        raw = raw || {};
        raw.id = data.id;
        raw.key = `${Math.random().toString(36).substr(2)}`;
        if (data.type === 'array') data.name = `${data.name || ''}[]`;
        if (data.type === 'closure') data.name = `${data.name || ''}()`;
        if (typeof data.name === 'string' && data.name.length > 100)
          raw.name = data.name.substr(0, 100);
        else
          raw.name = data.name;
        raw.nameClass = 'node-name';
        if (data.type === 'closure')
          raw.nameClass = `${raw.nameClass} closure`;
        raw.address = data.address;
        raw.selfSize = data.self_size;
        raw.nodeType = data.type;
        raw.retainedSize = data.retained_size;
        raw.distance = data.distance;
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
          raw.idomed = edge.idomed;
          raw.dominatesCount = data.dominates_count;
          raw.index = edge.index;
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
      loadMore(node, rawdata, number, customize) {
        var vm = this;
        var p = null;
        var setKey = customize && 'b4' || number / Devtoolx.limit === 2 && 'b1' || number / Devtoolx.limit === 4 && 'b2' || 'b3';
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
      },
      uniqueContextmenu(event, data, node, component) {
        this.contextmenu(this.tooltipType, event, data, node, component);
      },
      uniqueNodeClick() {
        this.showCustomizeStart = false;
        this.customizeStart = 0;
        this.nodeClick();
      },
      jump(node, rawdata) {
        if (this.showCustomizeStart) {
          if (isNaN(this.customizeStart)) {

          } else
            rawdata.edgesCurrent = Number(this.customizeStart) + Number(rawdata.edgesCurrent);
          this.loadMore(node, rawdata, this.limit * 2, true);
        } else {
          this.showCustomizeStart = true;
        }
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