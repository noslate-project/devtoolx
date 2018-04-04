'use strict';
const devtoolx = require('bindings')('devtoolx');
const heapTools = devtoolx.heapTools;
const V8Parser = heapTools.V8Parser;


function getName() {
  const path = require('path');
  let parser = new V8Parser(path.join(__dirname, './test/resource/test.heapsnapshot'));
  console.log(parser.getFileName());
  console.time('json');
  parser.parse();
  console.timeEnd('json');
  // let snap = JSON.parse(require('fs').readFileSync('./test/resource/test.heapsnapshot'));
  // console.log(snap.nodes.length);
}

getName();
// setInterval(getName, 1);