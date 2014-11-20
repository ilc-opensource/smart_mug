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

## Javascript Support

If you are not familiar with node-gyp, please install nodejs whose version is same as nodejs used on Edison. Currently it is v0.10.28. 

Get io-js library

```shell
git clone https://github.com/ilc-opensource/io-js
cd io-js
git checkout mug
```

Build with Edison toolchain. It may report a doc generation error, please ignore it.

```shell
source /opt/poky-edison/1.5.1/environment-setup-i586-poky-linux
make
```

Now all of smart mug's C/C++ APIs are exposed as Javascript, all of io-js's features can be used. If you only care about smart mug APIs, you can only copy the dirctory io-js/target/device to mug and use it as a normal. Below is a simple example of displaying text on mug in Javascript

```javascript
var IOLIB = require('./device');

var io = new IOLIB.IO()

var handle = io.mug_disp_init();

io.mug_init_font("");

io.mug_disp_text_marquee(handle, "hello", "yellow", 100, -1);

```
## Python Support

Todo

## Cloud Service

The cloud service for WeChat, remote draw etc, please visit: https://github.com/ilc-opensource/smart_mug_cloud

## UI & App management

Please visit: https://github.com/ilc-opensource/app_resource_management

## Licence

Free BSD

## Contacts

Project Manager: Jiqiang Song, jiqiang.song@intel.com

Programming: Zhanglin Liu, zhanglin.liu@intel.com 

Cloud Service & App Management: Chao Zhang, chao.a.zhang@intel.com


