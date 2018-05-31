(function () {
  Devtoolx.limit = 25;
  Devtoolx.ratioLimit = 0.2;
  Devtoolx.tooltipsTyleSource = 'position:absolute;display:block;font-size:1.0em;'
    + 'border:1px solid #ccc;font-family:Menlo;';
  Vue.component('my-tootip', Devtoolx.ToolTip);
  new Vue({
    el: '#app',
    components: {
      'tree-edges': Devtoolx.TreeEdges,
      'tree-retainers': Devtoolx.TreeRetainers,
      'tree-dominators': Devtoolx.TreeDominators
    },
    data: function () {
      return {
        address: '',
        rootid: 0,
        nodeData: {},
        statistics: { nodeCount: '-', edgeCount: '-', gcRoots: '-', totalSize: 0 },
        tooltipStyle: Devtoolx.tooltipsTyleSource + 'opacity:0.0;',
        tooltipData: {
          index: null,
          type: 'normal',
          childName: '',
          childSize: -1,
          parentOrdinalId: -1,
          childOrdinalId: -1,
          copyedElement: ''
        }
      }
    },
    mounted() {
      var vm = this;
      vm.getNode(`/statistics`).then(data => {
        vm.$set(vm.statistics, 'nodeCount', data.node_count || '-');
        vm.$set(vm.statistics, 'edgeCount', data.edge_count || '-');
        vm.$set(vm.statistics, 'gcRoots', data.gcroots || '-');
        vm.$set(vm.statistics, 'totalSize', data.total_size || 0);
      }).catch(err => vm.$message.error(err.message || 'Server Inner Error'));
      document.onclick = this.cancelTooltip.bind(this);
    },
    methods: {
      cleanInput() {
        this.address = '';
      },
      searchIdByAddress() {
        if (!this.address) return;
        if (this.address[0] !== '@' && isNaN(this.address)) {
          this.$message.error(`Error address: "${this.address}"`);
          return;
        }
        var vm = this;
        var task = '';
        if (vm.address[0] === '@') {
          task = vm.getNode(`/address/${vm.address}?current=0&limit=${Devtoolx.limit}`);
        } else {
          task = vm.getNode(`/ordinal/${vm.address}?current=0&limit=${Devtoolx.limit}`).then(data => data[0]);
        }
        task.then(data => {
          Object.assign(vm.nodeData, data)
          vm.rootid = data.id;
        }).catch(err => vm.$message.error(err.message || 'Server Inner Error'));
      },
      getNode(url, method) {
        var vm = this;
        method = method || 'get';
        return axios[method](url).then(res => {
          var data = res.data;
          if (data.ok) {
            return data.data;
          }
          throw new Error(data.message);
        });
      },
      formatSize(size) {
        let str = '';
        if (size / 1024 < 1) {
          str = `${(size).toFixed(2)} Bytes`;
        } else if (size / 1024 / 1024 < 1) {
          str = `${(size / 1024).toFixed(2)} KB`;
        } else if (size / 1024 / 1024 / 1024 < 1) {
          str = `${(size / 1024 / 1024).toFixed(2)} MB`;
        } else {
          str = `${(size / 1024 / 1024 / 1024).toFixed(2)} GB`;
        }
        return str;
      },
      getEdgeType(node) {
        return node.data.edgeType;
      },
      getTitle(node) {
        var data = node.data || {};
        return `id: ${data.id}, self_size: ${this.formatSize(data.selfSize)}`;
      },
      getAdditional(node) {
        var data = node.data;
        var retainedSizeSource = `size: ${this.formatSize(data.retainedSize)}`;
        var retainedSize = retainedSizeSource;
        var parent = node && node.parent && node.parent || {};
        var parentData = parent.data || {};
        var parentRetainedSize = parentData.retainedSize;
        var parentSelfSize = parentData.selfSize;
        var parentDominatesCount = parentData.dominatesCount;
        if (this.totalSize && data.retainedSize / this.totalSize > Devtoolx.ratioLimit)
          retainedSize = `<strong style="color:#d20d0d">${retainedSizeSource}</strong>`;
        else if (this.totalSize && data.idomed && (parentRetainedSize / this.totalSize > Devtoolx.ratioLimit || parentData.warn)
          && data.retainedSize >= (parentRetainedSize - parentSelfSize) / parentDominatesCount) {
          retainedSize = `<strong style="color:#ff9800;font-style:italic;">${retainedSizeSource}</strong>`;
          data.warn = true;
        }
        return `(type: ${data.nodeType}, ${retainedSize}, distance: ${data.distance})`;
      },
      contextmenu(type, event, data, node, component) {
        var parentOrdinalId = node.parent && node.parent.data && node.parent.data.id;
        if (!parentOrdinalId && parentOrdinalId !== 0) parentOrdinalId = -1;
        var childOrdinalId = data.id;
        if (!childOrdinalId && childOrdinalId !== 0) childOrdinalId = -1;
        this.$set(this.tooltipData, 'index', data.index);
        this.$set(this.tooltipData, 'type', type);
        this.$set(this.tooltipData, 'childName', `${data.rawName} ${data.address}`);
        this.$set(this.tooltipData, 'childSize', data.retainedSize);
        this.$set(this.tooltipData, 'parentOrdinalId', parentOrdinalId);
        this.$set(this.tooltipData, 'childOrdinalId', childOrdinalId);
        this.tooltipStyle = Devtoolx.tooltipsTyleSource + `left:${event.pageX + 15}px;top:${event.pageY + 12}px`;
      },
      cancelTooltip() {
        this.tooltipStyle = Devtoolx.tooltipsTyleSource + 'opacity:0.0;';
      },
      nodeClick() {
        this.tooltipStyle = Devtoolx.tooltipsTyleSource + 'opacity:0.0;';
      }
    },
    computed: {
      totalSize() {
        return this.statistics.totalSize;
      }
    }
  })
})();