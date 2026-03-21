//
// Created by Harel on 3/19/26.
//

#ifndef MY_SECURE_FILE_TRANSFER_RSA_H
#define MY_SECURE_FILE_TRANSFER_RSA_H

#include <stdlib.h>
#include <stdint.h>
#include <sys/random.h>

#define RSA_SIZE 32

typedef struct {
    uint32_t p;
    uint32_t q;
    uint16_t e;
    uint64_t n;
    uint64_t d;
} rsa_param;

typedef struct {
    uint64_t n;
    uint64_t exp;
} key_pair;

void generate_keys(rsa_param *params);
char* encrypt(char *message, ssize_t size, const key_pair *key);
char* decrypt(char *message, ssize_t size, const key_pair *key);

#endif //MY_SECURE_FILE_TRANSFER_RSA_H