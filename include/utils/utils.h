#pragma once
#include"user.h"
#include<iostream>
using namespace std;

void show(User user){
    cout<<user.getId()<<"--"<<user.getName()<<"--"<<user.getPwd()<<"--"<<user.getState()<<endl;
}
