#pragma once

#include<muduo/net/TcpConnection.h>
#include<unordered_map>
#include<functional>
#include<mutex>

#include"usermodel.h"
#include"json.hpp"
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
    // 一对一聊天业务
    void oneChat(const TcpConnectionPtr& conn ,json & js,Timestamp time);
    // 处理客户端异常退出，修改用户状态
    void clinetCloseException(const TcpConnectionPtr& conn);
    // 获取消息对应的处理器
    MsgHandler getHandler(int msgid);
private:
    // 存储消息id和其对应的业务处理方法
    std::unordered_map<int,MsgHandler> _msgHandlerMap;
    // 单例设计模式
    ChatService();

    // 定义互斥锁，用于保证_userConnMap的线程安全
    std::mutex _connMutex;
    // 存储在线用户的通信连接,int->用户的id,用户到有消息发给用户时，可以推送给用户
    std::unordered_map<int,TcpConnectionPtr> _userConnMap;

    // 数据操作类对象
    userModel _userModel;
};