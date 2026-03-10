//
// Created by Harel on 3/4/2026.
//

#include "client.h"

int open_client_socket(const char *ip, const int port) {
    const int sock = socket(AF_INET, SOCK_STREAM, TCP);
    struct sockaddr_in sock_addr;

    if (sock == INVALID_SOCKET) {
        write_log("Error at socket call.\n");
        return INVALID_SOCKET;
    }

    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(port);
    sock_addr.sin_addr.s_addr = inet_addr(ip);

    sprintf("Connecting to: %s:%d\n", ip, port);
    if (connect(sock, (struct sockaddr*) &sock_addr, sizeof(sock_addr)) == SOCKET_ERROR) {
        sprintf(log_msg, "Error Connecting. ID: %s\n", strerror(errno));
        write_log(log_msg);
        return INVALID_SOCKET;
    }

    return sock;
}

void help() {
    char *help = "Commands:\n\n"
                 "get [file]\t\tGets a file from the server.\n"
                 "set [file]\t\tUploads a file to the server.\n"
                 "ls\t\t\tLists all files in the servers directory.\n"
                 "help\t\t\tPrints this message.\n";
    printf("%s", help);
}

void get(const int sock, const char input[BUFSIZE]) {
    char *file_name = get_arg(input);
    if (file_name == NULL) {
        write_log("Invalid Argument.\n");
        return;
    }

    if (send_packet(sock, input, (int)strlen(input)) == SOCKET_ERROR) {
        sprintf(log_msg, "Error sending get. ID: %s\n", strerror(errno));
        write_log(log_msg);
        free(file_name);
        return;
    }

    sprintf(log_msg, "Writing To: %s\n", file_name);
    write_log(log_msg);
    recv_file(sock, file_name);

    free(file_name);
}

void set(const int sock, const char input[BUFSIZE]) {
    char *file_name = get_arg(input);
    if (file_name == NULL) {
        write_log("Invalid Argument.\n");
        return;
    }

    if (send_packet(sock, input, (int)strlen(input)) == SOCKET_ERROR) {
        sprintf(log_msg, "Error sending get. ID: %s\n", strerror(errno));
        write_log(log_msg);
        free(file_name);
        return;
    }

    sprintf(log_msg, "Reading %s\n", file_name);
    write_log(log_msg);
    send_file(sock, file_name);

    free(file_name);
}

void list_dir(const int sock, char input[BUFSIZE]) {
    if (send_packet(sock, input, (int)strlen(input)) == SOCKET_ERROR) {
        sprintf(log_msg, "Error Sending The Command. ID: %s\n", strerror(errno));
        write_log(log_msg);
        return;
    }

    ssize_t size;
    if ((size = recv_packet(sock, buffer)) == SOCKET_ERROR) {
        sprintf(log_msg, "Error Receiving The Number Of Files. ID: %s\n", strerror(errno));
        write_log(log_msg);
        return;
    }

    if (size >= BUFSIZE) {
        write_log("Size Too Big.");
        return;
    }

    buffer[size] = '\0';
    const long file_num = strtol(buffer, NULL, 10);
    for (int i = 0; i < file_num; i++) {
        if ((size = recv_packet(sock, buffer)) == SOCKET_ERROR) {
            sprintf(log_msg, "Error Receiving The File's Name. ID: %s\n", strerror(errno));
            write_log(log_msg);
            return;
        }
        if (size >= BUFSIZE) {
            write_log("File Name Size Too Big.\n");
            continue;
        }
        buffer[size] = '\0';
        sprintf(log_msg, "%s\n", buffer);
        write_log(log_msg);
    }

    sprintf(log_msg, "\nListed %ld Files.\n", file_num);
    write_log(log_msg);
}

int process_cmd(char input[BUFSIZE], const int sock) {
    char *cmd = get_method(input);
    if (cmd == NULL) {
        write_log("Invalid Command.\n");
        return 1;
    }

    if (strcmp(cmd, "help") == 0) {
        help();
    } else if (strcmp(cmd, "get") == 0) {
        get(sock, input);
    } else if (strcmp(cmd, "set") == 0) {
        set(sock, input);
    } else if (strcmp(cmd, "ls") == 0) {
        list_dir(sock, input);
    } else if (strcmp(cmd, "exit") == 0) {
        free(cmd);
        if (send_packet(sock, "exit\0", (int)strlen("exit\0")) == SOCKET_ERROR) {
            sprintf(log_msg, "Error sending the data. ID: %s\n", strerror(errno));
            write_log(log_msg);
            return 1;
        }
        return 0;
    } else if (strcmp(cmd, "shutdown") == 0) {
        free(cmd);
        if (send_packet(sock, "shutdown\0", (int)strlen("shutdown\0")) == SOCKET_ERROR) {
            sprintf(log_msg, "Error sending the data. ID: %s\n", strerror(errno));
            write_log(log_msg);
            return 1;
        }
        return 0;
    } else {
        write_log("Unknown Command.\n");
    }

    free(cmd);
    return 1;
}

void session(const int sock) {
    int flag = 1;

    while (flag) {
        write_log("> ");
        fgets(buffer, BUFSIZE, stdin);
        flag = process_cmd(buffer, sock);
    }
}

void client(char *ip, int port) {
    if (ip == NULL) {
        ip = IP;
    }
    if (port == 0) {
        port = PORT;
    }

    // TODO: Implement login.

    const int sock = open_client_socket(ip, port);

    sprintf(log_msg, "Session started with host: %s\n", ip);

    session(sock);

    free(ip);
    close(sock);
}
