#pragma once

#include<mysql/mysql.h>
#include<string>


// 数据库操作类
class MySQL {
    public:
        // 
        MySQL();
        ~MySQL();
        // 连接数据库
        bool connect();
        // 更新操作
        bool update(std::string sql);
        // 查询操作
        MYSQL_RES* query(std::string sql);
        // 获取连接
        MYSQL* getConnection();
    private:
        //表示一条与mysql的连接 
        MYSQL* _conn;
};