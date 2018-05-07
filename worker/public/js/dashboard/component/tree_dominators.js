(function () {
  var TreeDominators = {
    template: '#tree-template',
    data() {
      return {
        type: 'dominators',
        props: { label: 'name', isLeaf: 'exists' },
        loadMoreStatus: { b1: false, b2: false, b3: false },
        limit: Devtoolx.limit
      }
    },
    props: ['getNode', 'formatSize', 'getEdgeType', 'getTitle', 'getAdditional'],
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
        if (data.type === 'closure')
          raw.nameClass = `${raw.nameClass} closure`;
        if (data.is_gcroot)
          raw.nameClass = `${raw.nameClass} node-gcroot`;
        raw.address = data.address;
        raw.self_size = data.self_size;
        raw.nodeType = data.type;
        raw.retainedSize = data.retained_size;
        raw.distance = data.distance;
        if (data.edge_name_or_index) {
          var edgeType = data.edge_type;
          if (edgeType === 'property' || edgeType === 'element' || edgeType === 'shortcut') {
            raw.edgeClass = 'property';
          }
          if (edgeType === 'context') {
            raw.edgeClass = 'context';
          }
          raw.fromEdge = data.edge_name_or_index || '';
          raw.edgeType = edgeType;
        }
        raw.idomed = true;
        return raw;
      },
      loadNode(node, resolve) {
        var vm = this;
        if (node.level === 0) {
          vm.getNode(`/ordinal/${vm.rootid}?current=0&limit=${Devtoolx.limit}&type=retainers`)
            .then(data => resolve([vm.formatNode(data[0])]))
            .catch(err => vm.$message.error(err.message || 'Server Inner Error'));
          return;
        }
        vm.getNode(`/dominates/${node.data.id}?current=0&limit=${Devtoolx.limit}`)
          .then(data => {
            var dominates = data.dominates;
            var results = dominates.map(dominate => vm.formatNode(dominate));
            if (!data.dominates_end) {
              results.push({
                id: node.data.id,
                loadMore: true,
                dominatesCurrent: data.dominates_current,
                exists: true,
                dominatesLeft: data.dominates_left
              });
            }
            resolve(results);
          }).catch(err => vm.$message.error(err.message || 'Server Inner Error'));
      },
      loadMore(node, rawdata, number) {
        var vm = this;
        var setKey = number / Devtoolx.limit === 2 && 'b1' || number / Devtoolx.limit === 4 && 'b2' || 'b3';
        vm.$set(vm.loadMoreStatus, setKey, true);
        vm.getNode(`/dominates/${rawdata.id}?current=${rawdata.dominatesCurrent}&limit=${number}`)
          .then(data => {
            data.dominates.forEach((dominate, i) => {
              var data = vm.formatNode(dominate);
              node.parent.insertBefore({ data }, node);
            });
            if (data.dominates_end)
              node.parent.childNodes.pop();
            else {
              rawdata.dominatesCurrent = data.dominates_current;
              rawdata.dominatesLeft = data.dominates_left;
            }
            vm.$set(vm.loadMoreStatus, setKey, false);
          }).catch(err => vm.$message.error(err.message || 'Server Inner Error'));;
      }
    }
  };
  if (typeof Devtoolx === 'undefined') window.Devtoolx = {};
  Devtoolx.TreeDominators = TreeDominators;
})();