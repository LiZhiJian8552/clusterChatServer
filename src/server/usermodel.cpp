#include "usermodel.h"
#include "db.h"
#include<iostream>
#include <muduo/base/Logging.h>

bool userModel::insert(User &user){
    // 1.组装sql语句
    char sql[1024]={0};
    std::sprintf(sql,"insert into User(name,password,state) values('%s','%s','%s')",
        user.getName().c_str(),user.getPwd().c_str(),user.getState().c_str());
    //2. 执行sql 
    MySQL mysql;
    if(mysql.connect()){
        if(mysql.update(sql)){
            LOG_INFO<<"add user sucess => sql:"<<sql;
            // 获取插入成功的用户数据生成的主键 id
            user.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}