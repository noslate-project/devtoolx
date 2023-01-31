(function () {
  var TreeDominators = {
    template: '#tree-template',
    data() {
      return {
        type: 'dominators',
        props: { label: 'name', isLeaf: 'exists' },
        loadMoreStatus: { b1: false, b2: false, b3: false, b4: false },
        limit: Devtoolx.limit,
        tooltipType: 'dominates',
        customizeStart: 0,
        showCustomizeStart: false
      }
    },
    props: ['getNode', 'formatSize', 'getEdgeType', 'getTitle', 'getAdditional',
      'contextmenu', 'tooltipStyle', 'nodeClick', 'tooltipData', 'rootid'],
    methods: {
      isString(data) {
        return data.type === 'concatenated string' || data.type === 'sliced string' || data.type === 'string';
      },
      formatNode(data, edge, raw) {
        raw = raw || {};
        raw.id = data.id;
        raw.key = `${Math.random().toString(36).substr(2)}`;
        if (data.type === 'array') data.name = `${data.name || ''}[]`;
        if (data.type === 'closure') data.name = `${data.name || ''}()`;
        if (this.isString(data)) data.name = `"${data.name || ''}"`;
        if (typeof data.name === 'string' && data.name.length > 100)
          raw.name = data.name.substr(0, 100);
        else
          raw.name = data.name;
        raw.rawName = data.name;
        raw.nameClass = 'node-name';
        if (this.isString(data))
          raw.nameClass = `${raw.nameClass} string`;
        if (data.type === 'closure')
          raw.nameClass = `${raw.nameClass} closure`;
        if (data.is_gcroot)
          raw.nameClass = `${raw.nameClass} node-gcroot`;
        raw.address = data.address;
        raw.selfSize = data.self_size;
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
        raw.dominatesCount = data.dominates_count;
        raw.index = data.index;
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
      loadMore(node, rawdata, number, customize) {
        var vm = this;
        var setKey = customize && 'b4' || number / Devtoolx.limit === 2 && 'b1' || number / Devtoolx.limit === 4 && 'b2' || 'b3';
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
            rawdata.dominatesCurrent = Number(this.customizeStart) + Number(rawdata.dominatesCurrent);
          this.loadMore(node, rawdata, this.limit * 2, true);
        } else {
          this.showCustomizeStart = true;
        }
      }
    }
  };
  if (typeof Devtoolx === 'undefined') window.Devtoolx = {};
  Devtoolx.TreeDominators = TreeDominators;
})();