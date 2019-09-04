#pragma once
#include "db-classes.hpp"
#include "pqxx/pqxx"
#include <cstdio>
#include <cstdlib>
#include <feverrpc/threadmanager.hpp>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>


// https://github.com/Courier-Developer/feverrpc/blob/master/lock.cpp
// 关注ThreadManager的可以调用的方法，比如查询是否在线
extern ThreadManager threadManager;
int create_package(int uid, int groupid, std::string package_name);

/**
 * @brief 设置数据库的时区
 *
 * @return int
 */
int set_timezone();

/**
 * @brief 根据用户名匹配密码
 *
 * @param username 用户名
 * @param password 密码
 * @return int 0-密码错误 1-密码正确 -1-数据库连接错误
 */
int login(std::string username, std::string password, std::string ip);

/**
 * @brief 判断用户名是否重复
 *
 * @param username 用户名
 * @return int 正数：存在并返回此用户id 0未重复 -1数据库连接失败
 */
int is_username_exists(std::string username);

/**
 * @brief 注册，保存用户注册时提供的基本信息
 *
 * @param username 用户名
 * @param password 密码
 * @param nickname 昵称
 * @param ismale 性别
 * @return int -2 用户名已存在 -1 数据库连接错误 正数 用户的id
 */
int register_account(std::string username, std::string password, std::string nickname, bool ismale);

/**
 * @brief 正常下线，修改lastlogintime字段
 *
 * @param uid 用户id
 * @return true 正常下线
 * @return false 数据库连接错误
 */
bool logout(int uid);

/**
 * @brief 获取用户的所有信息 by id
 *
 * @param uid 用户id
 * @return UserInfo
 */
UserInfo get_info_by_uid(int uid);

/**
 * @brief 获取用户的所有信息 by username
 *
 * @param username
 * @return UserInfo
 */
UserInfo get_info_by_username(std::string username);

/**
 * @brief 更新用户信息
 *
 * @param uid 用户id
 * @param ui 新的用户信息
 * @return true 更新成功
 * @return false 数据库连接错误
 */
bool update_info(int uid, UserInfo ui);

/**
 * @brief 获得一个用户的所有好友信息
 *
 * @param uid 用户id
 * @return std::vector<Friend> 包含所有好友信息的vector
 */
std::vector<Friend> list_friends(int uid);

/// \brief 申请好友
///
/// 需要同时向ThreadManager发送请求
/// 在数据库中存储的时候需要存储单向的
bool request_friend(int uid, int friend_id);

/// \brief 同意好友请求
///
/// 在数据库中存储需要存储双向的
bool make_friend(int uid, int friend_id);

/// \brief 删除好友 拒绝也可以用这个
bool delete_friend(int uid, int friend_id);

/**
 * @brief 判断某用户是否已经有这个名字的package，不存在则创建package
 *
 * @param uid
 * @param package_name
 * @return int -1数据库连接失败 否则返回package的id
 */
int find_package(int uid, std::string package_name);

/// \brief 修改好友所在分组
bool change_package(int owner_id, int friend_id, std::string package_name);

///删除群组
bool delete_package (int ownerId, int packageId);

/// \brief 根据名字搜索群聊
///没改！！！别乱用
std::vector<ChatGroup> search_group(std::string group_name);

/// \brief 加入群聊
bool join_chatGroup(int uid, int groupid);

/// \brief 获得某人加入的所有群聊
std::vector<ChatGroup> list_chat_groups(int uid);

/// \brief 创建群聊
/// \return ChatGroup 创建后的群聊信息，主要是包含id
ChatGroup create_chat_group(int uid, std::string nickname);

/// \brief 获得某个群聊的所有用户信息
std::vector<Friend> get_group_mumber(int group_id);

/// \brief 离开一个群聊 退群
///
/// 如果是群主，直接解散群？
/// A：目前暂不设置群成员中特殊身份
/// TODO：所有人都退群的时候怎么办
bool leave_group(int uid, int group_id);

/// \brief 插入一条消息，创建、修改时间均为now
Message insert_message(int senderId, int receiverId, MsgType type, bool isToGroup,std::string content);

/// \brief 获得上次离线以后所有关于自己的聊天记录
///
/// 包括人对人和群组的
std::vector<Message> get_unread_messages(int uid);

//TODO
std::vector<Message> get_all_message(int uid);

/// \brief 从该路径中读取文件
/// \param file_name 文件名，可以用相对路径
/// \return std::vector<char> 存放数据
// std::vector<char> read_file(string file_name) {
//     std::ifstream file(file_name, std::ios::binary);
//     // Stop eating new lines in binary mode!!!
//     file.unsetf(std::ios::skipws);

//     std::streampos fileSize;

//     file.seekg(0, std::ios::end);
//     fileSize = file.tellg();
//     file.seekg(0, std::ios::beg);

//     std::vector<char> vec;
//     vec.reserve(fileSize);

//     vec.insert(vec.begin(), std::istream_iterator<char>(file),
//                std::istream_iterator<char>());
//     file.close();
//     return vec;
// }

/// \brief 向该路径存放文件
/// \param file_name 文件名，与 read_file 中路径一致
/// \param data 数据
/// \return Void
// void save_file(string file_name, std::vector<char> &data) {
//     std::ofstream file(file_name, std::ios::out | std::ios::binary);
//     file.write((const char *)&data[0], data.size());
//     file.close();
// }
/// \brief 获得文件信息内容
/// TODO incomplete definition
/// 根据path取出存储在文件系统里的文件
// Message download_data(int uid, std::string path);

/// \brief 上传文件
/// TODO incomplete definition
/// 根据path和data将文件存储
// bool upload_data(int uid, std::string path);

UserInfo get_my_info();

std::vector<Friend> get_all_friends_info();

std::vector<ChatGroup> get_all_chatGroups_info();
std::vector<package> get_all_my_package();
/**
 * @brief 新建好友分组
 *
 * @param uid 好友id
 * @param groupid 若为1 则表示数据库自动生成
 * 为-1或0则为注册账户时生成的自动黑名单or默认分组
 * @param package_name 分组的名字
 * @return int 1成功 -1 连接数据库失败
 */
int create_package(int uid, int groupid, std::string package_name);

// TODO：更改群组名

bool change_name_of_package(int uid, int package_id, std::string package_name);

/**
 * @brief 创建一个群聊并且加入用户
 *
 * @param chatGroupName
 * @param members
 * @return int
 */
int create_chatGroup_and_invite_friends(std::string chatGroupName,std::vector<int> members);

/**
 * @brief 发送消息 内部调用insert_message
 * 
 * @param senderid 
 * @param receiverid 
 * @param type 
 * @param istoGroup 
 * @param content 
 * @return int 
 */
int send_message (int senderid, int receiverid, MsgType type, bool istoGroup, std::string content);

/**
 * @brief Get the chatGroupWithMembers object
 * 
 * @param chatGroupId 
 * @return chatGroup_with_members 
 */
chatGroup_with_members get_chatGroupWithMembers (int chatGroupId);


std::vector<char> read_file_(string file_name);
int save_file_(string file_name, std::vector<char> data);
std::vector<chatGroup_with_members> get_all_chatGroups();