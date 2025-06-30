#pragma once
#include<user.h>

#include"connectionPool.h"

// user表的数据操作类
class UserModel{
public:
    UserModel():connPool(ConnectionPool::instance()){}
    // 向User表中添加
    bool insert(User& user);
    // 通过id查询用户信息
    User query(int id);
    // 更新用户信息
    bool updateState(User user);
    // 重置用户的登录状态
    void resetState();
private:
    ConnectionPool* connPool;
};