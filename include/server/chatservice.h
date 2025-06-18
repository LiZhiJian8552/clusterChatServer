#pragma once

#include<muduo/net/TcpConnection.h>
#include<unordered_map>
#include"json.hpp"
#include<functional>
using namespace muduo;
using namespace muduo::net;
using json=nlohmann::json;

// 处理消息的事件回调方法类型
using MsgHandler=std::function<void(const TcpConnectionPtr& conn,json&js,Timestamp time)>;

// 聊天服务器业务类
class ChatService{
public:
    // 获取单例对象的接口函数
    static ChatService* instance();
    // 处理登录业务
    void login(const TcpConnectionPtr&conn,json& js,Timestamp time);
    // 处理注册业务
    void reg(const TcpConnectionPtr&conn,json& js,Timestamp time);
    // 没有对应的处理器时的默认处理器
    void defaultHandler(const TcpConnectionPtr& conn,json& js,Timestamp time);
    // 获取消息对应的处理器
    MsgHandler getHandler(int msgid);
private:
    // 存储消息id和其对应的业务处理方法
    std::unordered_map<int,MsgHandler> _msgHandlerMap;
    // 单例设计模式
    ChatService();
};