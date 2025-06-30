#pragma once

#include<mysql/mysql.h>
#include<string>


// 数据库操作类
class Connection {
    public:
        // 
        Connection();
        ~Connection();
        // 连接数据库
        bool connect(std::string ip,unsigned short port,std::string user,std::string password,std::string dbname);
        // 更新操作
        bool update(std::string sql);
        // 查询操作
        MYSQL_RES* query(std::string sql);
        
        // 获取连接
        MYSQL* getConnection();

        // 刷新连接的存活时间
        void refreshAliveTime(){_alivetime=clock();}
        // 获取连接的空闲时间
        clock_t getAliveTime(){return clock()-_alivetime;}
    private:
        //表示一条与mysql的连接 
        MYSQL* _conn;
        //记录进入空闲状态后(进入队列) 的起始存活时间
        clock_t _alivetime;
};