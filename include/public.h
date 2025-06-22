#pragma once

/*
    server和client的公共文件
*/

enum EnMsgType{
    DEFAULT_MSG=0,  //默认（即没有下面对应id时使用）
    LOGIN_MSG,      //登录消息id
    REG_MSG,        //注册消息id
    ONE_CHAT_MSG,   //聊天消息
    REG_MSG_ACK,    //注册响应消息
    LOGIN_MSG_ACK  //登录响应消息
    
};