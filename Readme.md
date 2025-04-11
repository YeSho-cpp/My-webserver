# TinyWebServer


Linux下C++轻量级Web服务器，助力初学者快速实践网络编程，搭建属于自己的服务器.

- 使用 **线程池** + **非阻塞**socket + epoll(ET和LT均实现) + 事件处理(Reactor和模拟Proactor均实现) 的并发模型
- 使用状态机解析HTTP请求报文，支持解析**GET**和**POST**请求
- 访问服务器数据库实现web端用户注册、登录功能，可以请求服务器图片和视频文件
- 实现**同步/异步日志系统**，记录服务器运行状态
- 经Webbench压力测试可以实现上万的并发连接数据交换

# 框架

<img src="https://camo.githubusercontent.com/326c456073716c6a81d925154df43a7787cf4088b794590c76a0f122274e7ef4/687474703a2f2f7777312e73696e61696d672e636e2f6c617267652f303035544a3263376c79316765306a3161747135686a33306736306c6d3077342e6a7067" alt="image.png" style="zoom:100%;" />


# 项目记录

- [一起写webserver 项目(一) | My Blog](https://www.yesho.top/2024/04/29/%E4%B8%80%E8%B5%B7%E5%86%99webserver%20%E9%A1%B9%E7%9B%AE(%E4%B8%80)/)
- [一起写webserver 项目(二) | My Blog](https://www.yesho.top/2024/04/29/%E4%B8%80%E8%B5%B7%E5%86%99webserver%20%E9%A1%B9%E7%9B%AE(%E4%BA%8C)/)
- [一起写webserver 项目(三) | My Blog](https://www.yesho.top/2024/04/30/%E4%B8%80%E8%B5%B7%E5%86%99webserver%20%E9%A1%B9%E7%9B%AE(%E4%B8%89)/)
- [一起写webserver 项目(四) | My Blog](https://www.yesho.top/2024/04/30/%E4%B8%80%E8%B5%B7%E5%86%99webserver%20%E9%A1%B9%E7%9B%AE(%E5%9B%9B)/)
- [一起写webserver 项目(五) | My Blog](https://www.yesho.top/2024/04/30/%E4%B8%80%E8%B5%B7%E5%86%99webserver%20%E9%A1%B9%E7%9B%AE(%E4%BA%94)/)

