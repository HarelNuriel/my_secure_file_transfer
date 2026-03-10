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

    long size = 0;
    if (fseek(fp, 0, SEEK_END) == 0) {
        size = ftell(fp);
    }

    fclose(fp);
    return size;
}

int send_packet(const int sock, const char *buffer, const unsigned int length) {
    const unsigned int n_len = htonl(length);
    if (send(sock, &n_len, sizeof(unsigned int), 0) == SOCKET_ERROR) {
        printf("Error sending buffer size. ID: %s\n", strerror(errno));
        return SOCKET_ERROR;
    }

    if (send(sock, buffer, length, 0) == SOCKET_ERROR) {
        printf("Error sending buffer. ID: %s\n", strerror(errno));
        return SOCKET_ERROR;
    }

    return 0;
}

ssize_t recv_packet(const int sock, char *buffer) {
    ssize_t pkt_size = 0;
    unsigned int len = 0;
    if ((pkt_size = recv(sock, &len, sizeof(int), 0)) == SOCKET_ERROR) {
        printf("Error receiving the packet's size. ID: %s\n", strerror(errno));
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
            printf("Error receiving the packet. ID: %s\n", strerror(errno));
            return SOCKET_ERROR;
        }
        temp += pkt_size;
        size -= pkt_size;
    }

    return ntohl(len);
}

int send_file(const int sock, const char *file_name) {
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
    const unsigned int len = sprintf(s_size, "%ld", size);

    if (send_packet(sock, s_size, len) ==  SOCKET_ERROR) {
        printf("Error Sending The File Size. ID: %s\n", strerror(errno));
        return 0;
    }

    for (; size > BUFSIZE; size -= BUFSIZE) {
        if (fread(buffer, sizeof(char), BUFSIZE, fp)) {
            if (send_packet(sock, buffer, BUFSIZE) == SOCKET_ERROR) {
                printf("Error Sending The File. ID: %s\n", strerror(errno));
                return 0;
            }
        }
    }

    if (size > 0) {
        if (fread(buffer, sizeof(char), size, fp)) {
            if (send_packet(sock, buffer, size) == SOCKET_ERROR) {
                printf("Error Sending The File. ID: %s\n", strerror(errno));
                return 0;
            }
        }
    } else if (ferror(fp)) {
        printf("File reading error.\n");
    }

    fclose(fp);

    return 1;
}

int recv_file(const int sock, const char *file_name) {
    FILE *fp = fopen(file_name, "wb");
    char buffer [BUFSIZE];
    ssize_t size;

    if (fp == NULL) {
        printf("Error opening the file.\n");
        return 0;
    }

    if ((size = recv_packet(sock, buffer)) == SOCKET_ERROR) {
        printf("Error receiving file size.");
        return 0;
    }

    if (size >= BUFSIZE) {
        printf("Size Too Big.");
        return 0;
    }
    buffer[size] = '\0';
    long len = strtol(buffer, NULL, 10);

    for (; len > BUFSIZE; len -= size) {
        if ((size = recv_packet(sock, buffer)) == SOCKET_ERROR) {
            printf("Error Receiving File Data.");
            return 0;
        }
        fwrite(buffer, sizeof(char), size, fp);
    }

    while (len > 0) {
        if ((size = recv_packet(sock, buffer)) == SOCKET_ERROR) {
            printf("Error Receiving File Data.");
            return 0;
        }
        fwrite(buffer, sizeof(char), size, fp);
        len -= size;
    }

    printf("File Received Successfully.\n");
    fclose(fp);
    return 1;
}

void free_double_pointer(char** arr, const int length) {
    for (int i = 0; i < length; i++) {
        free(arr[i]);
    }
    free(arr);
}

void write_log(const char *path, char *msg) {
    FILE *fp = fopen(path, "a");
    if (fp == NULL) {
        printf("Error opening server.log");
        return;
    }

    if (fprintf(fp, "%s", msg) == -1) {
        printf("Error writing to server.log");
    }

    fclose(fp);
}