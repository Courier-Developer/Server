#include <feverrpc/threadmanager.hpp>
#include <database/db-functions.hpp>
#include <feverrpc/feverrpc.hpp>
#include <feverrpc/utils.hpp>
#include <iostream>

extern ThreadManager threadManager;

int main(int argc, char const *argv[]) {
    while (1) {
        thread _thread{[]() {
            FeverRPC::Server _rpc(threadManager);
            _rpc.s2c();
        }};
        thread_guard g(_thread);

        thread _thread_1{[]() {
            FeverRPC::Server rpc(threadManager);
            rpc.bind("login", login);
            rpc.bind("logout", logout);
            rpc.bind("register", register_account);
            rpc.bind("get_info", get_info);
            rpc.bind("update_info", update_info);
            rpc.bind("list_friends", list_friends);
            rpc.bind("get_friend", get_friend);
            rpc.bind("request_friend", request_friend);
            rpc.bind("make_friend", make_friend);
            rpc.bind("delete_friend", delete_friend);
            rpc.bind("change_friendGroup", change_friendGroup);
            rpc.bind("search_group", search_group);
            rpc.bind("join_chatGroup", join_chatGroup);
            rpc.bind("list_chat_groups", list_chat_groups);
            rpc.bind("create_chat_group", create_chat_group);
            rpc.bind("get_group_mumber", get_group_mumber);
            rpc.bind("leave_group", leave_group);
            rpc.bind("insert_message", insert_message);
            rpc.bind("get_unread_messages", get_unread_messages);

            rpc.c2s();
        }};
        thread_guard gg(_thread_1);
    }
    return 0;
}
