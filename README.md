# High Level C/C++ API for Smart Mug

## Build

* Please build the iohub library. 

* Set up toolchain for Edison. 

```shell
source /opt/poky-edison/1.5.1/environment-setup-i586-poky-linux
```

* Install and build libuv

```shell
cd lib
git clone https://github.com/joyent/libuv
cd libuv
./gyp_uv.py -f make -Dtarget_arch=ia32
make -C out
cd ../..
```
 
* Build libmug.a

```shell
make
```

* Build test

```shell
make test
```
