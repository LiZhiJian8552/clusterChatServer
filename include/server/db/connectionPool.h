#pragma once

#include"db.h"

#include<iostream>
#include<string>
#include<queue>
#include<mutex>
#include<atomic>
#include<memory>
#include<thread>
#include<functional>
#include<condition_variable>

using namespace std;

class ConnectionPool{
public:
    //获取 连接池对象的实例
    static ConnectionPool* instance();

    // 从连接池中获取一个空闲连接
    shared_ptr<Connection> getConnection();

    // 关闭连接池
    void shutdownPool(){
        _isRunning=false;
    }

private:
    ConnectionPool();

    // 从配置文件中加载配置项
    bool loadConfigFile();

    // 运行在独立的线程中，专门负责生成新连接
    void produceConnectionTask();

    // 启动一个定时线程，扫描多余的空闲连接，专门用于销毁空闲时间过长的连接
    void scannerConnectionTask();

    // mysql的ip地址
    string _ip;
    // mysql的port
    unsigned short _port;
    // mysql的用户名
    string _username;
    // mysql的密码
    string _password;
    // 数据库名称
    string _dbname;

    // 初始连接数量
    int _initSize;
    // 最大连接数量
    int _maxSize;
    // 最大空闲时间
    int _maxIdleTime;
    // 超时时间
    int _connectionTimeOut;

    
    // 存储msql连接的队列
    queue<Connection*> _connectionQueue;
    // 保证_connectionQueue线程安全的锁
    mutex _queueMutex;
    // 记录所创建的conn的总数量
    atomic_int _connectionCnt;

    // 设置条件变量，用于连接生产线程和消费线程(唤醒消费线程)
    condition_variable cv;

    atomic_bool _isRunning;
};