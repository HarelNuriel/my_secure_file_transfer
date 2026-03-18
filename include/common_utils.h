//
// Created by Harel on 3/6/2026.
//

#ifndef SECURE_FILE_TRANSFER_COMMON_UTILS_H
#define SECURE_FILE_TRANSFER_COMMON_UTILS_H

#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <dirent.h>
#include <sys/stat.h>
#include "sha256.h"

#define BUFSIZE 1024
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#define UNKNOWN_CMD (-2)
#define TCP 0
#define IP "127.0.0.1\0"
#define PORT 4444
#define CRED_SIZE (SHA256_SIZE * 2 + 1)

int get_command_len(const char input[BUFSIZE]);
char* get_method(const char input[BUFSIZE]);
char** get_args(char input[BUFSIZE], int *argc);
char* get_arg(const char input[BUFSIZE]);
long get_file_size(const char *file_name);
int send_file(int sock, const char *file_name);
int recv_file(int sock, const char *file_name);
void free_double_pointer(char** arr, int length);
void write_log(char *msg);
ssize_t recv_packet(int sock, char *buffer, long max_size);
int send_packet(int sock, const char *buffer, unsigned int length);
void set_log_stream(FILE *stream);
char* get_path(const char* dir);
void close_log_stream();

#endif //SECURE_FILE_TRANSFER_COMMON_UTILS_H