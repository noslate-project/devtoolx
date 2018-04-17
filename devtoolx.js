'use strict';
const devtoolx = require('bindings')('devtoolx');
const heapTools = devtoolx.heapTools;
const V8Parser = heapTools.V8Parser;

exports = module.exports = devtoolx;

exports.snapshot = require('./worker/snapshot');