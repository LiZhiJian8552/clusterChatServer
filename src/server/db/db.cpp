#include<db.h>
#include <muduo/base/Logging.h>

// 数据库配置信息
static std::string server="127.0.0.1";
static std::string user="root";
static std::string password="123456";
static std::string dbname="chat";


MySQL::MySQL(){
    // 为该连接申请空间
    _conn = mysql_init(nullptr);
}
// 
MySQL::~MySQL() {
    // 释放该空间
    if (_conn != nullptr) {
        mysql_close(_conn);
    }
}
// 连接数据库
bool MySQL::connect(){
    MYSQL* p=mysql_real_connect(_conn,server.c_str(),user.c_str(),password.c_str(),dbname.c_str(),3306,nullptr,0);
    if(p!=nullptr){
        // 设置编码，支持中文
        mysql_query(_conn,"set names utf8mb4");
        LOG_INFO<<"connect mysql sucess!";
    }else{
        LOG_INFO<<"connect mysql fail!";
    }
    return p;
}
// 更新操作
bool MySQL::update(std::string sql){
    // mysql_query 执行sql操作,成功执行返回0，出错返回非0值
    if(mysql_query(_conn,sql.c_str())){
        LOG_INFO<<__FILE__<<":"<<__LINE__<<":"<<sql<<"更新失败!";
        return false;
    }
    return true;
}
// 查询操作
/*
    MYSQL_RES 是一个表示 MySQL 查询结果的类型定义，
    基于结构体 MYSQL_RES。
    它包含了查询结果的行数、字段信息、数据指针、当前行的列长度、连接句柄、方法指针以及其他与结果集处理相关的元数据和状态信息。
*/
MYSQL_RES* MySQL::query(std::string sql){
    if(mysql_query(_conn,sql.c_str())){
        LOG_INFO<<__FILE__<<":"<<__LINE__<<":"<<sql<<"查询失败!";
        return nullptr;
    }
    return mysql_use_result(_conn);
}

MYSQL *MySQL::getConnection(){
    return _conn;
}
