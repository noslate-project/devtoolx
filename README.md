# <span style="font-family:Menlo;">DevTools X</span>

[![npm version](https://badge.fury.io/js/devtoolx.svg)](https://badge.fury.io/js/devtoolx)
[![Package Quality](http://npm.packagequality.com/shield/devtoolx.svg)](http://packagequality.com/#?package=devtoolx)
[![npm](https://img.shields.io/npm/dt/devtoolx.svg)](https://www.npmjs.com/package/devtoolx)
[![license](https://img.shields.io/github/license/mashape/apistatus.svg)](LICENSE)

<span style="font-family:Menlo;">JavaScript Developer Toolbox</span>

* <span style="font-family:Menlo;">Aid in the analysis of memory</span>

## <span style="font-family:Menlo;">Installation</span>

```bash
npm install devtoolx -g
```

## <span style="font-family:Menlo;">Compatibility</span>

* <span style="font-family:Menlo;">Node.js v4.x</span>
* <span style="font-family:Menlo;">Node.js v6.x</span>
* <span style="font-family:Menlo;">Node.js v8.x</span>

<span style="font-family:Menlo;">Attention: local compilation needs gcc version >= v4.9.2</span>

## <span style="font-family:Menlo;">Usage</span>

### <span style="font-family:Menlo;">Search node by address or ordinal id</span>

```bash
devtoolx -s <heapsnapshot file> [-p <port>]
```

<span style="font-family:Menlo;">example:</span>

![devtoox.gif](https://raw.githubusercontent.com/hyj1991/devtoolx/master/assets/devtoolx.gif)

### <span style="font-family:Menlo;">Color</span>

* <span style="font-family:Menlo;background:#c0eafd">light bule: means gc root</span>

### <span style="font-family:Menlo;">Help</span>

```bash
devtoolx -h
devtoolx --help
```

## <span style="font-family:Menlo;">License</span>

[<span style="font-family:Menlo;">MIT License</span>](LICENSE)

<span style="font-family:Menlo;">Copyright (c) 2018 hyj1991</span>
