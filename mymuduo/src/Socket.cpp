#include"Socket.h"
#include"Logger.h"
#include"InetAddress.h"

#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<strings.h>
#include <netinet/tcp.h>

Socket::~Socket()
{
    ::close(sockfd_);
}

void Socket::bindAddress(const InetAddress &localaddr){
    int ret=::bind(sockfd_,(sockaddr*)localaddr.getsockAddr(),sizeof(sockaddr_in));
    if(ret<0){
        LOG_FATAL("bind sockfd:%d fail\n",sockfd_);
    }
}

void Socket::listen(){
    int ret=::listen(sockfd_,1024);
    if(ret<0){
        LOG_FATAL("listen sockfd:%d fail\n",sockfd_);
    }
}

int Socket::accept(InetAddress *peeraddr){
    struct sockaddr_in addr;
    socklen_t len=sizeof addr;
    bzero(&addr,sizeof addr);
    int connfd=::accept4(sockfd_,(struct sockaddr*)&addr,&len,SOCK_NONBLOCK|SOCK_CLOEXEC);
    if(connfd>=0){
        peeraddr->setSockAddr(addr);
    }
    return connfd;
}

// 关闭写端
void Socket::shutdownWrite(){
    if(::shutdown(sockfd_,SHUT_WR)<0){
        LOG_ERROR("shutdownWrite error");
    }
}

void Socket::setTcpNoDelay(bool on){
    int optval=on?1:0;
    ::setsockopt(sockfd_,IPPROTO_TCP,TCP_NODELAY,&optval,sizeof optval);
}

void Socket::setReuseAddr(bool on){
    int optval=on?1:0;
    ::setsockopt(sockfd_,SOL_SOCKET,SO_REUSEADDR,&optval,sizeof optval);
}

void Socket::setReusePort(bool on){
    int optval=on?1:0;
    ::setsockopt(sockfd_,SOL_SOCKET,SO_REUSEPORT,&optval,sizeof optval);
}

void Socket::setKeepAlive(bool on){
    int optval=on?1:0;
    ::setsockopt(sockfd_,SOL_SOCKET,SO_KEEPALIVE,&optval,sizeof optval);
}
