#include "json.hpp"
#include "group.h"
#include "user.h"
#include "public.h"

#include<iostream>
#include<thread>
#include<string>
#include<vector>
#include<chrono>
#include<ctime>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unordered_map>
#include<functional>
#include<semaphore.h>
#include<atomic>



using namespace std;
using json=nlohmann::json;




// 记录当前系统登录的用户信息
User g_currentUser;
// 记录当前登录用户的好友列表信息
vector<User> g_currentUserFriendList;
// 记录当前登录用户的群列表信息
vector<Group> g_currentUserGroupList;
// 控制主菜单页面程序
bool isMainMenuRunning=false;
// 用于读写线程之间的通信
sem_t rwsem;
// 记录登录状态
atomic_bool g_isLoginSucess{false};


// 显示当前登录成功用户的基本信息
void showCurrentUserData();
// 接受线程
void readTaskHandler(int clientfd);
// 获取系统时间（聊天信息需要添加信息时间） 
string getCurrentTime();
// 主聊天页面程序
void mainMenu(int);


// help command handler
void help(int fd=0,string str="");
// chat command handler
void chat(int,string);
// addfriend command handler
void addfriend(int,string);
// creategroup command handler
void creategroup(int,string);
// addgroup command handler
void addgroup(int,string);
// groupchat command handler
void groupchat(int,string);
// loginout command handler
void loginout(int,string);

// 处理注册的响应逻辑
void doRegResponse(json &responsejs);
// 处理登录的响应逻辑
void doLoginResponse(json &responsejs);


unordered_map<string,string> commandMap={
    {"help","显示所有支持的命令,格式help"},
    {"chat","一对一聊天,格式chat:friendid:message"},
    {"addfriend","添加好友,格式addfriend:friendid"},
    {"creategroup","创建群组,格式creategroup:groupname:groupdesc"},
    {"addgroup","加入群组,格式addgroup:groupid"},
    {"groupchat","群聊,格式groupchat:groupid:message"},
    {"loginout","注销,格式loginout"}
};
unordered_map<string,function<void(int,string)>> commandHandlerMap={
    {"help",help},
    {"chat",chat},
    {"addfriend",addfriend},
    {"creategroup",creategroup},
    {"addgroup",addgroup},
    {"groupchat",groupchat},
    {"loginout",loginout}
};



// 聊天客户端程序实现，main线程用于发送线程，子线程用于接受线程
int main(int argc,char** argv){
    if(argc<3){
        cerr<<"command invalid! example: ./ChatClient 127.0.0.1 6000"<<endl;
        exit(-1);
    }
    // 解析通过命令行参数传递的ip和port
    char *ip=argv[1];
    uint16_t port=atoi(argv[2]);

    // 创建client端的socket
    int clientfd=socket(AF_INET,SOCK_STREAM,0);
    if(clientfd==-1){
        cerr << "socket create error" << endl;
        exit(-1);
    }

    // 填写client需要连接的server信息ip+port
    sockaddr_in server;
    memset(&server,0,sizeof(sockaddr_in));

    server.sin_family=AF_INET;
    server.sin_port=htons(port);
    server.sin_addr.s_addr=inet_addr(ip);

    // client与server进行连接
    if(-1==connect(clientfd,(sockaddr*)&server,sizeof(sockaddr_in))){
        cerr << "connect server error" << endl;
        close(clientfd);
        exit(-1);
    }

    // 初始化读写线程通信的信号量
    sem_init(&rwsem,0,0);

    // 连接服务器成功，启动接受子线程
    std::thread readTask(readTaskHandler,clientfd);
    readTask.detach();

    for(;;){
        // 显示首页面菜单 登录、注册、退出
        cout << "========================" << endl;
        cout << "1. login" << endl;
        cout << "2. register" << endl;
        cout << "3. quit" << endl;
        cout << "========================" << endl;
        cout << "choice:";
        int choice=0;
        cin>>choice;
        cin.get();  //读掉缓冲区残留的回车

        switch(choice){
            //登录业务
            case 1:{
                int id=0;
                char pwd[50]={0};
                cout << "user id:";
                cin >> id;
                cin.get(); // 读掉缓冲区残留的回车
                cout << "user password:";
                cin.getline(pwd,50);

                json js;
                js["msgid"]=LOGIN_MSG;
                js["id"]=id;
                js["password"]=pwd;
                string request=js.dump();

                g_isLoginSucess=false;
                int len=send(clientfd,request.c_str(),strlen(request.c_str())+1,0);
                if(len==-1){
                    cerr<<"send login msg error: "<<request<<endl;
                }
                // 等待信号量，由子线程处理完登录的响应消息后通知这里
                sem_wait(&rwsem);
                if(g_isLoginSucess){
                    // 进入聊天菜田主页面
                    isMainMenuRunning=true;
                    mainMenu(clientfd);
                }
            }
            break;
            //注册业务
            case 2:{
                char name[50]={0};
                char pwd[50]={0};
                cout << "username:";
                // cin >> && scanf遇到空格会结束
                cin.getline(name, 50);
                cout << "userpassword:";
                cin.getline(pwd, 50);

                json js;
                js["msgid"]=REG_MSG;
                js["name"]=name;
                js["password"]=pwd;

                string request=js.dump();
                int len=send(clientfd,request.c_str(),strlen(request.c_str())+1,0);
                if(len==-1){
                    cerr<<"send rsg msg error: "<<request<<endl;
                }
                sem_wait(&rwsem);
            }
            break;
            case 3:{
                close(clientfd);
                sem_destroy(&rwsem);
                exit(0);
            }
            default:
                cerr<<"invlid input!"<<endl;
                break;
        }
    }

    return 0;
}

