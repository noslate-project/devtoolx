(function () {
  Devtoolx.limit = 50;
  new Vue({
    el: '#app',
    components: {
      'tree-edges': Devtoolx.TreeEdges,
      'tree-retainers': Devtoolx.TreeRetainers
    },
    data: function () {
      return {
        address: '',
        rootid: 0,
        nodeData: {},
        statistics: { nodeCount: '-', edgeCount: '-', gcRoots: '-' }
      }
    },
    mounted() {
      var vm = this;
      vm.getNode(`/statistics`).then(data => {
        vm.$set(vm.statistics, 'nodeCount', data.node_count || '-');
        vm.$set(vm.statistics, 'edgeCount', data.edge_count || '-');
        vm.$set(vm.statistics, 'gcRoots', data.gcroots || '-');
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
      getNodeId(node) {
        return `id: ${node.data.id}`
      }
    }
  })
})();