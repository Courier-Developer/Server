---
marp: true

---

# Courier
## 一个易于扩展的及时聊天软件。

<br />
<br />

> https://github.com/Courier-Developer


<br />
<br />

- *市面上所有聊天软件的交集*

<!--
page_number: true
footer: 第13组   ==   [谢威宇, 孙彰亮, 袁瑞泽, 杨福浩, 冯开宇, 王聚海]
-->

---

# 特性

- 注册 / 登录 / 登出
- 个人 
- 查找 / 添加 / 删除好友
- 

---

# :cake: Members
## 谢威宇

- GTK+

## 孙璋亮

- 客户端底层逻辑处理
- 与RPC、上层UI、底层数据库交互

---

# :cake: Members


## 袁瑞泽

- 客户端数据库操作
- 类型转换

## 杨福浩

- 服务器数据库构建与维护
- 提供服务器数据库增、删、改、查的C++接口

---

# :trollface:Members

## 冯开宇

- 基于Socket的双向RPC框架
- 线程管理

## 王聚海

- 项目设计
- ~~啦啦操~~

---

# 技术选型

- C++
- GTKmm
- Postgres
- 双向RPC
- Msgpack 序列化方案
- MVC
- 多线程
- git/github
- CMake
- Clion + VSCode

---

# 关键代码 [FeverRPC](https://github.com/Courier-Developer/feverrpc)

这个RPC可以让你写出类似于以下的代码

客户端
```C++
// Client.cpp
#include "feverrpc.hpp"

int main(){
    FeverRPC::Client rpc("127.0.0.1");

    int ans = rpc.call<int>("add", 1, 2, 3);
}
```

---

```C++
// Server
#include "feverrpc.hpp"

int add(int a, int b, int c){
    return a + b + c;
}

int main(){
    FeverRPC::Server rpc;

    rpc.bind("add", add);
    rpc.c2s(); 
}
```

---

## 同时还...

- 使用TCP/Socket长连接
- 双向RPC
- 支持任意长度、类型参数绑定
- 基于Msgpack，可自定义序列化类型
- Socket支持任意大小传输功能 (int)
- 支持多线程，有多线程调度模块
- 服务端线程可相互通信
- 嵌入登录功能

---

## 实现方式

- std::function + std::bind
- 