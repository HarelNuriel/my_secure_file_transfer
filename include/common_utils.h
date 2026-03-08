//
// Created by Harel on 3/6/2026.
//

#ifndef SECURE_FILE_TRANSFER_COMMON_UTILS_H
#define SECURE_FILE_TRANSFER_COMMON_UTILS_H

#include <stdio.h>
#include <winsock2.h>

#define BUFSIZE 1024
#define IP "127.0.0.1\0"
#define PORT 1234

int get_command_len(const char input[BUFSIZE]);
char* get_method(const char input[BUFSIZE]);
char* get_arg(const char input[BUFSIZE]);
long get_file_size(const char *file_name);
int send_file(SOCKET sock, const char *file_name);
int recv_file(SOCKET sock, const char *file_name);
void free_double_pointer(char** arr, int length);

#endif //SECURE_FILE_TRANSFER_COMMON_UTILS_H