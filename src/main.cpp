#include <database/db-functions.hpp>
#include <feverrpc/feverrpc.hpp>
#include <feverrpc/threadmanager.hpp>
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
            rpc.bind("get_info_by_uid", get_info_by_uid);
            rpc.bind("get_info_by_username",get_info_by_username);
            rpc.bind("update_info", update_info);
            rpc.bind("list_friends", list_friends);
            rpc.bind("request_friend", request_friend);
            rpc.bind("make_friend", make_friend);
            rpc.bind("delete_friend", delete_friend);

            rpc.bind("find_package",find_package);
            rpc.bind("change_package",change_package);
            rpc.bind("create_package",create_package);
            rpc.bind("create_chatGroup_and_invite_friends",create_chatGroup_and_invite_friends);

            rpc.bind("join_chatGroup", join_chatGroup);
            rpc.bind("list_chat_groups", list_chat_groups);
            rpc.bind("create_chat_group", create_chat_group);
            rpc.bind("get_group_mumber", get_group_mumber);
            rpc.bind("leave_group", leave_group);
            rpc.bind("insert_message", insert_message);
            rpc.bind("get_unread_messages", get_unread_messages);

            rpc.bind("get_my_info",get_my_info);
            rpc.bind("get_all_friends_info",get_all_friends_info);
            rpc.bind("get_all_chatGroups_info",get_all_chatGroups_info);

            rpc.c2s();
        }};
        thread_guard gg(_thread_1);
    }
    return 0;
}
