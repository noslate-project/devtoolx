'use strict';
const path = require('path');
const devtoolx = require('..');
const heapTools = devtoolx.heapTools;
const V8Parser = heapTools.V8Parser;
const snapshot = path.join(__dirname, './resource/test.heapsnapshot');

function getNode() {
  let parser = new V8Parser(snapshot);
  parser.parse({ mode: 'search' });

  // let node = parser.getNodeByOrdinalId([1, 2, 3], 0, 2)
  // let node = parser.getNodeByOrdinalId([1, 2, 3], 0, 2, { type: 'retainers' })
  // let node = parser.getNodeByOrdinalId([1, 2, 3], 0, 2)
  let nodes = parser.getDominatorByIDom(0, 2, 1);

  console.log(nodes, nodes.length);
}


getNode();

console.time('cost');
// require('..').snapshot(snapshot).listen(3001, () => console.timeEnd('cost'));