void mainMenu(int clientfd){
    help();

    char buffer[1024]={0};
    while(isMainMenuRunning){
        cin.getline(buffer,1024);
        string commandbuf(buffer);
        string command;
        int idx=commandbuf.find(":");
        if(-1==idx){
            command=commandbuf;
        }else{
            command=commandbuf.substr(0,idx);
        }
        auto it=commandHandlerMap.find(command);
        if(it==commandHandlerMap.end()){
            cerr<<"invalid input command!"<<endl;
            continue;
        }
        // 找到了对应的命令，调用相应的命令处理回调，mainMenu对修改封闭，添加新功能不需要修改该函数
        it->second(clientfd,commandbuf.substr(idx+1,commandbuf.size()-idx));
    }
}


void help(int,string){
    cout<<"show command list >>>"<<endl;
    // p--->pair<string,string>
    for(auto& p:commandMap){
        cout<<p.first<<" : "<<p.second<<endl;
    }
    cout<<endl;
}
// addfriend：friendid
void addfriend(int clientfd,string str){
    int friendid=atoi(str.c_str());
    json js;
    js["msgid"]=ADD_FRIEND_MSG;
    js["id"]=g_currentUser.getId();
    js["friendid"]=friendid;
    string buffer=js.dump();

    int len=send(clientfd,buffer.c_str(),strlen(buffer.c_str())+1,0);
    if(-1==len){
        cerr<<"send addfriend msg error -> "<<buffer<<endl;
    }
}
//chat:friendid:message
void chat(int clientfd,string str){
    
    int idx=str.find(":");
    if(-1==idx){
        cerr<<"chat command invalid!"<<endl;
        return ;
    }

    int friendid=atoi(str.substr(0,idx).c_str());
    string message=str.substr(idx+1,str.size()-idx);

    json js;
    js["msgid"]=ONE_CHAT_MSG;
    js["id"]=g_currentUser.getId();
    js["name"]=g_currentUser.getName();
    js["toid"]=friendid;
    js["msg"]=message;
    js["time"]=getCurrentTime();
    string buffer=js.dump();

    int len=send(clientfd,buffer.c_str(),strlen(buffer.c_str())+1,0);
    if(-1==len){
        cerr<<"send chat msg error -> "<<buffer<<endl;
    }
}
// creategroup:groupname:groupdesc
void creategroup(int clientfd,string str){
    int idx=str.find(":");
    if(-1==idx){
        cerr<<"creategroup command invalid!"<<endl;
        return;
    }
    string groupname=str.substr(0,idx);
    string groupdesc=str.substr(idx+1,str.size()-idx);

    json js;
    js["msgid"] = CREATE_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupname"] = groupname;
    js["groupdesc"] = groupdesc;
    string buffer=js.dump();

    int len=send(clientfd,buffer.c_str(),strlen(buffer.c_str())+1,0);
    if(-1==len){
        cerr << "send creategroup msg error -> " << buffer << endl;
    }
}
// addgroup:groupid
void addgroup(int clientfd,string str){
    int groupid=atoi(str.c_str());
    json js;
    js["msgid"] = ADD_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupid"] = groupid;
    string buffer=js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len){
        cerr << "send addgroup msg error -> " << buffer << endl;
    }
}
// groupchat:groupid:message
void groupchat(int clientfd,string str){
    int idx=str.find(":");
    if(-1==idx){
        cerr<<"groupchat command invalid!"<<endl;
        return;
    }

    int groupid=atoi(str.substr(0,idx).c_str());
    string message=str.substr(idx+1,str.size()-idx);

    json js;
    js["msgid"] = GROUP_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["groupid"] = groupid;
    js["msg"] = message;
    js["time"] = getCurrentTime();
    string buffer=js.dump();
    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len){
        cerr << "send groupchat msg error -> " << buffer << endl;
    }
}
// loginout
void loginout(int clientfd,string str){
    json js;
    js["msgid"] = LOGINOUT_MSG;
    js["id"] = g_currentUser.getId();
    string buffer = js.dump();
    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str()) + 1, 0);
    if (-1 == len){
        cerr << "send loginout msg error -> " << buffer << endl;
    }else
    {
        isMainMenuRunning = false;
    }  
}
// 处理注册的响应逻辑
void doRegResponse(json &responsejs){
    // 注册失败
    if(0!=responsejs["errno"].get<int>()){
        cerr << "name is already exist, register error!" << endl;
    }else{  //注册成功
        cout << "name register success, userid is " << responsejs["id"]
                << ", do not forget it!" << endl;
    }
}
// 处理登录的响应逻辑
void doLoginResponse(json &responsejs){
    // 登录失败
    if(0!=responsejs["errno"].get<int>()){
        cerr << responsejs["errmessage"] << endl;
        g_isLoginSucess=false;
    }else{  //登陆成功
        // 记录当前用户的id和name
        g_currentUser.setId(responsejs["id"].get<int>());
        g_currentUser.setName(responsejs["name"]);

        // 记录当前用户的好友列表信息
        if(responsejs.contains("friends")){
            // 初始化
            g_currentUserFriendList.clear();

            vector<string> vec=responsejs["friends"];
            for(string& str:vec){
                json js=json::parse(str);
                User user;
                user.setId(js["id"].get<int>());
                user.setName(js["name"]);
                user.setState(js["state"]);
                g_currentUserFriendList.push_back(user);
            }
        }

        // 记录当前用户的群组列表信息
        if(responsejs.contains("groups")){
            g_currentUserGroupList.clear();

            vector<string> vec1=responsejs["groups"];
            for(string& groupstr:vec1){
                json grpjs=json::parse(groupstr);
                Group group;
                group.setId(grpjs["id"].get<int>());
                group.setName(grpjs["groupname"]);
                group.setDesc(grpjs["groupdesc"]);

                vector<string> vec2=grpjs["users"];
                for(string& userstr:vec2){
                    GroupUser user;
                    json js=json::parse(userstr);
                    user.setId(js["id"].get<int>());
                    user.setName(js["name"]);
                    user.setState(js["state"]);
                    user.setRole(js["role"]);
                    group.getUsers().push_back(user);
                }

                g_currentUserGroupList.push_back(group);
            }
        }

        // 显示登录用户的基本信息
        showCurrentUserData();

        // 显示当前用户的离线信息，个人聊天信息或者群聊信息
        if(responsejs.contains("offlinemsg")){
            vector<string> vec=responsejs["offlinemsg"];
            for(string& str:vec){
                json js=json::parse(str);
                // time + [id] + name + " said: " + xxx
                if(ONE_CHAT_MSG==js["msgid"].get<int>()){
                    cout << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>()
                            << " said: " << js["msg"].get<string>() << endl;
                }else{
                    cout << "群消息[" << js["groupid"] << "]:" << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>()
                            << " said: " << js["msg"].get<string>() << endl;
                }
            }
        }
        g_isLoginSucess=true;
    }
}

