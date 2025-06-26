

# 集群聊天服务器

## 项目简介
这是一个基于C++的集群聊天服务器项目，实现了即时通讯系统的主要功能，包括用户登录/注册、一对一聊天、群组创建与管理、群聊天以及离线消息处理等。

## 主要功能
- 用户登录与注册
- 好友添加与管理
- 群组创建与加入
- 点对点聊天与群聊天
- 用户离线消息存储与读取
- 用户登录状态管理

## 数据库结构
本项目使用MySQL数据库，主要数据表包括：
- **user**: 存储用户信息
- **friend**: 存储用户好友关系
- **offlineMessage**: 存储用户离线消息
- **allgroup**: 存储群组基本信息
- **groupuser**: 存储群组成员信息

## 核心模块
- **ChatServer**: 服务器主模块，处理客户端连接与消息接收
- **ChatService**: 业务逻辑处理模块，包括登录、注册、聊天等消息处理
- **MySQL**: 数据库连接与操作模块
- **UserModel, FriendModel, GroupModel, OfflineMessageModel**: 数据模型模块，分别对应不同数据表的操作
- **Redis**: 消息订阅与发布模块，用于服务器间通信

## 使用方法
1. 编译项目
2. 启动数据库并导入`chat.sql`
3. 启动服务器
4. 客户端登录并使用命令进行操作

## 客户端命令
- `help`：显示帮助信息
- `chat`：一对一聊天
- `addfriend`：添加好友
- `creategroup`：创建群组
- `addgroup`：加入群组
- `groupchat`：群组聊天
- `loginout`：注销登录

## 依赖项
- [Muduo网络库](https://github.com/chenshuo/muduo)
- [nlohmann/json](https://github.com/nlohmann/json)
- MySQL数据库
- Redis数据库

## 许可证
本项目采用MIT License，请查看具体文件以获取更多信息。