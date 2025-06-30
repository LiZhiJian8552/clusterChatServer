# 集群聊天服务器

## 项目简介
这是一个基于C++的集群聊天服务器项目，使用了muduo网络库、MySQL数据库以及Redis消息中间件。该项目支持用户注册、登录、单聊、群聊、添加好友、创建和加入群组等功能。

## 主要功能
- 用户登录与注册
- 好友添加与管理
- 群组创建与加入
- 点对点聊天与群聊天
- 用户离线消息存储与读取
- 用户登录状态管理

## 最终实现效果

![screenshots1](https://wu-zhouzhou.oss-cn-qingdao.aliyuncs.com/img_for_typora/screenshots1.gif)

## 数据库结构
本项目使用MySQL数据库，主要数据表包括：
### user 表
存储用户信息

| 字段名      | 类型         | 描述       |
|-----------|--------------|------------|
| id        | INT          | 用户ID (主键) |
| name      | VARCHAR(50)  | 用户名      |
| password  | VARCHAR(50)  | 密码       |
| state     | VARCHAR(10)  | 状态 (默认'offline') |

### friend 表
存储好友关系信息

| 字段名      | 类型        | 描述        |
|-----------|-------------|-------------|
| userid    | INT         | 用户ID (外键) |
| friendid  | INT         | 好友ID (外键) |

### offlineMessage 表
存储离线消息

| 字段名      | 类型          | 描述         |
|-----------|---------------|--------------|
| userid    | INT           | 用户ID (外键) |
| message   | VARCHAR(1024) | 消息内容      |

### allgroup 表
存储群组信息

| 字段名      | 类型         | 描述        |
|-----------|--------------|------------|
| id        | INT          | 群组ID (主键) |
| groupname | VARCHAR(50)  | 群组名称     |
| groupdesc | VARCHAR(200) | 群组描述     |

### groupuser 表
存储群组用户关系信息

| 字段名      | 类型         | 描述         |
|-----------|--------------|--------------|
| groupid   | INT          | 群组ID (外键) |
| userid    | INT          | 用户ID (外键) |
| role      | VARCHAR(10)  | 角色          |

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