void readTaskHandler(int clientfd){
    for(;;){
        char buffer[1024]={0};
        int len=recv(clientfd,buffer,1024,0);
        if(-1==len||0==len){
            close(clientfd);
            exit(-1);
        }

        // 接受ChatServer转发的数据，反序列化 生成 json数据对象
        json js=json::parse(buffer);
        int msgtype=js["msgid"].get<int>();
        if(ONE_CHAT_MSG==msgtype){
            cout<<js["time"].get<string>()<<" ["<<js["id"]<<"] "<<js["name"].get<string>()<<" said: "<<js["msg"].get<string>()<<endl;
            continue;
        }
        if(GROUP_CHAT_MSG==msgtype){
            cout << "群消息[" << js["groupid"] << "]:" << js["time"].get<string>() << " [" << js["id"] << "]" << js["name"].get<string>()
                 << " said: " << js["msg"].get<string>() << endl;
            continue;
        }
        // 登录
        if(LOGIN_MSG_ACK==msgtype){
            doLoginResponse(js);
            sem_post(&rwsem);   //通知主线程，登录结果处理完成
            continue;
        }
        if(REG_MSG_ACK==msgtype){
            doRegResponse(js);
            sem_post(&rwsem);   //通知主线程，注册结果处理完成
            continue;
        }
    }
}

string getCurrentTime(){
    auto tt=std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm* ptm=localtime(&tt);
    char date[60]={0};
    sprintf(date,"%d-%02d-%02d %02d:%02d:%02d",(int)ptm->tm_yday+1900,(int)ptm->tm_mon+1,(int)ptm->tm_mday,(int)ptm->tm_hour,(int)ptm->tm_min,(int)ptm->tm_sec);
    return std::string(date);
}

void showCurrentUserData(){
    cout << "======================login user======================" << endl;
    cout << "current login user => id:" << g_currentUser.getId() << " name:" << g_currentUser.getName() << endl;

    cout << "----------------------friend list---------------------" << endl;
    if(!g_currentUserFriendList.empty()){
        for(User& user:g_currentUserFriendList){
            cout<<user.getId()<<" "<<user.getName()<<" "<<user.getState()<<endl;
        }
    }

    cout << "----------------------group list----------------------" << endl;
    if(!g_currentUserGroupList.empty()){
        for(Group& group:g_currentUserGroupList){
            cout<<group.getId()<<" "<<group.getName()<<" "<<group.getDesc()<<endl;
            for(GroupUser& user:group.getUsers()){
                cout<<user.getId()<<" "<<user.getName()<<" "<<user.getState()<<" "<<user.getRole()<<endl;
            }
        }
    }
    cout << "======================================================" << endl;
}
