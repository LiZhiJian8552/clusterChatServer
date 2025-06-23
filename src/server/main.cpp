#include"chatserver.h"
#include"chatservice.h"


#include<iostream>
#include<signal.h>


// 处理服务器ctrl+c结束后，重置user的状态信息,使用捕获信号
void resetHandler(int){
    ChatService::instance()->reset();
    exit(0);
}


int main(){
    // 捕获ctrl+c
    signal(SIGINT,resetHandler);
    
    EventLoop loop;
    InetAddress addr("127.0.0.1",6000);
    ChatServer server(&loop,addr,"ChatServer");
    server.start();
    loop.loop();

    return 0;
}