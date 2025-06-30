#pragma once

#include"group.h"
#include"connectionPool.h"
#include<string>
#include<vector>

class GroupModel{
public:
    GroupModel():connPool(ConnectionPool::instance()){}

    // 创建群组
    bool createGroup(Group& group);
    // 加入群组
    void addGroup(int userid,int groupid,std::string role);
    // 查询用于所在群组信息
    std::vector<Group> queryGroups(int userid);
    // 根据指定的groupid查询群组用户id列表，除userid外，主要用于群聊天业务给群组其他成员群发消息
    std::vector<int> queryGroupUsers(int userid,int groupid);
private:
    ConnectionPool* connPool;
};