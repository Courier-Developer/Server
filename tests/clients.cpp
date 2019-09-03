#include <feverrpc/feverrpc.hpp>
#include <feverrpc/threadmanager.hpp>
#include <feverrpc/utils.hpp>
#include <database/db-functions.hpp>
#include <iostream>
using namespace std;

int deal_push_friend(Friend f) {
    cout << f << endl;
    return 2;
}

int deal_message_push(Message m) {
    cout << m << endl;
    return 1;
}

Login ret_login_info() {
    return Login{username : "example", password : "password",ip:"test.ip"};
}

int main() {

    thread _thread{[]() {
        FeverRPC::Client _rpc("127.0.0.1");
        _rpc.bind("login", ret_login_info);
        _rpc.bind("deal_friend_push", deal_push_friend);
        _rpc.bind("deal_message_push", deal_message_push);
        _rpc.s2c();
    }};
    thread_guard g(_thread);

    FeverRPC::Client rpc("127.0.0.1");
    rpc.call<int>("login", Login{username : "uname", password : "pwod"});
    vector<char> data =
        rpc.call<std::vector<char>>("read_file", "tests/client.cpp");
    save_file("bin/client.txt", data);
    this_thread::sleep_for(chrono::seconds(2000));
    string ans = rpc.call<string>("repeat", "Yes! ", 5);
    cout << ans << endl; // Yes! Yes! Yes! Yes! Yes!
    int ans2 = rpc.call<int>("echo");
    cout << "login result: " << ans2 << endl; // Yes! Yes! Yes! Yes! Yes!
}