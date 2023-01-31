'use strict';
const devtoolx = require('bindings')('devtoolx');

exports = module.exports = devtoolx;

exports.snapshot = require('./worker/snapshot');