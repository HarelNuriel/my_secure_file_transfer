//
// Created by Harel on 3/15/26.
//

#ifndef MY_SECURE_FILE_TRANSFER_SHA256_H
#define MY_SECURE_FILE_TRANSFER_SHA256_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>

#define SHA256_SIZE 64

char* sha256(const char *plain_text);

#endif //MY_SECURE_FILE_TRANSFER_SHA256_H