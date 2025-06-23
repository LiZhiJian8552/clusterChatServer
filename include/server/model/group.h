#pragma once
#include<string>
#include<vector>

#include"groupuser.h"

// Group表的ORM类
class Group{
public:
    Group(int id=-1,std::string name="",std::string desc=""){
        this->id=id;
        this->name=name;
        this->desc=desc;
    }
    void setId(int id){this->id=id;}
    void setName(std::string name){this->name=name;}
    void setDesc(std::string desc){this->desc=desc;}
    

    int getId(){return this->id;}
    std::string getName(){return this->name;}
    std::string getDesc(){return this->desc;}
    std::vector<GroupUser>&getUsers(){return this->users;}
private:
    // 群id
    int id;
    // 群名称
    std::string name;
    // 群描述
    std::string desc;
    // 群成员
    std::vector<GroupUser> users; 
};