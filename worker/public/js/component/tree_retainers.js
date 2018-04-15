(function () {
  var TreeRetainers = {
    template: '#tree',
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
                var name = `${data.name}::${data.address}`;
                vm.node.exists[name] = true;
                resolve([{
                  name,
                  fromEdge: '',
                  additional: `(type: ${data.type}, size: ${vm.formatSize(data.self_size)})`,
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
                  var name = `${r.data.name}::${r.data.address}`;
                  var result = {
                    name,
                    fromEdge: `${data.retainers[i].name_or_index}`,
                    additional: `(type: ${r.data.type}, size: ${vm.formatSize(r.data.self_size)})`,
                    retainers: r.data.retainers
                  };
                  if (node.root.exists[name]) {
                    result.exists = true;
                  } else {
                    node.root.exists[name] = true;
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
        root.data.retainers = this.nodeData.retainers;
        console.log(root.data.retainers);
        this.node.exists = {};
        this.node.exists[this.nodeData.name] = true;
      }
    }
  };
  if (typeof Devtoolx === 'undefined') window.Devtoolx = {};
  Devtoolx.TreeRetainers = TreeRetainers;
})();