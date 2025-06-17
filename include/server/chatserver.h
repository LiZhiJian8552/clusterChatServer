#pragma once

#include<muduo/net/TcpServer.h>
#include<muduo/net/EventLoop.h>

using namespace muduo;
using namespace muduo::net;


class ChatServer{
public:
    // 初始化聊天服务器对象
    ChatServer(EventLoop* loop,const InetAddress& listenAddr,const string& nameArg);
    // 启动服务
    void start();
private:
    // 与链接相关的回调
    void onConnection(const TcpConnectionPtr&);
    // 读写事件相关的回调
    void onMessage(const TcpConnectionPtr&,Buffer*,Timestamp);

    TcpServer _server;
    EventLoop*_loop;
};