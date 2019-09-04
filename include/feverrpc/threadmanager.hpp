#ifndef FEVERRPC_THREADMENAGER_H_
#define FEVERRPC_THREADMENAGER_H_

#pragma once
#include "msgpack.hpp"
#include <cstdio>
#include <database/db-classes.hpp>
#include <iostream>
#include <map>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <unistd.h>
using std::cout;
using std::endl;
using std::map;
using std::queue;
using std::string;
using std::thread;

enum class PushType {
    MESSAGE,
    FRIEND_WANGTED,
    FRIEND_CONFIRMED,
    FRIEND_DELETED,
    LOGIN,
    LOGOUT,
    GROUP_ADDED,
};

MSGPACK_ADD_ENUM(PushType);

class UserStatus {

  public:
    const std::thread::id thread_id;
    int socket_handler;
    int count;
    queue<std::pair<PushType, Friend>> friend_queue;
    queue<std::pair<PushType, Message>> message_queue;
    queue<std::pair<PushType, ChatGroup>> group_queue;
    queue<std::pair<PushType, int>> status_queue;

    UserStatus(thread::id tid, int socket_handler)
        : thread_id(tid), count(0), socket_handler(socket_handler){};
    UserStatus()
        : count(0), thread_id(std::this_thread::get_id()), socket_handler(-1){};
};

/// \brief 用于维护在线状态，用户对应的线程号, 心跳count，消息队列，采用互斥锁解决竞争问题。
class ThreadManager {
    map<int, UserStatus> _status;
    map<std::thread::id, int> _threads;
    std::mutex _mtx;
    static const int COUNT_LIMIT = 30;

  public:
    
