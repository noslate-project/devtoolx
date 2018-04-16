(function () {
  Devtoolx.limit = 100;
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
        nodeData: {}
      }
    },
    methods: {
      cleanInput() {
        this.address = '';
      },
      searchIdByAddress() {
        if (!this.address) return;
        if (this.address[0] !== '@') {
          this.$message.error('node address must starts with \'@\'!');
        }
        var vm = this;
        axios.get(`/address/${vm.address}?current=0&limit=${Devtoolx.limit}`)
          .then(res => {
            let data = res.data;
            if (data.ok) {
              data = res.data && res.data.data;
              Object.assign(vm.nodeData, data)
              vm.rootid = data.id;
            } else {
              vm.$message.error(data.message);
            }
          })
          .catch(err => vm.$message.error(err));
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
      }
    }
  })
})();