(function () {
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
        axios.get(`/address/${vm.address}`)
          .then(res => {
            let data = res.data;
            if (data.ok) {
              data = res.data && res.data.data;
              vm.nodeData.name = `${data.name}::${data.address}`;
              vm.nodeData.edges = data.edges;
              vm.nodeData.retainers = data.retainers;
              vm.rootid = data.id;
            } else {
              vm.$message.error(data.message);
            }
          })
          .catch(err => vm.$message.error(err));
      }
    }
  })
})();