#include <database/db-functions.hpp>
#include <feverrpc/feverrpc.hpp>
#include <feverrpc/threadmanager.hpp>
#include <feverrpc/utils.hpp>
#include <iostream>
using namespace std;

extern ThreadManager threadManager;

int deal_push_friend(Friend f) {
    cout << f << endl;
    return 2;
}

int deal_message_push(Message m) {
    cout << m << endl;
    return 1;
}


Login ret_login_info() {
    return Login{username : "fky", password : "password", ip : "123.1.1.1"};
}

int main() {
    FeverRPC::Client rpc("127.0.0.1");
    rpc.call<int>("login", "fky", "password","1.1.1.1");
    thread _thread{[]() {
        FeverRPC::Client _rpc("127.0.0.1");
        _rpc.bind("login", ret_login_info);
        _rpc.bind("deal_friend_push", deal_push_friend);
        _rpc.bind("newMessage", deal_message_push);
        _rpc.s2c();
    }};
    thread_guard g(_thread);
    rpc.call<int>("send_message",2,2,1,false,"Hello, My Friend!");

    this_thread::sleep_for(chrono::seconds(2));
    rpc.call<bool>("logout", 1);
    this_thread::sleep_for(chrono::seconds(20000));
}