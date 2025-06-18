#include "chatserver.h"
#include "json.hpp"
#include"chatservice.h"

#include<functional>
#include<string>
using json=nlohmann::json;

ChatServer::ChatServer(EventLoop* loop,
                        const InetAddress& listenAddr,
                        const string& nameArg)
                        :_server(loop,listenAddr,nameArg),
                        _loop(loop)
{
    // 注册链接回调
    _server.setConnectionCallback(std::bind(&ChatServer::onConnection,this,std::placeholders::_1));

    // 注册消息回调
    _server.setMessageCallback(std::bind(&ChatServer::onMessage,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3));

    // 设置线程数量
    _server.setThreadNum(4);
}

void ChatServer::start(){
    _server.start();
}

void ChatServer::onConnection(const TcpConnectionPtr & conn){
    // 客户端断开链接
    if(!conn->connected()){
        conn->shutdown();
    }
}

void ChatServer::onMessage(const TcpConnectionPtr &conn, Buffer *buffer, Timestamp time){
    string buf=buffer->retrieveAllAsString();
    // 数据的反序列化：数据的解码
    json js=json::parse(buf);
    // 通过js["msgid"]获取==》业务handler

    // 获取msgid对应的处理器,js["msgid"].get<int>()将json键对应的值强转成某个类型
    auto msgHandler=ChatService::instance()->getHandler(js["msgid"].get<int>());
    // 回调消息绑定好的事件处理器，来执行相应的业务处理
    msgHandler(conn,js,time);
}
