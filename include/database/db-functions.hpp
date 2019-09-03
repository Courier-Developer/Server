#pragma once
#include "db-classes.hpp"
#include "pqxx/pqxx"
#include <feverrpc/threadmanager.hpp>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <vector>
#include <iterator>
#include <fstream>

#define DBLOGINFO                                                              \
    "dbname=postgres user=postgres password=example "                          \
    "hostaddr=59.110.233.235 port=5432"
#define CONNECTION_ERROR "connection erroe"
#define NOT_FOUND_ERROR "not found"
#define SUCCESS_INFO "success"

// https://github.com/Courier-Developer/feverrpc/blob/master/lock.cpp
// 关注ThreadManager的可以调用的方法，比如查询是否在线
extern ThreadManager threadManager;

void output(pqxx::result &R) {
    for (pqxx::const_result_iterator i = R.begin(); i != R.end(); ++i) {
        for (int j = 0; j < i.size(); j++) {
            std::cout << i[j].name() << ":  " << i[j].c_str() << std::endl;
        }
    }
}

/**
 * @brief 设置数据库的时区
 *
 * @return int
 */
int set_timezone() {
    pqxx::connection C(DBLOGINFO);
    if (C.is_open()) {
        pqxx::work W(C);
        std::string sql = "set timezone='PRC';";
        pqxx::result R = W.exec(sql);
        W.commit();
        return 1;
    } else {
        return 0;
    }
}

/**
 * @brief 根据用户名匹配密码
 *
 * @param username 用户名
 * @param password 密码
 * @return int 0-密码错误 1-密码正确 -1-数据库连接错误
 */
int login(std::string username, std::string password, std::string ip) {
    // TODO
    pqxx::connection C(DBLOGINFO);
    if (C.is_open()) {
        pqxx::work W(C);
        std::string sql =
            "select id,username,password from userinfo where username='" +
            username + "';";
        pqxx::result R = W.exec(sql);

        if (strcmp(R[0][2].c_str(), password.c_str()) == 0){
            std::string sql_setUp = "update userinfo set ip = '" + ip + "', lastlogintime = now()::timestamp where id = " + R[0][0].c_str() + ";";
            W.exec(sql_setUp);
            W.commit();
            return 1;
        }
        else
            return 0;
    } else {
        return -1;
    }
}

/**
 * @brief 判断用户名是否重复
 *
 * @param username 用户名
 * @return int 1重复 0未重复 -1数据库连接失败
 */
int is_username_exists(std::string username) {
    pqxx::connection C(DBLOGINFO);
    if (C.is_open()) {
        pqxx::work W(C);
        std::string sql =
            "select id from userinfo where username='" + username + "';";
        pqxx::result R = W.exec(sql);
        if (R.size() != 0) {
            return 1;
        } else
            return 0;
    } else {
        return -1;
    }
}

/**
 * @brief 注册，保存用户注册时提供的基本信息
 *
 * @param username 用户名
 * @param password 密码
 * @param nickname 昵称
 * @param ismale 性别
 * @return int -2 用户名已存在 -1 数据库连接错误 正数 用户的id
 */
int register_account(std::string username, std::string password,
                     std::string nickname, bool ismale) {
    if (is_username_exists(username) == 1) {
        return -2;
    }
    pqxx::connection C(DBLOGINFO);
    if (C.is_open()) {
        pqxx::work W(C);
        std::string sql = "insert into postgres.public.userinfo (username, "
                          "password, nickname) values ('" +
                          username + "', '" + password + "', '" + nickname +
                          "');";
        pqxx::result R_insert = W.exec(sql);
        W.commit();
        sql = "select id from userinfo where username='" + username + "';";
        pqxx::work W_select(C);
        pqxx::result R_select = W_select.exec(sql);
        std::string id_str = R_select[0][0].c_str();
        int i = std::stoi(id_str);
        return i;
    } else {
        return -1;
    }
}

/**
 * @brief 正常下线
 *
 * @param uid 用户id
 * @return true 正常下线
 * @return false 数据库连接错误
 */
bool logout(int uid) {
    uid = threadManager.get_uid();
    printf("[db-funcs][logout]%d\n",uid);
    pqxx::connection C(DBLOGINFO);
    if (C.is_open()) {
        pqxx::work W(C);
        std::string sql =
            "update userinfo set lastlogintime = now() where id = " +
            std::to_string(uid) + ";";
        pqxx::result R = W.exec(sql);
        W.commit();
        return 1;
    } else
        return 0;
}

