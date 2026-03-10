//
// Created by Harel on 3/6/2026.
//

#include "common_utils.h"

#include <math.h>

static FILE *log_stream;

char* get_arg(const char input[BUFSIZE]) {
    int cmd_len = get_command_len(input);
    char log_msg[BUFSIZE];
    if (cmd_len >= strlen(input)) {
        write_log("Invalid Argument.\n");
        return NULL;
    }

    cmd_len++;
    const size_t name_len = strlen(input + cmd_len);
    if (name_len == 0) {
        write_log("Invalid Argument.\n");
        return NULL;
    }

    char *file_name = malloc(sizeof(char) * (name_len + 1));

    if (file_name == NULL) {
        snprintf(log_msg, BUFSIZE, "Error allocating memory. ID: %s\n", strerror(errno));
        write_log(log_msg);
        return NULL;
    }

    for (int i = 0; i < name_len; i++) {
        file_name[i] = input[i + cmd_len];
    }

    if (file_name[name_len - 1] == '\n') {
        file_name[name_len - 1] = '\0';
    } else {
        file_name[name_len] = '\0';
    }

    return file_name;
}

char* get_method(const char input[BUFSIZE]) {
    int len = get_command_len(input);

    if (len == 0) {
        return NULL;
    }

    char *cmd = malloc(sizeof(char) * (len + 1));
    if (cmd == NULL) {
        return 0;
    }
    len = 0;

    while (input[len] != ' ' && input[len] != '\0' && input[len] != '\n') {
        cmd[len] = input[len];
        len++;
    }
    cmd[len] = '\0';

    return cmd;
}

int get_command_len(const char input[BUFSIZE]) {
    int len = 0;
    while (input[len] != ' ' && input[len] != '\0' && input[len] != '\n') {
        len++;
    }

    return len;
}

long get_file_size(const char *file_name) {
    FILE *fp = fopen(file_name, "rb");
    char log_msg[BUFSIZE];
    if (fp == NULL) {
        snprintf(log_msg, BUFSIZE, "Error Opening The File. ID: %s\n", strerror(errno));
        write_log(log_msg);
        return -1;
    }

    long size = 0;
    if (fseek(fp, 0, SEEK_END) == 0) {
        size = ftell(fp);
    }

    fclose(fp);
    return size;
}

int send_packet(const int sock, const char *buffer, const unsigned int length) {
    const unsigned int n_len = htonl(length);
    char log_msg[BUFSIZE];
    if (send(sock, &n_len, sizeof(unsigned int), 0) == SOCKET_ERROR) {
        snprintf(log_msg, BUFSIZE, "Error sending buffer size. ID: %s\n", strerror(errno));
        write_log(log_msg);
        return SOCKET_ERROR;
    }

    if (send(sock, buffer, length, 0) == SOCKET_ERROR) {
        snprintf(log_msg, BUFSIZE, "Error sending buffer. ID: %s\n", strerror(errno));
        write_log(log_msg);
        return SOCKET_ERROR;
    }

    return 0;
}

ssize_t recv_packet(const int sock, char *buffer) {
    ssize_t pkt_size = 0;
    unsigned int len = 0;
    char log_msg[BUFSIZE];
    if ((pkt_size = recv(sock, &len, sizeof(int), 0)) == SOCKET_ERROR) {
        snprintf(log_msg, BUFSIZE, "Error receiving the packet's size. ID: %s\n", strerror(errno));
        write_log(log_msg);
        return SOCKET_ERROR;
    }
    if (pkt_size == 0) {
        return 0;
    }

    unsigned int size = ntohl(len);
    char *temp = buffer;
    memset(buffer, 0, BUFSIZE);
    while (size > 0) {
        if ((pkt_size = recv(sock, temp, size, 0)) == SOCKET_ERROR) {
            snprintf(log_msg, BUFSIZE, "Error receiving the packet. ID: %s\n", strerror(errno));
            write_log(log_msg);
            return SOCKET_ERROR;
        }
        if (pkt_size == 0) {
            return 0;
        }
        temp += pkt_size;
        size -= pkt_size;
    }

    return ntohl(len);
}

