#include "offlinemessagemodel.h"
#include"db.h"

#include<vector>

// 存储用户的离线消息 
void OfflineMessageModel::insert(int userid,std::string msg){
    char sql[1024]={0};
    std::sprintf(sql,"insert into offlinemessage values(%d,'%s')",userid,msg.c_str());

    MySQL mysql;
    if(mysql.connect()){
        mysql.update(sql);
    }
}
// 删除用户的离线消息
void OfflineMessageModel::remove(int userid){
    char sql[1024]={0};
    std::sprintf(sql,"delete from offlinemessage where userid=%d",userid);

    MySQL mysql;
    if(mysql.connect()){
        mysql.update(sql);
    }
}
// 查询用户的离线消息
std::vector<std::string> OfflineMessageModel::query(int userid){
    char sql[1024]={0};
    std::sprintf(sql,"select message from offlinemessage where userid=%d",userid);
    std::vector<std::string> msg;
    MySQL mysql;
    if(mysql.connect()){
        MYSQL_RES* res=mysql.query(sql);
        if(res!=nullptr){
            // 将用户的所有离线消息放入vector
            MYSQL_ROW row;
            while((row=mysql_fetch_row(res))!=nullptr){
                msg.push_back(row[0]);
            }
            mysql_free_result(res);
            return msg;
        }
    }
    return msg;
}