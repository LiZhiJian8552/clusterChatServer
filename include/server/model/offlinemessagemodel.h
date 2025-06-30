#pragma once

#include<string>
#include<vector>

#include"connectionPool.h"

// 提供离线消息表的操作接口方法
class OfflineMessageModel{
public:
    OfflineMessageModel():connPool(ConnectionPool::instance()){}
    // 存储用户的离线消息 
    void insert(int userid,std::string msg);
    // 删除用户的离线消息
    void remove(int userid);
    // 查询用户的离线消息
    std::vector<std::string> query(int userid);
private:
    ConnectionPool* connPool;
};