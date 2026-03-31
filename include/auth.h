//
// Created by Harel on 3/12/26.
//

#ifndef MY_SECURE_FILE_TRANSFER_AUTH_H
#define MY_SECURE_FILE_TRANSFER_AUTH_H

#include "RSA.h"
#include "common_utils.h"
#include "sha256.h"

#define PASSWD_FILE "users.txt"
#define VALID_CREDS 1
#define PRIVILEGES_OK 1
#define INVALID_CREDS (-2)
#define INSUFFICIENT_PRIVILEGES (-3)

int auth_ready();
void free_auth();
char *prepare_creds(const char *name, const char *passwd);
int validate_auth_user(const char *hash);
int add_user(const char *hash, int privileges);
int rm_user(const char *name_hash, int id);
int change_privileges(const char *name_hash, int privileges, int id);

struct user {
    int sock;
    struct key_pair key;
    unsigned int privilege;
};

typedef enum {
    PRIV_READ = 1,
    PRIV_WRITE = 2,
    PRIV_EDIT_USERS = 4,
    PRIV_ADMIN = 7
} privileges;

#endif // MY_SECURE_FILE_TRANSFER_AUTH_H