    /// \brief 注册对应线程的uid，用于保证数据库操作代码能够获取uid
    void reg_thread(int uid) {
        std::lock_guard<std::mutex> guard(_mtx);
        _threads[std::this_thread::get_id()] = uid;
    }
    /// \brief 获得自己线程的uid，但首先保证注册过
    int get_uid() {
        std::lock_guard<std::mutex> guard(_mtx);
        return _threads[std::this_thread::get_id()];
    }
    /// \brief 注销一个线程对应的uid，与 `reg_thread` 互斥
    bool unreg_thread() {
        std::lock_guard<std::mutex> guard(_mtx);
        return _threads.erase(std::this_thread::get_id());
    }
    /// \brief 判断一个用户是否在线
    bool online(int uid) {
        std::lock_guard<std::mutex> guard(_mtx);
        if (_status.count(uid) <= 0) {
            return false;
        }
        if (_status[uid].count > COUNT_LIMIT) {
            // if timeout, unregister the user.
            _status.erase(uid);
            return false;
        }
        return true;
    }
    /// \brief 判断一个用户是否被强制下线了，只有 `FeverRPC` 会使用它
    bool force_logout(int uid) {
        // 是否被强制下线，在确保曾经login情况下使用
        std::lock_guard<std::mutex> guard(_mtx);
        if (_status[uid].thread_id != std::this_thread::get_id()) {
            // 被新的登录挤下线
            return true;
        }
        return false;
    }
    /// \brief 注册一个新的用户-线程等信息，`FeverRPC` 使用，并且能够挤下已在线的同一用户
    bool reg(int uid, const int socket_handler,
             thread::id tid = std::this_thread::get_id()) {
        // register

        if (online(uid) && force_logout(uid)) {
            print();
            cout << "[TM] 已检测到在线用户，干掉它" << endl;
            unreg(uid);
            {
                std::lock_guard<std::mutex> guard(_mtx);
                _status.insert(
                    std::pair<int, UserStatus>({uid, {tid, socket_handler}}));
            }
            // 一个hack，防止自己注册的进程被别人注销
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
        std::lock_guard<std::mutex> guard(_mtx);
        _status.insert(
            std::pair<int, UserStatus>({uid, {tid, socket_handler}}));
        // _threads[std::this_thread::get_id()] = uid;
        printf("[TM] register a new thread %lld from uid:%d.\n", tid, uid);
        print();
        return true;
    }

    /// \brief 注销，与 `reg` 对应
    bool unreg(int uid) {
        std::lock_guard<std::mutex> guard(_mtx);
        if (_status[uid].socket_handler >= 0) {

            printf("[TM] thread [%lld] close socket [%d]",
                   std::this_thread::get_id(), _status[uid].socket_handler);
            // close(_status[uid].socket_handler);
        }
        _status.erase(uid);
        printf("[TM] unregister uid %d from thread:%lld.\n", uid,
               std::this_thread::get_id());

        print();
        return true;
    }
    
    /// \brief 向客户端推送有关好友的即时消息
    /// \param uid
    /// \param PushType pt: PushType::Friend_xxxx 是可以使用的
    /// \param Friend f  那个Friend
    bool push(int uid, PushType pt, Friend f) {
        std::lock_guard<std::mutex> guard(_mtx);
        _status[uid].friend_queue.push(std::make_pair(pt, f));
        return true;
    }

    /// \brief 向客户端推送即时消息
    /// \param uid
    /// \param PushType pt: PushType::Message 是可以使用的
    /// \param Message m  对应Message
    bool push(int uid, PushType pt, Message m) {
        std::lock_guard<std::mutex> guard(_mtx);
        _status[uid].message_queue.push(std::make_pair(pt, m));
        return true;
    }

    /// \brief 向客户端推送有关分组即时消息
    /// \param uid
    /// \param PushType pt: PushType::Group_ADDED 是可以使用的
    /// \param ChatGroup cg 
    bool push(int uid, PushType pt, ChatGroup cg) {
        std::lock_guard<std::mutex> guard(_mtx);
        _status[uid].group_queue.push(std::make_pair(pt, cg));
        return true;
    }

    /// \brief 向客户端推送有关登录/登出即时消息
    /// \param uid
    /// \param PushType pt: PushType::LOGIN PushType::LOGOUT 是可以使用的
    /// \param int user_id 对方的uid
    bool push(int uid, PushType pt, int user_id) {
        std::lock_guard<std::mutex> guard(_mtx);
        _status[uid].status_queue.push(std::make_pair(pt, user_id));
        return true;
    }
    /// \brief 查看相应队列是否有推送
    /// FeverRPC使用
    // tp: 1 - friend_queue, 2 - message_queue, 3 - group_queue, 4 - status_queue
    bool have_push(int uid, int tp) {
        if (!online(uid))
            return false;
        std::lock_guard<std::mutex> guard(_mtx);
        // if (_status[uid].friend_queue.empty() &&
        // _status[uid].message_queue.empty()) return false;
        if (tp == 1) {
            if (_status[uid].friend_queue.empty())
                return false;
            else
                return true;
        } else if (tp == 2) {
            if (_status[uid].message_queue.empty())
                return false;
            else
                return true;
        } else if (tp == 3) {
            if (_status[uid].group_queue.empty())
                return false;
            else
                return true;
        } else if (tp == 4) {
            if (_status[uid].status_queue.empty())
                return false;
            else
                return true;
        }

        return false;
    }
    /// \brief 获得一个Friend相关的Push，
    /// FerverRPC使用
    std::pair<PushType, Friend> get_push_friend(int uid) {
        std::lock_guard<std::mutex> guard(_mtx);
        auto ret = _status[uid].friend_queue.front();
        _status[uid].friend_queue.pop();
        return ret;
    }
    /// \brief 获得一个Message相关的Push，
    /// FerverRPC使用
    std::pair<PushType, Message> get_push_message(int uid) {
        std::lock_guard<std::mutex> guard(_mtx);
        auto ret = _status[uid].message_queue.front();
        _status[uid].message_queue.pop();
        return ret;
    }
    /// \brief 获得一个Group相关的Push，
    /// FerverRPC使用
    std::pair<PushType, ChatGroup> get_push_group(int uid) {
        std::lock_guard<std::mutex> guard(_mtx);
        auto ret = _status[uid].group_queue.front();
        _status[uid].group_queue.pop();
        return ret;
    }
    /// \brief 获得一个Login/Logout相关的Push，
    /// FerverRPC使用
    std::pair<PushType, int> get_push_status(int uid) {
        std::lock_guard<std::mutex> guard(_mtx);
        auto ret = _status[uid].status_queue.front();
        _status[uid].status_queue.pop();
        return ret;
    }

    friend std::ostream &operator<<(std::ostream &os,
                                    const ThreadManager &_tm) {
        cout << "{" << endl;
        for (auto elem : _tm._status) {
            cout << "    " << elem.first << " " << elem.second.thread_id << "--"
                 << elem.second.socket_handler << "\n";
        }
        cout << "}\n";
        return os;
    }
    void print() {
        cout << "{" << endl;
        for (auto elem : _status) {
            cout << "    " << elem.first << " " << elem.second.thread_id << "--"
                 << elem.second.socket_handler << "\n";
            cout <<"\t\tfriend_queue:"<<elem.second.friend_queue.size()<<endl;
            cout <<"\t\tmessage_queue:"<<elem.second.message_queue.size()<<endl;
            cout <<"\t\tgroup_queue:"<<elem.second.group_queue.size()<<endl;
            cout <<"\t\tstatus_queue:"<<elem.second.status_queue.size()<<endl;
        }
        cout << "}\n";
    }
};

/// \brief RAII线程守护
/// 使用RAII的方式保证线程一定会在最终join
class thread_guard {
    std::thread &t;

  public:
    explicit thread_guard(std::thread &_t) : t(_t){};

    ~thread_guard() {
        puts("destructing thread");
        if (t.joinable()) {
            printf("[%lld]waiting for anthor thread to destruct %lld \n",
                   std::this_thread::get_id(), t.get_id());

            t.join();
            printf("[%lld]thread destructed\n", std::this_thread::get_id());
        }
    }

    thread_guard(const thread_guard &) = delete;
    thread_guard &operator=(const thread_guard &) = delete;
};

#endif // FEVERRPC_THREADMANAGER_H_