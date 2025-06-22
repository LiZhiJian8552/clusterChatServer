#pragma once
#include<user.h>

// user表的数据操作类
class userModel{
public:
    // 向User表中添加
    bool insert(User& user);
    // 通过id查询用户信息
    User query(int id);
    // 更新用户信息
    bool updateState(User user);
private:
};