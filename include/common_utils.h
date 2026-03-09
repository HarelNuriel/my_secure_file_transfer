//
// Created by Harel on 3/6/2026.
//

#ifndef SECURE_FILE_TRANSFER_COMMON_UTILS_H
#define SECURE_FILE_TRANSFER_COMMON_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define BUFSIZE 1024
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#define TCP 0
#define IP "127.0.0.1\0"
#define PORT 4444

int get_command_len(const char input[BUFSIZE]);
char* get_method(const char input[BUFSIZE]);
char* get_arg(const char input[BUFSIZE]);
long get_file_size(const char *file_name);
int send_file(int sock, const char *file_name);
int recv_file(int sock, const char *file_name);
void free_double_pointer(char** arr, int length);
void write_log(const char *path, char *msg);

#endif //SECURE_FILE_TRANSFER_COMMON_UTILS_H