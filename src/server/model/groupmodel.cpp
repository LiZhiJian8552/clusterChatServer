#include"groupmodel.h"
#include"db.h"
#include"utils.h"

// 创建群组
bool GroupModel::createGroup(Group& group){
    char sql[1024]={0};
    std::sprintf(sql,"insert into allgroup(groupname,groupdesc) values('%s','%s')",group.getName().c_str(),group.getDesc().c_str());
    MySQL mysql;
    if(mysql.connect()){
        mysql.query(sql);
        // mysql_insert_id获取该连接自动插入的id
        group.setId(mysql_insert_id(mysql.getConnection()));
        return true;
    }    
    return false;
}
// 加入群组
void GroupModel::addGroup(int userid,int groupid,std::string role){
    char sql[1024]={0};
    std::sprintf(sql,"insert into groupuser values('%d','%d','%s')",userid,groupid,role.c_str());
    MySQL mysql;
    if(mysql.connect()){
        mysql.query(sql);
    }    
}
// 查询用于所在群组信息
std::vector<Group> GroupModel::queryGroups(int userid){
    char sql[1024]={0};
    std::sprintf(sql,"select allgroup.id,allgroup.groupname,allgroup.groupdesc from allgroup inner join groupuser on allgroup.id=groupuser.groupid where groupuser.userid=%d",userid);
    MySQL mysql;
    std::vector<Group> groupsVec;
    if(mysql.connect()){
        MYSQL_RES * res=mysql.query(sql);
        if(res!=nullptr){
            MYSQL_ROW row;
            while((row=mysql_fetch_row(res))!=nullptr){
                Group group;
                group.setId(std::atoi(row[0]));
                group.setName(row[1]);
                group.setDesc(row[2]);
                groupsVec.push_back(group);
            }
            mysql_free_result(res);
        }

        // 查询群组的用户信息
        for(auto& group:groupsVec){
            sprintf(sql,"select user.id,user.name,user.state,groupuser.grouprole from groupuser inner join user on groupuser.userid = user.id where groupuser.groupid=%d",group.getId());
            MYSQL_RES * res=mysql.query(sql);
            if(res!=nullptr){
                MYSQL_ROW row;
                while((row=mysql_fetch_row(res))!=nullptr){
                    GroupUser user;
                    user.setId(std::atoi(row[0]));
                    user.setName(row[1]);
                    user.setState(row[2]);
                    user.setRole(row[3]);
                    group.getUsers().push_back(user);
                }
                mysql_free_result(res);
            }
        }
    }
    return groupsVec;  
}
// 根据指定的groupid查询群组用户id列表，除userid外，主要用于群聊天业务给群组其他成员群发消息
std::vector<int> GroupModel::queryGroupUsers(int userid,int groupid){
    char sql[1024]={0};
    std::sprintf(sql,"select userid from groupuser where groupid=%d and userid != %d",groupid,userid);
    MySQL mysql;
    std::vector<int> idVec;
    if(mysql.connect()){
        MYSQL_RES * res=mysql.query(sql);
        if(res!=nullptr){
            MYSQL_ROW row;
            while((row=mysql_fetch_row(res))!=nullptr){
                idVec.push_back(std::atoi(row[0]));
            }
            mysql_free_result(res);
        }
    } 
    return idVec;
}