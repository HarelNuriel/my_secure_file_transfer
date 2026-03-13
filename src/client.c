//
// Created by Harel on 3/4/2026.
//

#include "client.h"

#include "auth.h"

int open_client_socket(const char *ip, const int port) {
    char log_msg[BUFSIZE];
    const int sock = socket(AF_INET, SOCK_STREAM, TCP);
    struct sockaddr_in sock_addr;

    if (sock == INVALID_SOCKET) {
        write_log("Error at socket call.\n");
        return INVALID_SOCKET;
    }

    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(port);
    sock_addr.sin_addr.s_addr = inet_addr(ip);

    snprintf(log_msg, BUFSIZE, "Connecting to: %s:%d\n", ip, port);
    if (connect(sock, (struct sockaddr*) &sock_addr, sizeof(sock_addr)) == SOCKET_ERROR) {
        snprintf(log_msg, BUFSIZE, "Error Connecting. ID: %s\n", strerror(errno));
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
    char log_msg[BUFSIZE];
    char *file_name = get_arg(input);
    if (file_name == NULL) {
        write_log("Invalid Argument.\n");
        return;
    }

    if (send_packet(sock, input, (int)strlen(input)) == SOCKET_ERROR) {
        snprintf(log_msg, BUFSIZE, "Error sending get. ID: %s\n", strerror(errno));
        write_log(log_msg);
        free(file_name);
        return;
    }

    snprintf(log_msg, BUFSIZE, "Writing To: %s\n", file_name);
    write_log(log_msg);
    recv_file(sock, file_name);

    snprintf(log_msg, BUFSIZE, "Successfully Received %s.\n", file_name);
    write_log(log_msg);
    free(file_name);
}

void set(const int sock, const char input[BUFSIZE]) {
    char log_msg[BUFSIZE];
    char *file_name = get_arg(input);
    if (file_name == NULL) {
        write_log("Invalid Argument.\n");
        return;
    }

    if (send_packet(sock, input, (int)strlen(input)) == SOCKET_ERROR) {
        snprintf(log_msg, BUFSIZE, "Error sending get. ID: %s\n", strerror(errno));
        write_log(log_msg);
        free(file_name);
        return;
    }

    snprintf(log_msg, BUFSIZE, "Reading %s\n", file_name);
    write_log(log_msg);
    send_file(sock, file_name);

    snprintf(log_msg, BUFSIZE, "Successfully Sent %s.\n", file_name);
    write_log(log_msg);
    free(file_name);
}

void list_dir(const int sock, char input[BUFSIZE]) {
    char log_msg[BUFSIZE], buffer[BUFSIZE];
    if (send_packet(sock, input, (int)strlen(input)) == SOCKET_ERROR) {
        snprintf(log_msg, BUFSIZE, "Error Sending The Command. ID: %s\n", strerror(errno));
        write_log(log_msg);
        return;
    }

    ssize_t size;
    if ((size = recv_packet(sock, buffer)) == SOCKET_ERROR) {
        snprintf(log_msg, BUFSIZE, "Error Receiving The Number Of Files. ID: %s\n", strerror(errno));
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
            snprintf(log_msg, BUFSIZE, "Error Receiving The File's Name. ID: %s\n", strerror(errno));
            write_log(log_msg);
            return;
        }
        if (size >= BUFSIZE) {
            write_log("File Name Size Too Big.\n");
            continue;
        }
        buffer[size] = '\0';
        snprintf(log_msg, BUFSIZE, "%s\n", buffer);
        write_log(log_msg);
    }

    snprintf(log_msg, BUFSIZE, "\nListed %ld Files.\n", file_num);
    write_log(log_msg);
}

int process_cmd(char input[BUFSIZE], const int sock) {
    char *cmd = get_method(input), log_msg[BUFSIZE];
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
            snprintf(log_msg, BUFSIZE, "Error sending the data. ID: %s\n", strerror(errno));
            write_log(log_msg);
            return 1;
        }
        return 0;
    } else if (strcmp(cmd, "shutdown") == 0) {
        free(cmd);
        if (send_packet(sock, "shutdown\0", (int)strlen("shutdown\0")) == SOCKET_ERROR) {
            snprintf(log_msg, BUFSIZE, "Error sending the data. ID: %s\n", strerror(errno));
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
    char buffer[BUFSIZE];

    while (flag) {
        write_log("> ");
        fgets(buffer, BUFSIZE, stdin);
        flag = process_cmd(buffer, sock);
    }
}

// TODO: After hash implementation refactor with known lengths.
unsigned int auth(const int sock) {
    char name[BUFSIZE], passwd[BUFSIZE];
    int is_valid = 0;

    printf("Please Enter Username and Password:\n");
    printf("Username: ");
    fgets(name, BUFSIZE, stdin);
    name[strcspn(name, "\n")] = '\0';

    printf("Password: ");
    fgets(passwd, BUFSIZE, stdin);
    passwd[strcspn(passwd, "\n")] = '\0';

    if (send_packet(sock, name, strlen(name)) == SOCKET_ERROR) {
        printf("Error Sending The Username. ID: %s\n", strerror(errno));
        return 0;
    }
    if (send_packet(sock, passwd, strlen(passwd)) == SOCKET_ERROR) {
        printf("Error Sending The Username. ID: %s\n", strerror(errno));
        return 0;
    }

    while (recv(sock, &is_valid, sizeof(int), 0) == SOCKET_ERROR) {
        printf("Error Receiving Answer. ID: %s\n", strerror(errno));
    }

    return ntohl(is_valid);
}

void client(char *ip, int port) {
    char log_msg[BUFSIZE];
    if (ip == NULL) {
        ip = IP;
    }
    if (port == 0) {
        port = PORT;
    }

    const int sock = open_client_socket(ip, port);

    snprintf(log_msg, BUFSIZE, "Session started with host: %s\n", ip);
    write_log(log_msg);

    unsigned int i = 0, is_valid;
    while ((is_valid = auth(sock)) != VALID_CREDS && i < 3) {
        if (is_valid == INVALID_CREDS) {
            printf("Invalid Credentials.\n");
            i++;
        }
    }
    if (is_valid == VALID_CREDS) {
        session(sock);
    }

    free(ip);
    close(sock);
}
