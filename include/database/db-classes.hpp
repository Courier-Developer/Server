#pragma once
#include <iostream>
#include <msgpack.hpp>
#include <string>

template <typename T> class Response {
    bool success;
    std::string msg;

  public:
    T data;
    Response(bool success, std::string msg, T data)
        : success(success), msg(msg), data(data){};
    Response(bool success, std::string msg) : success(success), msg(msg){};
    MSGPACK_DEFINE(success, msg);
    friend std::ostream &operator<<(std::ostream &os, Response &res) {
        std::cout << "[Response]"
                  << "success:" << res.success << ",msg:" << res.msg
                  << ",data:" << res.data << std::endl;
        return os;
    }
};

struct UserInfo {
    int id;
    std::string username;
    std::string createdTime;
    std::string lastLoginTime;
    // TODO avatar
    std::string birthday;
    bool isMale;
    std::string nickname;
    std::string password;
    std::string signature;
    std::string ip;
    MSGPACK_DEFINE(id, username, createdTime, lastLoginTime, birthday, isMale,
                   nickname, password, signature, ip);
    friend std::ostream &operator<<(std::ostream &os, UserInfo &d) {
        std::cout << "[UserInfo]"
                  << "id: " << d.id << ",username: " << d.username
                  << ", createdTime: " << d.createdTime
                  << ", lastLoginTime: " << d.lastLoginTime
                  << ", birthday: " << d.birthday << ", isMale: " << d.isMale
                  << ", nickName: " << d.nickname
                  << ", password: " << d.password
                  << ", signature: " << d.signature << ", ip: " << d.ip
                  << std::endl;
        return os;
    }
};

/// 好友和群聊好友都用这个？
struct Friend {
    int uid;
    int packageid; ///< 只存好友分组的信息！群聊好友中不用这个字段
    std::string username;
    std::string createdTime;
    std::string lastLoginTime;
    // TODO avatar
    std::string birthday;
    bool isMale;
    std::string ip;
    std::string nickname;
    bool isMute;
    std::string signature;
    bool isOnline;
    MSGPACK_DEFINE(uid, packageid, username, createdTime, lastLoginTime, birthday,
                   isMale, ip, nickname, isMute);
    friend std::ostream &operator<<(std::ostream &os, Friend &d) {
        std::cout << "[UserInfo]"
                  << "id: " << d.uid << ",username: " << d.username
                  << ", createdTime: " << d.createdTime
                  << ", lastLoginTime: " << d.lastLoginTime
                  << ", birthday: " << d.birthday << ", isMale: " << d.isMale
                  << ", nickName: " << d.nickname
                  << ", signature: " << d.nickname << ", ip: " << d.ip
                  << ", packageid: " << d.packageid << std::endl;
        return os;
    }
};

struct ChatGroup {
    int id;
    std::string name;
    MSGPACK_DEFINE(id, name);
    friend std::ostream &operator<<(std::ostream &os, ChatGroup &d) {
        std::cout << "[UserInfo]"
                  << "id: " << d.id << ",name: " << d.name << std::endl;
        return os;
    }
};

enum class MsgType { MSGTYPE_TEXT, MSGTYPE_FILE, MSGTYPE_IMAGE };

MSGPACK_ADD_ENUM(MsgType);
struct Message {
    int id;
    int sender;
    int receiver;
    MsgType type;
    std::string createdTime;
    std::string editedTime;
    bool isToGroup;
    std::string content;
    MSGPACK_DEFINE(id, sender, receiver, type, createdTime, isToGroup, content);
    friend std::ostream &operator<<(std::ostream &os, Message &d) {
        std::cout << "[UserInfo]"
                  << "id: " << d.id << ",sender: " << d.sender << ", receiver"
                  << d.receiver << ", createdTime:" << d.createdTime
                  << ", editedTime: " << d.editedTime
                  << ", isToGroup: " << d.isToGroup
                  << ", content: " << d.content << std::endl;
        return os;
    }
};

struct chatGroup_with_members {
    int chatGroupId;
    std::string chatGroupName;
    std::vector<int> members;
    MSGPACK_DEFINE(chatGroupId, chatGroupName, members);
};

struct package {
    int packageid;
    std::string packageName;
    MSGPACK_DEFINE(packageid, packageName);
};