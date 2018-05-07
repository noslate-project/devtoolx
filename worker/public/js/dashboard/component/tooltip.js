(function () {
  var showRepeatSource = 'show repeat';
  var ToolTip = {
    template: '#tooltip-template',
    props: ['tooltipStyle', 'tooltipData', 'getNode', 'formatSize', 'type'],
    data() {
      return { repeatLoading: false, repeatMsg: showRepeatSource }
    },
    methods: {
      formatTooltip(name, count, sizes) {
        return `<strong>${name}</strong> repeat: ` +
          `<span style="color:#3f51b5">${count}</span>, total sizes: ` +
          `<span style="color:#3f51b5">${this.formatSize(sizes)}</span>`;
      },
      getRepeat() {
        var vm = this;
        if (!vm.repeatMsg.includes(showRepeatSource)) return;
        var tooltipData = vm.tooltipData;
        if (tooltipData.parentOrdinalId !== -1 && tooltipData.childOrdinalId !== -1) {
          vm.repeatLoading = true;
          vm.getNode(`/repeat/parend_id/${tooltipData.parentOrdinalId}/child_id/${tooltipData.childOrdinalId}?type=${this.type}`)
            .then(data => {
              if (data.count === 0) {
                data.count = 1;
                data.total_retained_size = tooltipData.childSize;
              }
              vm.repeatMsg = vm.formatTooltip(tooltipData.childName, data.count, data.total_retained_size);
            })
            .catch(err => vm.$message.error(err.message || 'Server Inner Error'))
            .then(() => vm.repeatLoading = false)
        } else
          vm.repeatMsg = vm.formatTooltip(tooltipData.childName, 1, tooltipData.childSize);
      }
    },
    computed: {
      showRepeat() {
        return this.repeatMsg;
      }
    },
    watch: {
      tooltipStyle() {
        this.repeatMsg = showRepeatSource;
      }
    }
  }
  if (typeof Devtoolx === 'undefined') window.Devtoolx = {};
  Devtoolx.ToolTip = ToolTip;
})();