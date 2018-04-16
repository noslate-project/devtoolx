(function () {
  var TreeRetainers = {
    template: '#tree-template',
    data() {
      return { props: { label: 'name', isLeaf: 'exists' }, node: {}, type: 'retainers' }
    },
    props: ['rootid', 'nodeData'],
    methods: {
      loadNodeEdge(node, resolve) {
        var vm = this;
        if (node.level === 0) {
          vm.node = node;
          if (!vm.node.exists) vm.node.exists = {};
          axios.get(`/ordinal/${vm.rootid}`)
            .then(res => {
              let data = res.data;
              if (data.ok) {
                data = res.data && res.data.data;
                vm.node.exists[data.address] = true;
                resolve([{
                  name: data.name,
                  address: data.address,
                  fromEdge: '',
                  additional: `(type: ${data.type}, self_size: ${vm.formatSize(data.self_size)})`,
                  retainers: data.retainers
                }]);
              } else {
                vm.$message.error(data.message);
              }
            })
            .catch(err => vm.$message.error(err));
          return;
        }
        node.root = vm.node;
        var data = node.data;
        if (node.level > 0) {
          if (data.retainers) {
            var task = data.retainers.map(e => {
              return axios.get(`/ordinal/${e.from_node}`).then(res => res.data)
            });
            Promise.all(task).then((list) => {
              var result = list.map((r, i) => {
                if (r.ok) {
                  var result = {
                    name: r.data.name,
                    address: r.data.address,
                    alive: data.retainers[i].type === 'property' || data.retainers[i].type === 'element' || data.retainers[i].type === 'shortcut',
                    fromEdge: `${data.retainers[i].name_or_index}`,
                    additional: `(type: ${r.data.type}, self_size: ${vm.formatSize(r.data.self_size)})`,
                    retainers: r.data.retainers
                  };
                  if (node.root.exists[r.data.address]) {
                    result.exists = true;
                    result.class = 'disabled';
                  } else {
                    node.root.exists[r.data.address] = true;
                  }
                  return result;
                }
              }).filter(r => r);
              resolve(result);
            }).catch(err => vm.$message.error(err));
          }
        }
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
    },
    watch: {
      rootid() {
        var root = this.node.childNodes[0];
        root.childNodes = [];
        root.expanded = false;
        root.isLeaf = false;
        root.loaded = false;
        root.data.name = this.nodeData.name;
        root.data.address = this.nodeData.address;
        root.data.retainers = this.nodeData.retainers;
        root.data.additional = `(type: ${this.nodeData.type}, self_size: ${this.nodeData.self_size})`;
        this.node.exists = {};
        this.node.exists[this.nodeData.address] = true;
      }
    }
  };
  if (typeof Devtoolx === 'undefined') window.Devtoolx = {};
  Devtoolx.TreeRetainers = TreeRetainers;
})();