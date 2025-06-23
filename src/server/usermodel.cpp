#include "usermodel.h"
#include "db.h"
#include<iostream>
#include <muduo/base/Logging.h>
// 插入用户信息到数据库
bool UserModel::insert(User &user){
    // 1. 组装SQL插入语句，将用户的name、password、state插入user表
    char sql[1024]={0}; // 定义SQL语句缓冲区并初始化为0
    std::sprintf(sql,"insert into user(name,password,state) values('%s','%s','%s')",
        user.getName().c_str(),user.getPwd().c_str(),user.getState().c_str());
    // 2. 创建MySQL数据库操作对象
    MySQL mysql;
    // 3. 连接数据库，若连接成功则继续
    if(mysql.connect()){
        // 4. 执行SQL更新操作，插入数据
        if(mysql.update(sql)){
            // 5. 插入成功，记录日志，输出插入的SQL语句
            LOG_INFO<<"add user sucess => sql:"<<sql;
            // 6. 获取插入数据后自动生成的主键id
            // mysql_insert_id返回最近一次插入操作生成的自增ID
            user.setId(mysql_insert_id(mysql.getConnection()));
            // 7. 返回插入成功
            return true;
        }
    }
    // 8. 插入失败，返回false
    return false;
}

/**
 * @brief 查询指定ID的用户信息
 *
 * 该函数根据传入的用户ID，在user表中查找对应的用户信息，并返回一个User对象。
 * 
 * 实现流程如下：
 * 1. 构造SQL查询语句，查找user表中id等于给定id的记录。
 * 2. 创建MySQL数据库连接对象，并尝试连接数据库。
 * 3. 如果连接成功，调用MySQL::query方法执行SQL语句，返回MYSQL_RES*结果集指针。
 *    - MYSQL_RES* 是MySQL C API中用于保存查询结果的数据结构。
 * 4. 检查查询结果是否为空，若不为空则通过mysql_fetch_row获取一行数据（MYSQL_ROW）。
 *    - MYSQL_ROW是一个char*数组，包含当前行的所有字段值。
 * 5. 若成功获取到数据，则将各字段值赋给User对象的对应属性。
 * 6. 释放查询结果资源（mysql_free_result），防止内存泄漏。
 * 7. 返回填充好的User对象。
 * 8. 若查询失败或未找到用户，返回一个默认构造的User对象。
 *
 * @param id 需要查询的用户ID
 * @return User 查询到的用户对象，若未找到则返回空对象
 */
User UserModel::query(int id){
    // 1. 组装SQL查询语句，查找指定id的用户
    char sql[1024]={0}; // 定义SQL语句缓冲区
    std::sprintf(sql,"select * from user where id =%d",id); // 格式化SQL语句
    
    // 2. 创建MySQL数据库操作对象
    MySQL mysql;
    // 3. 连接数据库，若连接成功则继续
    if(mysql.connect()){
        // 4. 执行SQL查询操作，返回结果集指针
        MYSQL_RES * res = mysql.query(sql);
        // 5. 判断查询是否成功（结果集非空）
        if(res != nullptr){
            // 6. 获取结果集中的一行数据
            MYSQL_ROW row = mysql_fetch_row(res);
            if(row != nullptr){
                // 7. 查询到数据，填充User对象
                User user;
                user.setId(atoi(row[0]));    // row[0]为id，转换为int
                user.setName(row[1]);        // row[1]为name
                user.setPwd(row[2]);         // row[2]为password
                user.setState(row[3]);       // row[3]为state

                // 8. 释放结果集资源，防止内存泄漏
                mysql_free_result(res);
                // 9. 返回填充好的User对象
                return user;
            }
            mysql_free_result(res);
        }
    }
    // 10. 查询失败或未找到用户，返回默认User对象
    return User();
}

bool UserModel::updateState(User user){
    char sql[1024]={0};
    std::sprintf(sql,"update user set state='%s' where id =%d",user.getState().c_str(),user.getId());
    
    MySQL mysql;
    if(mysql.connect()){
        if(mysql.update(sql)){
            return true;
        }
    }
    return false;
}
