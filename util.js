'use strict';
const logger = exports.logger = {
  infoConsole(str) {
    return `\x1b[35m${str}\x1b[0m`;
  },

  infoConsole2(str) {
    return `\x1b[32m${str}\x1b[0m`;
  },

  debugConsole(str) {
    return `\x1b[36m${str}\x1b[0m`;
  },

  errorConsole(str) {
    return `\x1b[31m${str}\x1b[0m`;
  },

  warnConsole(str) {
    return `\x1b[33m${str}\x1b[0m`;
  },

  lineConsole(str) {
    return `\x1b[4m${str}\x1b[0m`;
  }
};

exports.helpText = `
Usage: devtoolx [CMD]... [ARGS]

  ${logger.infoConsole(`-v --version`)}            show devtoolx version
  ${logger.infoConsole(`version`)}
  
  ${logger.infoConsole(`-h --help`)}               show devtoolx usage
  ${logger.infoConsole(`help`)}

  ${logger.infoConsole(`-s <file> [-p <port>]`)}   analysis js heapsnapshot, start web sever
                          with default port 3000, change port by "-p <port>"
  
  ${logger.infoConsole(`--no-open`)}               don't auto open browser when analysis completed
`;

exports.formatTime = function (ts) {
  ts = !isNaN(ts) && ts || 0;
  let str = '';
  if (ts < 1e3) {
    str = `${ts.toFixed(2)} ms`;
  } else if (ts < 1e3 * 60) {
    str = `${(ts / 1e3).toFixed(2)} s`;
  } else if (ts < 1e3 * 60 * 60) {
    str = `${(ts / (1e3 * 60)).toFixed(2)} min`;
  } else if (ts < 1e3 * 60 * 60 * 60) {
    str = `${(ts / (1e3 * 60 * 60)).toFixed(2)} h`;
  } else {
    str = `${ts.toFixed(2)} ms`;
  }

  return str;
}

exports.formatSize = function (size) {
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