/**
 * @brief 获取用户的所有信息
 *
 * @param uid 用户id
 * @return Response<UserInfo>
 */
Response<UserInfo> get_info(int uid) {
    uid = threadManager.get_uid();
    pqxx::connection C(DBLOGINFO);
    if (C.is_open()) {
        pqxx::work W(C);
        std::string sql = "select "
                          "id,username,createdtime,lastlogintime,birthday,"
                          "ismale,nickname,ip from userinfo where id=" +
                          std::to_string(uid) + ";";
        pqxx::result R = W.exec(sql);
        UserInfo info;
        info.id = atoi(R[0][0].c_str());
        info.username = R[0][1].c_str();
        info.createdTime = R[0][2].c_str();
        info.lastLoginTime = R[0][3].c_str();
        info.birthday = R[0][4].c_str();
        info.isMale = strcmp(R[0][5].c_str(), "true") ? 1 : 0;
        info.nickname = R[0][6].c_str();
        info.ip = R[0][7].c_str();
        Response<UserInfo> resp(1, "UserInfo", info);
        return resp;
    } else {
        Response<UserInfo> resp(0, CONNECTION_ERROR);
        return resp;
    }
}

/**
 * @brief 更新用户信息
 *
 * @param uid 用户id
 * @param ui 新的用户信息
 * @return true 更新成功
 * @return false 数据库连接错误
 */
bool update_info(int uid, UserInfo ui) {
    uid = threadManager.get_uid();
    pqxx::connection C(DBLOGINFO);
    if (C.is_open()) {
        pqxx::work W(C);
        std::string isMale = ui.isMale ? "true" : "false";
        std::string sql = "update userinfo set username = '" + ui.username +
                          "', password = '" + ui.password +
                          "', lastlogintime = '" + ui.lastLoginTime +
                          "', birthday = '" + ui.birthday + "', signature = '" +
                          ui.signature + "', ismale = " + isMale +
                          ", nickname = '" + ui.nickname + "' where id = '" +
                          std::to_string(uid) + "';";
        pqxx::result R = W.exec(sql);
        return 1;
    } else {
        return 0;
    }
}

/**
 * @brief 获得一个用户的所有好友信息
 *
 * @param uid 用户id
 * @return Response<std::vector<Friend>> 包含所有好友信息的vector
 */
Response<std::vector<Friend>> list_friends(int uid) {
    uid = threadManager.get_uid();
    pqxx::connection C(DBLOGINFO);
    if (C.is_open()) {
        pqxx::work W(C);
        std::string sql =
            "select u.id, friend.groupname, u.username, u.createdtime, "
            "u.lastlogintime, u.birthday, u.ismale, u.ip, u.nickname, "
            "friend.mute from friend inner join userinfo u on friend.owner = "
            "u.id where friend.owner = " +
            std::to_string(uid) + " and friend.isagreed = true;";
        pqxx::result R = W.exec(sql);

        std::vector<Friend> all_friend;
        for (pqxx::const_result_iterator row = R.begin(); row != R.end();
             ++row) {
            Friend tmp;
            tmp.uid = row[0].as<int>();
            tmp.group = row[1].c_str();
            tmp.username = row[2].c_str();
            tmp.createdTime = row[3].c_str();
            tmp.lastLoginTime = row[4].c_str();
            tmp.birthday = row[5].c_str();
            tmp.isMale = strcmp(row[6].c_str(), "true") == 0 ? 1 : 0;
            tmp.ip = row[7].c_str();
            tmp.nickname = row[8].c_str();
            tmp.isMute = strcmp(row[9].c_str(), "true") == 0 ? 1 : 0;
            all_friend.push_back(tmp);
        }
        Response<std::vector<Friend>> resp(1, "successful", all_friend);
        return resp;
    } else {
        Response<std::vector<Friend>> resp(0, CONNECTION_ERROR);
        return resp;
    }
}

/**
 * @brief 获得某个好友的信息
 *
 * @param uid 用户id
 * @param friend_id 朋友id
 * @return Response<Friend> 返回Friend的信息
 */
