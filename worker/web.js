'use strict';
const express = require('express');
const path = require('path');
const bodyParser = require('body-parser');
const serverFavicon = require('serve-favicon');
const compression = require('compression');
const devtoolx = require('..');
const heapTools = devtoolx.heapTools;
const V8Parser = heapTools.V8Parser;

function createServer() {
  let parser = new V8Parser(path.join(__dirname, '../test/resource/test.heapsnapshot'));
  parser.parse({ mode: 'search' });

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
      let node = parser.getNodeByAddress(req.params.address);
      res.json({ ok: true, data: node });
    } catch (e) {
      res.json({ ok: false, message: e.message });
    }
  });

  app.get('/ordinal/:ordinal', (req, res) => {
    try {
      let node = parser.getNodeByOrdinalId(parseInt(req.params.ordinal));
      res.json({ ok: true, data: node });
    } catch (e) {
      res.json({ ok: false, message: e.message });
    }
  });

  return app;
}

module.exports = createServer;