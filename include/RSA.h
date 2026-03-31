//
// Created by Harel on 3/19/26.
//

#ifndef MY_SECURE_FILE_TRANSFER_RSA_H
#define MY_SECURE_FILE_TRANSFER_RSA_H

#include <stdint.h>
#include <stdlib.h>
#include <sys/random.h>

#define RSA_SIZE 32

struct rsa_param {
    uint32_t p;
    uint32_t q;
    uint16_t e;
    uint64_t n;
    uint64_t d;
};

struct key_pair {
    uint64_t n;
    uint64_t exp;
};

struct enc_msg {
    char *msg;
    uint64_t len;
};

void generate_keys(struct rsa_param *params);
struct enc_msg encrypt(char *message, ssize_t size, const struct key_pair key);
char *decrypt(char *message, ssize_t size, const struct key_pair key);

#endif // MY_SECURE_FILE_TRANSFER_RSA_H
