#pragma once
#include"user.h"

// 群成员类
class GroupUser:public User{
public:
    void setRole(std::string role){this->role=role;}
    std::string getRole(){return this->role;}
private:
    std::string role;
};
