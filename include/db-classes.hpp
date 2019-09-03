#include <iostream>
#include <string>


template <typename T> class Response {
    bool success;
    std::string msg;

  public:
    T data;
    Response(bool success, std::string msg,T data):success(success),msg(msg),data(data){};
    Response(bool success, std::string msg):success(success),msg(msg){};
};

struct UserInfo{
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
};

/// 好友和群聊好友都用这个？
/// A：群聊中的用Userinfo
struct Friend{
    int uid;
    std::string group; ///< 哪个用户组，是好友与好友关系
    ///已解决
    std::string username;
    std::string createdTime;
    std::string lastLoginTime;
    // TODO avatar
    std::string birthday;
    bool isMale;
    std::string ip;
    std::string nickname;
    bool isMute;
};

struct ChatGroup{
    int id;
    std::string name;
};

enum MsgType {
    MSGTYPE_TEXT,
    MSGTYPE_FILE,
    MSGTYPE_IMAGE
};

class Message{

public:
    int id;
    int sender;
    int receiver;
    MsgType type;
    std::string createdTime;
    std::string editedTime;
    bool isToGroup;
    std::string content;
};