int send_file(const int sock, const char *file_name) {
    long size = get_file_size(file_name);
    char buffer[BUFSIZE], s_size[BUFSIZE], log_msg[BUFSIZE];
    FILE *fp = fopen(file_name, "rb");

    if (fp == NULL) {
        snprintf(log_msg, BUFSIZE, "Error opening the file. ID: %s\n", strerror(errno));
        write_log(log_msg);
        return 0;
    }

    if (size == -1) {
        return 0;
    }
    const unsigned int len = snprintf(s_size, BUFSIZE, "%ld", size);

    if (send_packet(sock, s_size, len) ==  SOCKET_ERROR) {
        snprintf(log_msg, BUFSIZE, "Error Sending The File Size. ID: %s\n", strerror(errno));
        write_log(log_msg);
        return 0;
    }

    for (; size > BUFSIZE; size -= BUFSIZE) {
        if (fread(buffer, sizeof(char), BUFSIZE, fp)) {
            if (send_packet(sock, buffer, BUFSIZE) == SOCKET_ERROR) {
                snprintf(log_msg, BUFSIZE, "Error Sending The File. ID: %s\n", strerror(errno));
                write_log(log_msg);
                return 0;
            }
        }
    }

    if (size > 0) {
        if (fread(buffer, sizeof(char), size, fp)) {
            if (send_packet(sock, buffer, size) == SOCKET_ERROR) {
                snprintf(log_msg, BUFSIZE, "Error Sending The File. ID: %s\n", strerror(errno));
                write_log(log_msg);
                return 0;
            }
        }
    } else if (ferror(fp)) {
        snprintf(log_msg, BUFSIZE, "File reading error. ID: %s\n", strerror(errno));
        write_log(log_msg);
    }

    fclose(fp);

    return 1;
}

int recv_file(const int sock, const char *file_name) {
    FILE *fp = fopen(file_name, "wb");
    char buffer [BUFSIZE], log_msg[BUFSIZE];
    ssize_t size;

    if (fp == NULL) {
        snprintf(log_msg, BUFSIZE, "Error opening the file. ID: %s\n", strerror(errno));
        write_log(log_msg);
        return 0;
    }

    if ((size = recv_packet(sock, buffer)) == SOCKET_ERROR) {
        snprintf(log_msg, BUFSIZE, "Error receiving file size. ID: %s\n", strerror(errno));
        write_log(log_msg);
        return 0;
    }

    if (size >= BUFSIZE) {
        snprintf(log_msg, BUFSIZE, "Size Too Big.\n");
        write_log(log_msg);
        return 0;
    }
    buffer[size] = '\0';
    long len = strtol(buffer, NULL, 10);

    for (; len > BUFSIZE; len -= size) {
        if ((size = recv_packet(sock, buffer)) == SOCKET_ERROR) {
            snprintf(log_msg, BUFSIZE, "Error Receiving File Data. ID: %s\n", strerror(errno));
            write_log(log_msg);
            return 0;
        }
        fwrite(buffer, sizeof(char), size, fp);
    }

    while (len > 0) {
        if ((size = recv_packet(sock, buffer)) == SOCKET_ERROR) {
            snprintf(log_msg, BUFSIZE, "Error Receiving File Data. ID: %s\n", strerror(errno));
            write_log(log_msg);
            return 0;
        }
        fwrite(buffer, sizeof(char), size, fp);
        len -= size;
    }

    snprintf(log_msg, BUFSIZE, "File Received Successfully.\n");
    write_log(log_msg);
    fclose(fp);
    return 1;
}

void free_double_pointer(char** arr, const int length) {
    for (int i = 0; i < length; i++) {
        free(arr[i]);
    }
    free(arr);
}

void write_log(char *msg) {
    FILE *fp = log_stream == NULL ? stdout : log_stream;
    if (fp == NULL) {
        return;
    }
    fprintf(fp, "%s", msg);
    fflush(fp);
}

void set_log_stream(FILE *stream) {
    log_stream = stream;
}