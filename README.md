# High Level C/C++ API for Smart Mug

## Setup toolchain

First of all please setup i586 toolchain 

```shell
source /opt/poky-edison/1.5.1/environment-setup-i586-poky-linux
```

## Install libuv (optional)

If you only want to use C/C++ APIs, libuv is optional. But if you want to use the Javascript API, you need to build the libuv library and create Nodejs native addon with iot-io library

```shell
cd lib
git clone https://github.com/joyent/libuv
```

Makefile will automatically detect libuv directory and setup compiling environment. If lib/libuv doesn't exist, it will build with pthread.

NOTICE: Make sure the version of libuv you checked out is the same as which of libuv used in nodejs. For example, the nodejs 0.10.28 uses libuv 0.10.27, then you need to checkout the right version.

```shell
cd lib/libuv
git checkout v0.10.27
```

Then follow the README to build the libuv. Don't forget to setup i586 toolchain

## Build

* Please build the iohub library. 

* Set up i586 toolchain

* Build libmug.a

```shell
make
```

* Build test

```shell
make test
```
