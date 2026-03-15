//
// Created by Harel on 3/12/26.
//

#ifndef MY_SECURE_FILE_TRANSFER_AUTH_H
#define MY_SECURE_FILE_TRANSFER_AUTH_H

#include "common_utils.h"
#include "auth.h"

#define PASSWD_FILE "users.txt"
#define VALID_CREDS 1
#define INVALID_CREDS (-2)

int auth_ready();
void free_auth();
int validate_auth_user(const char *name, const char *passwd);

struct user {
    char *name;
    unsigned long long key;
    unsigned char priv;
};

typedef  enum {
    PRIV_READ = 1,
    PRIV_WRITE = 2,
    PRIV_EDIT_USERS = 4,
    PRIV_ADMIN = 7
} privileges;

#endif //MY_SECURE_FILE_TRANSFER_AUTH_H