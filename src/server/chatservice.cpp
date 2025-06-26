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
    // 用户基本业务管理相关事件处理回调注册
    _msgHandlerMap.insert({DEFAULT_MSG,std::bind(&ChatService::defaultHandler,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)});
    _msgHandlerMap.insert({LOGIN_MSG,std::bind(&ChatService::login,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)});
    _msgHandlerMap.insert({REG_MSG,std::bind(&ChatService::reg,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG,std::bind(&ChatService::oneChat,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)});
    _msgHandlerMap.insert({ADD_FRIEND_MSG,std::bind(&ChatService::addFriend,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)});
    
    // 群组业务管理相关事件处理回调注册
    _msgHandlerMap.insert({CREATE_GROUP_MSG,std::bind(&ChatService::createGroup,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)}); 
    _msgHandlerMap.insert({ADD_GROUP_MSG,std::bind(&ChatService::addGroup,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)});
    _msgHandlerMap.insert({GROUP_CHAT_MSG,std::bind(&ChatService::groupChat,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)});
    _msgHandlerMap.insert({LOGINOUT_MSG,std::bind(&ChatService::loginout,this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)});

    // 连接redis服务器
    if(_redis.connect()){
        // 设置上报消息的回调
        _redis.init_notify_handler(std::bind(&ChatService::handleRedisSubscribeMessage,this,std::placeholders::_1,std::placeholders::_2));
    }

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
            response["errmessage"]="this account is using, input another!";
            response["errno"]=2;
        }else{  //成功登录
            {
                // 登录成功之后记录用户连接信息
                lock_guard<mutex> lock(_connMutex);
                _userConnMap[user.getId()]=conn;
            }

            // 登录成功后,向redis订阅通道
            _redis.subscribe(user.getId());

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

            // 查询用户的群组信息
            vector<Group> groupuserVec=_groupModel.queryGroups(id);
            if(!groupuserVec.empty()){
                // group:[{groupid:[xxx, xxx, xxx, xxx]}]
                vector<string>groupV;
                for(Group& group:groupuserVec){
                    json grpjson;
                    grpjson["id"]=group.getId();
                    grpjson["groupname"]=group.getName();
                    grpjson["groupdesc"]=group.getDesc();
                    vector<string> userV;
                    for(GroupUser& user:group.getUsers()){
                        json js;
                        js["id"]=user.getId();
                        js["name"]=user.getName();
                        js["state"]=user.getState();
                        js["role"]=user.getRole();
                        userV.push_back(js.dump());
                    }
                    grpjson["users"]=userV;
                    groupV.push_back(grpjson.dump());
                }
                response["groups"]=groupV;
            }
        }
    }else{  //登录失败
        response["msgid"]=LOGIN_MSG_ACK;
        response["errmessage"]="id or password is invalid";
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



// 处理一对一聊天业务  {"msgid":3,"id":1,"from":"li","to":2,"message":"hello"} 
void ChatService::oneChat(const TcpConnectionPtr& conn ,json & js,Timestamp time){
    // 获取 接受者id
    int toid=js["toid"].get<int>();
    
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

    // 查询toid是否在线
    User user=_userModel.query(toid);
    if(user.getState()=="online"){
        _redis.publish(toid,js.dump());
        return ;
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
            // 查询toid是否在线
            User user=_userModel.query(id);
            if(user.getState()=="online"){
                _redis.publish(id,js.dump());
            }else{
                // 离线存储群消息
                _offlineMessageModel.insert(id,js.dump());
            }
        }
    }
}


// 添加好友 msgid id friendid {"msgid":4,"id":1,"friendid":2}
void ChatService::addFriend(const TcpConnectionPtr& conn ,json & js,Timestamp time){
    int userid=js["id"].get<int>();
    int friendid=js["friendid"].get<int>();
    
    // 存储双向好友信息
    _friendModel.insert(userid,friendid);
    _friendModel.insert(friendid,userid);
}


// 创建群组业务 {"msgid":5,"id":1,"groupname":"","groupdesc":""}
void ChatService::createGroup(const TcpConnectionPtr& conn ,json & js,Timestamp time){
    int userid=js["id"].get<int>();
    string name=js["groupname"];
    string desc=js["groupdesc"]; 

    // 存储新创建的群组信息
    Group group(-1,name,desc);
    bool isCreate=_groupModel.createGroup(group);
    if(isCreate){
        // 存储群组创建人信息
        _groupModel.addGroup(group.getId(),userid,"creator");
    }

}
// 加入群组业务 {"msgid":6,"id":2,"groupid":}
void ChatService::addGroup(const TcpConnectionPtr& conn ,json & js,Timestamp time){
    int userid=js["id"].get<int>();
    int groupid=js["groupid"].get<int>();
    _groupModel.addGroup(groupid,userid,"normal");
}
// 注销登录
void ChatService::loginout(const TcpConnectionPtr &conn, json &js, Timestamp time){
    int userid=js["id"].get<int>();
    // 将该id从ConnMap中去除
    {
        lock_guard<mutex> lock(_connMutex);
        auto it=_userConnMap.find(userid);
        if(it!=_userConnMap.end()){
            _userConnMap.erase(it);
        }
    }
    // 向redis取消订阅通道
    _redis.unsubscribe(userid);

    // 更新用户的状态信息
    User user(userid,"","","offline");
    _userModel.updateState(user);
}

// 处理客户端异常关闭
void ChatService::clinetCloseException(const TcpConnectionPtr& conn){
    User user;
    {
        lock_guard<mutex> lock(_connMutex);
        for(auto it=_userConnMap.begin();it!=_userConnMap.end();it++){
            if(it->second==conn){
                // 从map表中删除用户的连接信息
                user.setId(it->first);
                _userConnMap.erase(it);
                break;
            }
        }
    }
    // 向redis取消订阅通道
    _redis.unsubscribe(user.getId());

    // 更新用户的状态信息
    if(user.getId()!=-1){
        user.setState("offline");
        _userModel.updateState(user);
    }
}


void ChatService::defaultHandler(const TcpConnectionPtr &conn, json &js, Timestamp time){
    LOG_ERROR<<"msgid can not find handler!";
}


// 重置用户状态
void ChatService::reset(){
    // 更新所有用户的状态，将online->offline
    _userModel.resetState();
}


void ChatService::handleRedisSubscribeMessage(int userid,string msg){
     
    lock_guard<mutex> lock(_connMutex);
    auto it=_userConnMap.find(userid);
    if(it!=_userConnMap.end()){
        it->second->send(msg);
        return;
    }
    // 存储该用户的离线消息
    _offlineMessageModel.insert(userid,msg);
}

