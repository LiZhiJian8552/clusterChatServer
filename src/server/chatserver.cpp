#include "chatserver.h"
#include<functional>

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

void ChatServer::onConnection(const TcpConnectionPtr &){

}

void ChatServer::onMessage(const TcpConnectionPtr &, Buffer *, Timestamp){
    
}
