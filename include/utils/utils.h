#pragma once
#include"user.h"
#include<iostream>
using namespace std;

inline void show(User user){
    cout<<user.getId()<<"--"<<user.getName()<<"--"<<user.getPwd()<<"--"<<user.getState()<<endl;
}

inline void printLn(){
    cout<<"----------------"<<endl;
}