Response<Friend> get_friend(int uid, int friend_id) {
    uid = threadManager.get_uid();
    pqxx::connection C(DBLOGINFO);
    if (C.is_open()) {
        pqxx::work W(C);
        std::string sql =
            "select friend, groupname from friend where owner = " +
            std::to_string(uid) +
            " and friend.friend = " + std::to_string(friend_id) + ";";
        pqxx::result R = W.exec(sql);
        if (R.size() == 0) {
            Response<Friend> resp(0, NOT_FOUND_ERROR);
            W.commit();
            return resp;
        } else {
            Friend tmp;
            tmp.group = R[0][1].c_str();
            std::cout << "groupname:  " << tmp.group << std::endl;
            // pqxx::work W_usrinfo(C);
            std::string id_str = R[0][0].c_str();
            std::string sql =
                "select id, username, createdtime, lastlogintime, birthday, "
                "ismale, ip, nickname from userinfo where id = " +
                id_str + ";";
            pqxx::result R_usrinfo = W.exec(sql);
            tmp.uid = R[0][0].as<int>();
            tmp.username = R[0][1].c_str();
            tmp.createdTime = R[0][2].c_str();
            tmp.lastLoginTime = R[0][3].c_str();
            tmp.birthday = R[0][4].c_str();
            tmp.isMale = (strcmp(R[0][5].c_str(), "true")) == 0 ? 1 : 0;
            tmp.ip = R[0][6].c_str();
            tmp.nickname = R[0][7].c_str();
            Response<Friend> resp(1, SUCCESS_INFO, tmp);
            W.commit();
            return resp;
        }
    } else {
        Response<Friend> resp(0, CONNECTION_ERROR);
        return resp;
    }
}

/// \brief 申请好友
///
/// 需要同时向ThreadManager发送请求
/// 在数据库中存储的时候需要存储单向的
bool request_friend(int uid, int friend_id) {
    uid = threadManager.get_uid();
    pqxx::connection C(DBLOGINFO);
    if (C.is_open()) {
        pqxx::work W(C);
        std::string sql = "insert into friend (owner, friend, groupname, mute, "
                          "isagreed) values (" +
                          std::to_string(uid) + ", " +
                          std::to_string(friend_id) +
                          ", null, false, false);\ninsert into friend (owner, "
                          "friend, groupname, mute, isagreed) values (" +
                          std::to_string(friend_id) + ", " +
                          std::to_string(uid) + ", null, false, false);";
        W.exec(sql);
        W.commit();
        return 1;
    } else {
        return 0;
    }
}

/// \brief 同意好友请求
///
/// 在数据库中存储需要存储双向的
bool make_friend(int uid, int friend_id) {
    uid = threadManager.get_uid();
    pqxx::connection C(DBLOGINFO);
    if (C.is_open()) {
        pqxx::work W(C);
        std::string sql =
            "update friend set isagreed = true where owner = " +
            std::to_string(uid) +
            " and friend.friend = " + std::to_string(friend_id) +
            ";\nupdate friend set isagreed = true where owner = " +
            std::to_string(friend_id) +
            " and friend.friend = " + std::to_string(uid) + ";";
        W.exec(sql);
        W.commit();
        return 1;
    } else {
        return 0;
    }
}

/// \brief 删除好友 拒绝也可以用这个
bool delete_friend(int uid, int friend_id) {
    pqxx::connection C(DBLOGINFO);
    if (C.is_open()) {
        pqxx::work W(C);
        std::string sql =
            "delete from friend where owner = " + std::to_string(uid) +
            " and friend.friend = " + std::to_string(friend_id) +
            ";\ndelete from friend where friend.friend = " +
            std::to_string(friend_id) + " and owner = " + std::to_string(uid) +
            ";";
        W.exec(sql);
        W.commit();
        return 1;
    } else {
        return 0;
    }
}

/// \brief 修改好友所在群组
bool change_friendGroup(int owner_id, int friend_id, std::string group_name) {
    pqxx::connection C(DBLOGINFO);
    if (C.is_open()) {
        pqxx::work W(C);
        std::string sql = "update friend set groupname = '" + group_name +
                          "' where owner = " + std::to_string(owner_id) +
                          " and friend.friend = " + std::to_string(friend_id) +
                          ";";
        W.exec(sql);
        W.commit();
        return 1;
    } else
        return 0;
}

/// \brief 根据名字搜索群聊
Response<std::vector<ChatGroup>> search_group(std::string group_name) {
    pqxx::connection C(DBLOGINFO);
    if (C.is_open()) {
        pqxx::work W(C);
        std::string sql_search =
            "select id from chatgroup where name = '" + group_name + "';";
        pqxx::result R_search = W.exec(sql_search);
        if (R_search.size() == 0) {
            Response<std::vector<ChatGroup>> resp(0, NOT_FOUND_ERROR);
            return resp;
        } else {
            std::vector<ChatGroup> chatgroups;
            for (pqxx::result::const_iterator row = R_search.begin();
                 row != R_search.end(); ++row) {
                ChatGroup tmp;
                tmp.name = group_name;
                tmp.id = row[0].as<int>();
                chatgroups.push_back(tmp);
            }
            Response<std::vector<ChatGroup>> resp(1, SUCCESS_INFO, chatgroups);
            return resp;
        }

    } else {
        Response<std::vector<ChatGroup>> resp(0, CONNECTION_ERROR);
        return resp;
    }
}

