(function () {
  // var showRepeatSource = '<a style="cursor:pointer;text-decoration:none;color:#00bcd4">show repeat</a>';
  var showRepeatSource = '';
  var ToolTip = {
    template: '#tooltip-template',
    props: ['tooltipStyle', 'tooltipData', 'getNode', 'formatSize', 'type'],
    data() {
      return { repeatLoading: false, repeatMsg: showRepeatSource }
    },
    methods: {
      formatTooltip(name, count, sizes, index) {
        return `<strong>${name}</strong> `
          + `repeat: <span style="color:#673ab7">${count}</span>, `
          + `total sizes: <span style="color:#673ab7">${this.formatSize(sizes)}</span>`
        // + ((index || Number(index) === 0) && `, index: <span style="color:#673ab7">${index}</span>` || '');
      },
      getRepeat() {
        var vm = this;
        // if (!vm.repeatMsg.includes(showRepeatSource)) return;
        var tooltipData = vm.tooltipData;
        if (tooltipData.parentOrdinalId !== -1 && tooltipData.childOrdinalId !== -1) {
          vm.repeatLoading = true;
          vm.getNode(`/repeat/parend_id/${tooltipData.parentOrdinalId}/child_id/${tooltipData.childOrdinalId}?type=${tooltipData.type}`)
            .then(data => {
              if (data.count === 0) {
                data.count = 1;
                data.total_retained_size = tooltipData.childSize;
              }
              vm.repeatMsg = vm.formatTooltip(tooltipData.childName, data.count, data.total_retained_size, tooltipData.index);
            })
            .catch(err => vm.$message.error(err.message || 'Server Inner Error'))
            .then(() => vm.repeatLoading = false)
        } else
          vm.repeatMsg = vm.formatTooltip(tooltipData.childName, 1, tooltipData.childSize);
      },
      noop() {
        return false;
      }
    },
    computed: {
      showRepeat() {
        return this.repeatMsg;
      }
    },
    watch: {
      tooltipStyle() {
        this.getRepeat();
        // this.repeatMsg = showRepeatSource;
      }
    }
  }
  if (typeof Devtoolx === 'undefined') window.Devtoolx = {};
  Devtoolx.ToolTip = ToolTip;
})();