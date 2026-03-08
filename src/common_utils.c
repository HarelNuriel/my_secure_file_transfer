//
// Created by Harel on 3/6/2026.
//

#include "common_utils.h"

char* get_arg(const char input[BUFSIZE]) {
    int cmd_len = get_command_len(input);
    if (cmd_len >= strlen(input)) {
        printf("Invalid Argument.");
        return NULL;
    }

    cmd_len++;
    const size_t name_len = strlen(input + cmd_len);
    if (name_len == 0) {
        printf("Invalid Argument.");
        return NULL;
    }

    char *file_name = malloc(sizeof(char) * (name_len + 1));

    if (file_name == NULL) {
        printf("Error allocating memory.\n");
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
    if (fp == NULL) {
        printf("Error Opening The File.\n");
        return -1;
    }

    int size = 0;
    if (fseek(fp, 0, SEEK_END) == 0) {
        size = ftell(fp);
    }

    fclose(fp);
    return size;
}

int send_file(const SOCKET sock, const char *file_name) {
    long size = get_file_size(file_name);
    char buffer[BUFSIZE], s_size[BUFSIZE];
    FILE *fp = fopen(file_name, "rb");

    if (fp == NULL) {
        printf("Error opening the file.\n");
        return 0;
    }

    if (size == -1) {
        return 0;
    }
    const long len = sprintf(s_size, "%ld", size);

    if (send(sock, s_size, len, 0) ==  SOCKET_ERROR) {
        printf("Error Sending The File Size. ID: %d\n", WSAGetLastError());
        return 0;
    }

    // TODO: Fix TCP stream merging.
    for (; size > BUFSIZE; size -= BUFSIZE) {
        if (fread(buffer, sizeof(char), BUFSIZE, fp)) {
            if (send(sock, buffer, BUFSIZE, 0) == SOCKET_ERROR) {
                printf("Error Sending The File. ID: %d\n", WSAGetLastError());
                return 0;
            }
        }
    }

    if (size > 0) {
        if (fread(buffer, sizeof(char), size, fp)) {
            if (send(sock, buffer, size, 0) == SOCKET_ERROR) {
                printf("Error Sending The File. ID: %d\n", WSAGetLastError());
                return 0;
            }
        }
    } else if (ferror(fp)) {
        printf("File reading error.\n");
    }

    fclose(fp);

    return 1;
}

int recv_file(const SOCKET sock, const char *file_name) {
    FILE *fp = fopen(file_name, "wb");
    char buffer [BUFSIZE];
    int size;

    if (fp == NULL) {
        printf("Error opening the file.\n");
        return 0;
    }

    if ((size = recv(sock, buffer, BUFSIZE, 0)) == SOCKET_ERROR) {
        printf("Error receiving file size.");
        return 0;
    }

    if (size >= BUFSIZE) {
        printf("Size Too Big.");
        return 0;
    }
    buffer[size] = '\0';
    long len = strtol(buffer, NULL, 10);

    size = 0;
    for (; len > BUFSIZE; len -= size) {
        if ((size = recv(sock, buffer, BUFSIZE, 0)) == SOCKET_ERROR) {
            printf("Error Receiving File Data.");
            return 0;
        }
        fwrite(buffer, sizeof(char), size, fp);
    }

    // TODO: Fix receiving the final chuck (TCP Truncated)
    if (len > 0) {
        if ((size = recv(sock, buffer, len, 0)) == SOCKET_ERROR) {
            printf("Error Receiving File Data.");
            return 0;
        }
        fwrite(buffer, sizeof(char), size, fp);
    }

    printf("File Received Successfully.\n");
    fclose(fp);
    return 1;
}

void free_double_pointer(char** arr, int length) {
    for (int i = 0; i < length; i++) {
        free(arr[i]);
    }
    free(arr);
}