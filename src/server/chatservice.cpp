#include"chatservice.h"
#include<muduo/base/Logging.h>
#include<string>
#include"public.h"
using namespace muduo;


ChatService* ChatService::instance(){
    static ChatService service;
    return &service;
}
// 注册消息以及对应的回调
ChatService::ChatService(){
    _msgHandlerMap.insert({DEFAULT_MSG,std::bind(&ChatService::defaultHandler,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)});
    _msgHandlerMap.insert({LOGIN_MSG,std::bind(&ChatService::login,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)});
    _msgHandlerMap.insert({REG_MSG,std::bind(&ChatService::reg,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)});
}

MsgHandler ChatService::getHandler(int msgid){
    // 记录错误日志，msgid没有对应的事件处理回调
    auto it=_msgHandlerMap.find(msgid);
    // 未找到则返回默认处理器。空操作
    if(it==_msgHandlerMap.end()){
        return _msgHandlerMap[DEFAULT_MSG];
    }else{
        return _msgHandlerMap[msgid];
    }
}

// 处理登录业务
void ChatService::login(const TcpConnectionPtr&conn,json& js,Timestamp time){
    LOG_INFO<<"do login service!!!";
}
// 处理注册业务
void ChatService::reg(const TcpConnectionPtr&conn,json& js,Timestamp time){
    // 获取传入的数据
    string name=js["name"];
    string pwd=js["password"];
    
    User user;
    user.setName(name);
    user.setPwd(pwd);

    bool state=_userModel.insert(user);
    json response;
    // 插入（注册）成功
    if(state){
        response["msgid"]=REG_MSG_ACK;
        response["errno"]=0;
        response["id"]=user.getId();
    }else{  //插入（注册）失败
        response["msgid"]=REG_MSG_ACK;
        response["errno"]=1;
        response["msg"]="register error!";
        response["id"]=user.getId();
    }
    // 
    conn->send(response.dump());
}

void ChatService::defaultHandler(const TcpConnectionPtr &conn, json &js, Timestamp time){
    LOG_ERROR<<"msgid can not find handler!";
}
