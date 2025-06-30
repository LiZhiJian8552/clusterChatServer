#include "offlinemessagemodel.h"


#include<vector>

// 存储用户的离线消息 
void OfflineMessageModel::insert(int userid,std::string msg){
    char sql[1024]={0};
    std::sprintf(sql,"insert into offlinemessage values(%d,'%s')",userid,msg.c_str());

    shared_ptr<Connection> conn=connPool->getConnection();
    conn->update(sql);
}
// 删除用户的离线消息
void OfflineMessageModel::remove(int userid){
    char sql[1024]={0};
    std::sprintf(sql,"delete from offlinemessage where userid=%d",userid);

    shared_ptr<Connection> conn=connPool->getConnection();
    
    conn->update(sql);
    
}
// 查询用户的离线消息
std::vector<std::string> OfflineMessageModel::query(int userid){
    char sql[1024]={0};
    std::sprintf(sql,"select message from offlinemessage where userid=%d",userid);
    std::vector<std::string> msg;
    
    shared_ptr<Connection> conn=connPool->getConnection();
    MYSQL_RES* res=conn->query(sql);
    if(res!=nullptr){
        // 将用户的所有离线消息放入vector
        MYSQL_ROW row;
        while((row=mysql_fetch_row(res))!=nullptr){
            msg.push_back(row[0]);
        }
        mysql_free_result(res);
        return msg;
    }
    return msg;
}