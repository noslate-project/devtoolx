(function () {
  Devtoolx.limit = 25;
  Devtoolx.ratioLimit = 0.2;
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
        statistics: { nodeCount: '-', edgeCount: '-', gcRoots: '-', totalSize: 0 }
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
        return `id: ${node.data.id}, self_size: ${this.formatSize(node.data.self_size)}`
      },
      getAdditional(node) {
        var data = node.data;
        var retainedSize = `size: ${this.formatSize(data.retainedSize)}`;
        if (this.totalSize && data.retainedSize / this.totalSize > Devtoolx.ratioLimit)
          retainedSize = `<strong style="color:#d20d0d">${retainedSize}</strong>`;
        return `(type: ${data.nodeType}, ${retainedSize}, distance: ${data.distance})`;
      }
    },
    computed: {
      totalSize() {
        return this.statistics.totalSize;
      }
    }
  })
})();