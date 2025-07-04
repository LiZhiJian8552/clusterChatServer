#pragma once
#include"connectionPool.h"
#include<vector>
#include<user.h>

// 维护好友信息的操作接口方法
class FriendModel{
public:
    FriendModel():connPool(ConnectionPool::instance()){}
    // 添加好友关系
    void insert(int userid,int friendid);
    // 获取用户的好友列表
    std::vector<User> query(int userid);
private:
    ConnectionPool* connPool;
};