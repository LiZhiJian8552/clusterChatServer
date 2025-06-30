#include"friendmodel.h"


// 添加好友关系
void FriendModel::insert(int userid,int friendid){
    char sql[1024]={0};
    std::sprintf(sql,"insert into friend values(%d,%d)",userid,friendid);
    
    shared_ptr<Connection> conn=connPool->getConnection();
    conn->update(sql);
}
// 获取用户的好友列表
std::vector<User> FriendModel::query(int userid){
    char sql[1024]={0};
    std::sprintf(sql,"select user.id,user.name,user.state from user inner join friend on user.id =friend.friendid  where friend.userid=%d",userid);
    std::vector<User> vec;

    shared_ptr<Connection> conn=connPool->getConnection();

    
    MYSQL_RES *res=conn->query(sql);
    if(res!=nullptr){
        MYSQL_ROW row;
        while((row=mysql_fetch_row(res))!=nullptr){
            User user;
            user.setId(std::atoi(row[0]));
            user.setName(row[1]);
            user.setState(row[2]);
            vec.push_back(user);
        }
        mysql_free_result(res);
        return vec;
    }
    
    return vec;
}