/// \brief 加入群聊
bool join_chatGroup(int uid, int groupid) {
    pqxx::connection C(DBLOGINFO);
    if (C.is_open()) {
        pqxx::work W(C);
        std::string sql_search =
            "select name from chatgroup where id = " + std::to_string(groupid) +
            ";";
        pqxx::result R_search = W.exec(sql_search);
        if (R_search.size() == 0) {
            return 0;
        } else {
            std::string sql_addgroup =
                "insert into user_in_group (userid, groupid, mute) values (" +
                std::to_string(uid) + ", " + std::to_string(groupid) +
                ", false);";
            W.exec(sql_addgroup);
            W.commit();
            return 1;
        }
    } else {
        return 0;
    }
}

/// \brief 获得某人加入的所有群聊
Response<std::vector<ChatGroup>> list_chat_groups(int uid) {
    pqxx::connection C(DBLOGINFO);
    if (C.is_open()) {
        pqxx::work W(C);
        std::string sql_listGroup =
            "select id, name from chatgroup where id in (select groupid from "
            "user_in_group where userid = " +
            std::to_string(uid) + ");";
        pqxx::result R_listGroup = W.exec(sql_listGroup);
        std::vector<ChatGroup> chatgroups;
        for (pqxx::result::const_iterator row = R_listGroup.begin();
             row != R_listGroup.end(); ++row) {
            ChatGroup tmp;
            tmp.id = row[0].as<int>();
            tmp.name = row[1].c_str();
            chatgroups.push_back(tmp);
        }
        Response<std::vector<ChatGroup>> resp(1, SUCCESS_INFO, chatgroups);
        return resp;
    } else {
        Response<std::vector<ChatGroup>> resp(0, CONNECTION_ERROR);
        return resp;
    }
}

/// \brief 创建群聊
/// \return Response<ChatGroup> 创建后的群聊信息，主要是包含id
Response<ChatGroup> create_chat_group(int uid, std::string nickname) {
    pqxx::connection C(DBLOGINFO);
    if (C.is_open()) {
        pqxx::work W(C);

        std::string sql_MaxGroupId = "select max(id) from chatgroup;";
        pqxx::result R_MaxGroupId = W.exec(sql_MaxGroupId);
        int GroupId = R_MaxGroupId[0][0].as<int>() + 1;

        std::string sql = "insert into chatgroup (id, name) values (" +
                          std::to_string(GroupId) + ", '" + nickname + "');";
        W.exec(sql);
        W.commit();

        join_chatGroup(uid, GroupId);
        ChatGroup tmp;
        tmp.id = GroupId;
        tmp.name = nickname;
        Response<ChatGroup> resp(1, SUCCESS_INFO, tmp);
        return resp;
    } else {
        Response<ChatGroup> resp(0, CONNECTION_ERROR);
        return resp;
    }
}

/// \brief 获得某个群聊的所有用户信息
Response<std::vector<Friend>> get_group_mumber(int group_id) {
    pqxx::connection C(DBLOGINFO);
    if (C.is_open()) {
        pqxx::work W(C);
        std::string sql_listUsers =
            "select userid from user_in_group where groupid = " +
            std::to_string(group_id) + ";";
        pqxx::result R_listUsers = W.exec(sql_listUsers);
        std::string sql_findName = "select name from chatgroup where id = " +
                                   std::to_string(group_id) + ";";
        pqxx::result R_chatGroupName = W.exec(sql_findName);
        std::string groupName = R_chatGroupName[0][0].c_str();
        std::vector<Friend> friends_in_chatGroup;
        for (pqxx::result::const_iterator row = R_listUsers.begin();
             row != R_listUsers.end(); ++row) {
            Response<UserInfo> tmp(1, "temp");
            tmp = get_info(row[0].as<int>());

            UserInfo info_as_userInfo = tmp.data;
            Friend info_as_friendInfo;
            info_as_friendInfo.uid = info_as_userInfo.id;
            info_as_friendInfo.group = groupName;
            info_as_friendInfo.username = info_as_userInfo.username;
            info_as_friendInfo.createdTime = info_as_userInfo.createdTime;
            info_as_friendInfo.lastLoginTime = info_as_userInfo.lastLoginTime;
            info_as_friendInfo.birthday = info_as_userInfo.birthday;
            info_as_friendInfo.isMale = info_as_userInfo.isMale;
            info_as_friendInfo.nickname = info_as_userInfo.nickname;
            info_as_friendInfo.isMute = false; //用Friend存储群聊中的好友信息时这一项无意义。
            info_as_friendInfo.ip = info_as_userInfo.ip;
            friends_in_chatGroup.push_back(info_as_friendInfo);
        }
        Response<std::vector<Friend>> resp(1, SUCCESS_INFO,
                                           friends_in_chatGroup);
        return resp;
    } else {
        Response<std::vector<Friend>> resp(0, CONNECTION_ERROR);
        return resp;
    }
}

