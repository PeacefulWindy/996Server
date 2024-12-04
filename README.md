# 996Server
类skynet的服务端框架。

使用的库具体参考vcpkg.json

当前支持的功能：
---
网络：
- websocket(仅服务端)
- http
- tcp

序列化：
- json
- protobuf

数据库：
- mariadb(兼容mysql)
- redis

加密：
- base64
- sha3
- rsa

- timer
文档
---
待补充

编译(以996Server根目录为准)
---
Windows：需安装支持C++20的编译器(VS2022 17.8+)，CMake,Git
```
shell\buildDebug.bat或shell\buildRelease.bat
PS：也可以用VS2022打开CMakeLists.txt
```
Linux：以Linux Mint xfce 21.2为准(兼容Ubuntu系)
```
./shell/buildDebug.sh或./shell/buildRelease.sh
```

使用
---
Windows：
```
996Server.exe configs/exampleConfig.lua
```
Linux：
```
./996Server configs/exampleConfig.lua
```

禁止事项
---
- 在淘宝/csdn等任何平台出售本代码(不管获得任何收益)
- GitCode代码托管平台(因GitCode大批量搬运Github仓库而禁止)
- 不遵守AGPL-3.0的使用者(以邮件形式向作者申请，获得商业授权除外)
