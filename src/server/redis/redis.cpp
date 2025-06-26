#include"redis.h"
#include <iostream>
using namespace std;

// Redis类构造函数，初始化发布和订阅上下文指针为nullptr
Redis::Redis()
    :_publish_context(nullptr),_subscribe_context(nullptr){

}

// Redis类析构函数，释放发布和订阅上下文的资源
Redis::~Redis(){
    if(_publish_context!=nullptr){
        redisFree(_publish_context);
    }
    if(_subscribe_context!=nullptr){
        redisFree(_subscribe_context);
    }
}

// 连接Redis服务器，分别建立发布和订阅的连接
bool Redis::connect(){
    // 负责publish发布消息的上下文连接
    _publish_context=redisConnect("127.0.0.1",6379);
    if(nullptr==_publish_context){
        cerr<<"connect redis failed!"<<endl;
        return false;
    }

    // 负责subscribe订阅消息的上下文连接
    _subscribe_context=redisConnect("127.0.0.1",6379);
    if(nullptr==_subscribe_context){
        cerr<<"connect redis failed!"<<endl;
        return false;
    }

    // 在单独的线程中,监听通道上的事件,有消息给业务层进行上报
    thread t([&](){
        observer_channel_message();
    });
    t.detach(); // 分离线程，后台运行

    cout<<"connect redis-server sucess!"<<endl;
    return true;
}

// 发布消息到指定的Redis通道
bool Redis::publish(int channel, string messages){
    // 通过PUBLISH命令向指定通道发送消息
    redisReply* reply=(redisReply*)redisCommand(_publish_context,"PUBLISH %d %s",channel,messages.c_str());
    if(nullptr==reply){
        cerr<<"publish command failed!"<<endl;
        return false;
    }
    freeReplyObject(reply); // 释放命令回复对象
    return true;
}

// 订阅指定的Redis通道
bool Redis::subscribe(int channel){
    // SUBSCRIBE命令本身会造成线程阻塞等待通道里面发生消息，这里只做订阅通道，不接收通道消息
    // 通道消息的接收专门在observer_channel_message函数中的独立线程中进行
    // 只负责发送命令，不阻塞接收redis server响应消息，否则和notifyMsg线程抢占响应资源
    if(REDIS_ERR==redisAppendCommand(this->_subscribe_context,"SUBSCRIBE %d",channel)){
        cerr << "subscribe command failed!" << endl;
        return false;
    }
    // redisBufferWrite可以循环发送缓冲区,直到缓冲区数据发送完毕 (done被置为1)
    int done=0;
    while(!done){
        if(REDIS_ERR==redisBufferWrite(this->_subscribe_context,&done)){
            cerr << "subscribe command failed!" << endl;
            return false;
        }
    }
    return true;
}

// 取消订阅指定的Redis通道
bool Redis::unsubscribe(int channel){
    if(REDIS_ERR==redisAppendCommand(this->_subscribe_context,"UNSUBSCRIBE %d",channel)){
        cerr << "unsubscribe command failed!" << endl;
        return false;
    }
    int done=0;
    while(!done){
        if(REDIS_ERR==redisBufferWrite(this->_subscribe_context,&done)){
            cerr << "unsubscribe command failed!" << endl;
            return false;
        }
    }
    return true;
}

// 独立线程中执行，循环监听订阅通道的消息
void Redis::observer_channel_message(){
    redisReply* reply=nullptr;
    // 持续从订阅上下文中获取消息
    while(REDIS_OK==redisGetReply(this->_subscribe_context,(void**)&reply)){
        // 订阅收到的消息是一个带三元素的数组
        if(reply!=nullptr&& reply->element[2]!=nullptr&&reply->element[2]->str!=nullptr){
            // 业务层上报通道上发生的消息
            _notify_message_handler(atoi(reply->element[1]->str),reply->element[2]->str);
        }
        freeReplyObject(reply); // 释放命令回复对象
    }
    cerr << ">>>>>>>>>>>>> observer_channel_message quit <<<<<<<<<<<<<" << endl;
}

// 初始化消息通知回调对象
void Redis::init_notify_handler(function<void(int, string)> fn){
    this->_notify_message_handler=fn;
}
