# High Level C/C++ API for Smart Mug

## Install Edison SDK Linux

Download and install Edison SDK for Linux from https://communities.intel.com/docs/DOC-23242. By default it is installed on /opt/poky-edison/1.5.1/  

## Build

Setup Edison toolchain environment 

``` shell
source /opt/poky-edison/1.5.1/environment-setup-i586-poky-linux
```

Make debug version

```shell
make
```

Make release version

```shell
make Release=1
```

## Build test applications

```shell
make test
```

## Contacts

Project manager: Jiqiang Song, jiqiang.song@intel.com

SDK: Zhanglin Liu, zhanglin.liu@intel.com 

App Management: Chao Zhang, chao.a.zhang@intel.com