/// \brief 离开一个群聊
///
/// 如果是群主，直接解散群？
/// A：目前暂不设置群成员中特殊身份
/// TODO：所有人都退群的时候怎么办
bool leave_group(int uid, int group_id) {
    pqxx::connection C(DBLOGINFO);
    if (C.is_open()) {
        pqxx::work W_leave(C);
        std::string sql_leave =
            "delete from user_in_group where userid = " + std::to_string(uid) +
            " and groupid = " + std::to_string(group_id) + ";";
        W_leave.exec(sql_leave);
        W_leave.commit();
        return 1;
    } else {
        return 0;
    }
}

/// \brief 插入一条消息，创建、修改时间均为now
bool insert_message(int senderId, int receiverId, MsgType type, bool isToGroup,
                    std::string content) {
    pqxx::connection C(DBLOGINFO);
    if (C.is_open()) {
        pqxx::work W(C);

        std::string sql_findMaxId = "select max(id) from message;";
        int id;
        pqxx::result R_find = W.exec(sql_findMaxId);
        if (strcmp(R_find[0][0].c_str(), "") == 0) {
            id = 1;
        } else {
            id = R_find[0][0].as<int>() + 1;
        }

        std::string message_type_str =
            type == MsgType::MSGTYPE_TEXT ? "text"
                                 : type == MsgType::MSGTYPE_FILE ? "file" : "image";
        std::string is_to_group_str = isToGroup ? "true" : "false";
        std::string sql_istMsg =
            "insert into message (id, sender, receiver, type, createdtime, "
            "editedtime, istogroup, content) values (" +
            std::to_string(id) + ", " + std::to_string(senderId) + ", " +
            std::to_string(receiverId) + ", '" + message_type_str +
            "', now()::timestamp, now()::timestamp, " + is_to_group_str +
            ", '" + content + "');";
        W.exec(sql_istMsg);
        W.commit();
        return 1;
    } else {
        return 0;
    }
}

/// \brief 获得上次离线以后所有关于自己的聊天记录
///
/// 包括人对人和群组的
Response<std::vector<Message>> get_unread_messages(int uid) {
    pqxx::connection C(DBLOGINFO);
    if (C.is_open()) {
        pqxx::work W_getMsg(C);
        std::string sql_getMsg =
            "select id, sender, receiver, type, createdtime, istogroup, "
            "content from message where receiver = " +
            std::to_string(uid) +
            " and message.createdtime > (select lastlogintime from userinfo "
            "where id = " +
            std::to_string(uid) + ");";
        pqxx::result R = W_getMsg.exec(sql_getMsg);
        std::vector<Message> messages;
        for (pqxx::result::const_iterator row = R.begin(); row != R.end();
             ++row) {
            Message tmp;
            tmp.id = row[0].as<int>();
            tmp.sender = row[1].as<int>();
            tmp.receiver = row[2].as<int>();
            tmp.type = strcmp(row[3].c_str(), "text") == 0
                           ? MsgType::MSGTYPE_TEXT
                           : strcmp(row[3].c_str(), "FILE") == 0
                                 ? MsgType::MSGTYPE_FILE
                                 : MsgType::MSGTYPE_IMAGE;
            tmp.createdTime = row[4].c_str();
            tmp.editedTime = tmp.createdTime;
            tmp.isToGroup = strcmp(row[5].c_str(), "true") == 0 ? 1 : 0;
            tmp.content = row[6].c_str();
            messages.push_back(tmp);
        }
        Response<std::vector<Message>> resp(1, SUCCESS_INFO, messages);
        return resp;
    } else {
        Response<std::vector<Message>> resp(0, CONNECTION_ERROR);
        return resp;
    }
}

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
Response<Message> download_data(int uid, std::string path) {}

/// \brief 上传文件
/// TODO incomplete definition
/// 根据path和data将文件存储
bool upload_data(int uid, std::string path) {}
