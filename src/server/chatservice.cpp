#include<muduo/base/Logging.h>
#include<string>
#include<vector>


#include"chatservice.h"
#include"public.h"
#include"utils.h"

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
    _msgHandlerMap.insert({ONE_CHAT_MSG,std::bind(&ChatService::oneChat,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)});
    _msgHandlerMap.insert({ADD_FRIEND_MSG,std::bind(&ChatService::addFriend,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)});
    _msgHandlerMap.insert({CREATE_GROUP_MSG,std::bind(&ChatService::createGroup,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)});
    _msgHandlerMap.insert({ADD_GROUP_MSG,std::bind(&ChatService::addGroup,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)});
    _msgHandlerMap.insert({GROUP_CHAT_MSG,std::bind(&ChatService::groupChat,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)});
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

// 处理登录业务 id pwd {"msgid":1,"id":1,"password":"123456"} 
void ChatService::login(const TcpConnectionPtr&conn,json& js,Timestamp time){
    int id=js["id"];
    string pwd=js["password"];

    User user=_userModel.query(id);
    show(user);
    json response;
    // 查询到账户
    if(user.getId()!=-1&&user.getPwd()==pwd){
        // 该用户已经登陆
        if(user.getState()=="online"){
            response["msgid"]=LOGIN_MSG_ACK;
            response["message"]="该账号已经登录,不允许重复登录!";
            response["errno"]=2;
        }else{  //成功登录
            {
                // 登录成功之后记录用户连接信息
                lock_guard<mutex> lock(_connMutex);
                _userConnMap[user.getId()]=conn;
            }
            // 更新用户状态信息 state offline=>online
            user.setState("online");
            _userModel.updateState(user);
            // 构造返回信息
            response["msgid"]=LOGIN_MSG_ACK;
            response["errno"]=0;
            response["id"]=user.getId();
            response["name"]=user.getName();
            
            // 查询该用户是否有离线消息
            std::vector<std::string> msg=_offlineMessageModel.query(id);
            
            // 如果有离线消息
            if(!msg.empty()){
                response["offlinemsg"]=msg;
                // 删除离线消息
                _offlineMessageModel.remove(id);
            }

            // 查询该用户的好友信息并返回
            std::vector<User> userVec=_friendModel.query(id);
            if(!userVec.empty()){
                std::vector<std::string> vec;
                for(auto& user:userVec){
                    json js;
                    js["id"]=user.getId();
                    js["name"]=user.getName();
                    js["state"]=user.getState();
                    vec.push_back(js.dump());
                }
                response["friends"]=vec;
            }
        }
    }else{  //登录失败
        response["msgid"]=LOGIN_MSG_ACK;
        response["message"]="用户名或密码错误";
        response["errno"]=1;
    }
    conn->send(response.dump());
}
// 处理注册业务 {"msgid":2,"name":"wu","password":"123"} 
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
// 处理客户端异常关闭
void ChatService::clinetCloseException(const TcpConnectionPtr& conn){
    User user;
    {
        lock_guard<mutex> lock(_connMutex);
        for(auto it=_userConnMap.begin();it!=_userConnMap.end();it++){
            if(it->second==conn){
                // 从map表种删除用户的连接信息
                user.setId(it->first);
                _userConnMap.erase(it);
                break;
            }
        }
    }
    // 更新用户的状态信息
    if(user.getId()!=-1){
        user.setState("offline");
        _userModel.updateState(user);
    }
}


// 处理一对一聊天业务  {"msgid":3,"id":1,"from":"li","to":2,"message":"hello"} 
void ChatService::oneChat(const TcpConnectionPtr& conn ,json & js,Timestamp time){
    // 获取 接受者id
    int toid=js["to"].get<int>();
    
    //表示用户是否在线
    {
        //查询接收者conn
        auto it=_userConnMap.find(toid);
        // 
        if(it!=_userConnMap.end()){ 
            //toid在线，转发消息
            it->second->send(js.dump());
            
            return ;
        }
    }
    //不在线，存储离线消息
    _offlineMessageModel.insert(toid,js.dump());
}
// 群组聊天 {"msgid":3,"id":1,"from":"li","to":2,"message":"hello"} 
void ChatService::groupChat(const TcpConnectionPtr& conn ,json & js,Timestamp time){
    int userid=js["id"].get<int>();
    int groupid=js["groupid"].get<int>();

    // 查询该群组中其他人的id
    vector<int> useridVec=_groupModel.queryGroupUsers(userid,groupid);
    
    lock_guard<mutex> lock(_connMutex);
    // 像群组中每个人发送消息
    for(int id:useridVec){
        
        auto it=_userConnMap.find(id);
        // 在线直接转发
        if(it!=_userConnMap.end()){
            it->second->send(js.dump());
        }else{
            // 离线存储群消息
            _offlineMessageModel.insert(id,js.dump());
        }
    }
}


// 添加好友 msgid id friendid {"msgid":4,"id":1,"friendid":2}
void ChatService::addFriend(const TcpConnectionPtr& conn ,json & js,Timestamp time){
    int userid=js["id"].get<int>();
    int friendid=js["friendid"].get<int>();
    
    // 存储好友信息
    _friendModel.insert(userid,friendid);
}


// 创建群组业务 {"msgid":5,"id":1,"groupname":"","groupdesc":""}
void ChatService::createGroup(const TcpConnectionPtr& conn ,json & js,Timestamp time){
    int userid=js["id"].get<int>();
    string name=js["groupname"];
    string desc=js["groupdesc"]; 

    // 存储新创建的群组信息
    Group group(-1,name,desc);
    if(_groupModel.createGroup(group)){
        // 存储群组创建人信息
        _groupModel.addGroup(userid,group.getId(),"creator");
    }

}
// 加入群组业务 {"msgid":6,"id":2,"groupid":}
void ChatService::addGroup(const TcpConnectionPtr& conn ,json & js,Timestamp time){
    int userid=js["id"].get<int>();
    int groupid=js["groupid"].get<int>();
    _groupModel.addGroup(userid,groupid,"normal");
}



void ChatService::defaultHandler(const TcpConnectionPtr &conn, json &js, Timestamp time){
    LOG_ERROR<<"msgid can not find handler!";
}


// 重置用户状态
void ChatService::reset(){
    // 更新所有用户的状态，将online->offline
    _userModel.resetState();
}


