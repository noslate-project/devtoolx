(function () {
  var TreeRetainers = {
    template: '#tree-template',
    data() {
      return {
        props: { label: 'name', isLeaf: 'exists' },
        node: {},
        type: 'retainers',
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
      formatNode(data, retainer, raw) {
        raw = raw || {};
        raw.id = data.id;
        raw.key = `${Math.random().toString(36).substr(2)}`;
        if (data.type === 'array') data.name = `${data.name || ''}[]`;
        if (data.type === 'closure') data.name = `${data.name || ''}()`;
        if (typeof data.name === 'string' && data.name.length > 100)
          raw.name = data.name.substr(0, 100);
        else
          raw.name = data.name;
        raw.rawName = data.name;
        raw.nameClass = data.is_gcroot && 'node-name node-gcroot' || 'node-name';
        if (data.type === 'closure')
          raw.nameClass = `${raw.nameClass} closure`;
        raw.address = data.address;
        raw.selfSize = data.self_size;
        raw.nodeType = data.type;
        raw.retainedSize = data.retained_size;
        raw.distance = data.distance;
        raw.retainers = data.retainers;
        raw.retainersEnd = data.retainers_end;
        raw.retainersCurrent = data.retainers_current;
        raw.retainersLeft = data.retainers_left;
        if (!retainer) {
          raw.expandPaths = [data.address];
        }
        if (retainer) {
          if (retainer.type === 'property' || retainer.type === 'element' || retainer.type === 'shortcut') {
            raw.edgeClass = 'property';
          }
          if (retainer.type === 'context') {
            raw.edgeClass = 'context';
          }
          raw.fromEdge = `${retainer.name_or_index}`;
          raw.edgeType = retainer.type;
          raw.index = retainer.index;
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
          vm.getNode(`/ordinal/${vm.rootid}?current=0&limit=${Devtoolx.limit}&type=retainers`)
            .then(data => resolve([vm.formatNode(data[0])]))
            .catch(err => vm.$message.error(err.message || 'Server Inner Error'));
          return;
        }
        var data = node.data;
        if (node.level > 0) {
          if (data.retainers) {
            var ids = data.retainers.map(r => r.from_node).join(',');
            if (ids == '') return resolve([])
            vm.getNode(`/ordinal/${ids}/?current=0&limit=${Devtoolx.limit}&type=retainers`)
              .then((list) => {
                var result = list.map((r, i) => {
                  var result = vm.formatNode(r, data.retainers[i]);
                  vm.formatPaths(data, result, r.address);
                  return result;
                }).filter(r => r);
                if (!data.retainersEnd) {
                  result.push({
                    id: data.id,
                    loadMore: true,
                    retainersCurrent: data.retainersCurrent,
                    exists: true,
                    retainersLeft: data.retainersLeft
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
        vm.getNode(`/ordinal/${rawdata.id}?current=${rawdata.retainersCurrent}&limit=${number}&type=retainers`)
          .then(parent => {
            p = parent = parent[0];
            var ids = parent.retainers.map(r => r.from_node).join(',');
            if (ids == '') return [];
            return vm.getNode(`/ordinal/${ids}/?current=0&limit=${Devtoolx.limit}&type=retainers`);
          }).then(list => {
            list.forEach((r, i) => {
              var data = vm.formatNode(r, p.retainers[i]);
              vm.formatPaths(node.parent.data, data, r.address);
              node.parent.insertBefore({ data }, node);
            });
            if (p.retainers_end) {
              node.parent.childNodes.pop();
            } else {
              rawdata.retainersCurrent = p.retainers_current;
              rawdata.retainersLeft = p.retainers_left;
            }
            vm.$set(vm.loadMoreStatus, setKey, false);
          }).catch(err => vm.$message.error(err.message || 'Server Inner Error'));
      },
      uniqueContextmenu(event, data, node, component) {
        return false;
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
            rawdata.retainersCurrent = Number(this.customizeStart) + Number(rawdata.retainersCurrent);
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
  Devtoolx.TreeRetainers = TreeRetainers;
})();