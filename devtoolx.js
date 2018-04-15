'use strict';
const devtoolx = require('bindings')('devtoolx');
const heapTools = devtoolx.heapTools;
const V8Parser = heapTools.V8Parser;

exports = module.exports = devtoolx;

exports.web = require('./worker/web');

// after is test code, will be removed
function getName() {
  const path = require('path');
  let parser = new V8Parser(path.join(__dirname, './test/resource/test.heapsnapshot'));
  console.log(parser.getFileName());
  console.time('json');
  parser.parse({ mode: 'search' });
  // console.log(parser.getNodeId(12));
  // let node = parser.getNodeByOrdinalId(1);
  let node = parser.getNodeByAddress("@1");
  // node.edges.forEach(e => e.to_node = parser.getNodeByOrdinalId(e.to_node).name);
  console.log(node);
  // console.log(parser.getNodeByOrdinalId(9999));
  console.timeEnd('json');
  // let snap = JSON.parse(require('fs').readFileSync('./test/resource/test.heapsnapshot'));
  // console.log(snap.nodes[6], snap.snapshot.meta.node_types[0][9]);
}

// getName();
// setInterval(getName, 1);

exports.web().listen(3001);