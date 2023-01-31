'use strict';
const express = require('express');
const path = require('path');
const bodyParser = require('body-parser');
const serverFavicon = require('serve-favicon');
const compression = require('compression');
const devtoolx = require('..');
const logger = require('../util').logger;
const formatTime = require('../util').formatTime;
const heapTools = devtoolx.heapTools;
const V8Parser = heapTools.V8Parser;

function checkOptions(options) {
  let method = options.method;
  if (method && (method !== 'tarjan') && method !== 'data_iteration') {
    throw new Error(`not support method ${method}!`);
  }
}

function getMethod(method) {
  let name = '';
  switch (method) {
    case 'tarjan':
      name = 'Lengauer Tarjan';
      break;
    case 'data_iteration':
      name = 'Data Itreation';
      break;
    default:
      name = 'Data Itreation';
      break;
  }
  return name;
}

function createServer(snapshot, options) {
  checkOptions(options);
  let parser = new V8Parser(snapshot);
  process.stdout.write(logger.debugConsole(` <Using ${getMethod(options.method)}>\n`));
  parser.parse(Object.assign({ mode: 'search' }, options), function (progress) {
    let message = '';
    if (progress.status === 'start') {
      message = ` - ${progress.type}...`;
    }
    if (progress.status === 'end') {
      message = `\x1b[3D\x1b[K ${logger.warnConsole(`${formatTime(progress.cost)}`)}\n`;
    }
    process.stdout.write(logger.debugConsole(`${message}`));
  });
  let app = express();
  app.set('views', path.join(__dirname, './view'));
  app.engine('.html', require('ejs').renderFile);
  app.set('view engine', 'html');
  app.use(compression());
  app.use(express.static(path.join(__dirname, './public')));
  app.use(serverFavicon(path.join(__dirname, './public/favicon.ico')));
  app.use(bodyParser.json({ limit: '50mb' }));
  app.use(bodyParser.urlencoded({ extended: false, limit: '10mb' }));

  app.get('/', (req, res) => {
    res.render('index', {
      msg: 'hello'
    });
  });

  app.get('/address/:address', (req, res) => {
    try {
      let current = !isNaN(req.query.current) && parseInt(req.query.current);
      let limit = !isNaN(req.query.limit) && parseInt(req.query.limit);
      let node = parser.getNodeByAddress(req.params.address, current, limit);
      res.json({ ok: true, data: node });
    } catch (e) {
      res.json({ ok: false, message: e.message });
    }
  });

  app.get('/ordinal/:ordinal', (req, res) => {
    try {
      let ids = req.params.ordinal.split(',').map(id => parseInt(id));
      let current = !isNaN(req.query.current) && parseInt(req.query.current);
      let limit = !isNaN(req.query.limit) && parseInt(req.query.limit);
      let type = req.query.type;
      let node = parser.getNodeByOrdinalId(ids, current, limit, { type });
      res.json({ ok: true, data: node });
    } catch (e) {
      res.json({ ok: false, message: e.message });
    }
  });

  app.get('/statistics', (req, res) => {
    try {
      let statistics = parser.getStatistics();
      res.json({ ok: true, data: statistics });
    } catch (e) {
      res.json({ ok: false, message: e.message });
    }
  });

  app.get('/dominates/:id', (req, res) => {
    try {
      let id = parseInt(req.params.id);
      let current = !isNaN(req.query.current) && parseInt(req.query.current);
      let limit = !isNaN(req.query.limit) && parseInt(req.query.limit);
      let dominates = parser.getDominatorByIDom(id, current, limit);
      res.json({ ok: true, data: dominates });
    } catch (e) {
      res.json({ ok: false, message: e.message });
    }
  });

  app.get('/repeat/parend_id/:parend_id/child_id/:child_id', (req, res) => {
    try {
      let parendId = !isNaN(req.params.parend_id) && parseInt(req.params.parend_id);
      let childId = !isNaN(req.params.child_id) && parseInt(req.params.child_id);
      let repeat = parser.getChildRepeat(parendId, childId, req.query.type);
      res.json({ ok: true, data: repeat });
    } catch (e) {
      res.json({ ok: false, message: e.message });
    }
  });

  return app;
}

module.exports = createServer;