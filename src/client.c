//
// Created by Harel on 3/4/2026.
//

#include "client.h"

SOCKET open_client_socket(const char *ip, const int port) {
    const SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct sockaddr_in sock_addr;

    if (sock == INVALID_SOCKET) {
        printf("Error at socket call.\n");
        return INVALID_SOCKET;
    }

    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(port);
    sock_addr.sin_addr.s_addr = inet_addr(ip);

    printf("Connecting to: %s:%d\n", ip, port);
    if (connect(sock, (struct sockaddr*) &sock_addr, sizeof(sock_addr)) == SOCKET_ERROR) {
        printf("Error Connecting. ID: %d\n", WSAGetLastError());
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

void get(const SOCKET sock, const char input[BUFSIZE]) {
    char *file_name = get_arg(input);
    if (file_name == NULL) {
        printf("Invalid Argument.\n");
        return;
    }

    if (send(sock, input, (int)strlen(input), 0) == SOCKET_ERROR) {
        printf("Error sending get. ID: %d\n", WSAGetLastError());
        free(file_name);
        return;
    }

    printf("Writing To: %s\n", file_name);
    recv_file(sock, file_name);

    free(file_name);
}

void set(const SOCKET sock, const char input[BUFSIZE]) {
    char *file_name = get_arg(input);
    if (file_name == NULL) {
        printf("Invalid Argument.\n");
        return;
    }

    if (send(sock, input, (int)strlen(input), 0) == SOCKET_ERROR) {
        printf("Error sending get. ID: %d\n", WSAGetLastError());
        free(file_name);
        return;
    }

    printf("Reading %s\n", file_name);
    send_file(sock, file_name);

    free(file_name);
}

// TODO: Fix TCP stream merging
void list_dir(const SOCKET sock, char input[BUFSIZE]) {
    if (send(sock, input, (int)strlen(input), 0) == SOCKET_ERROR) {
        printf("Error Sending The Command. ID: %d\n", WSAGetLastError());
        return;
    }

    char buffer[BUFSIZE];
    int size;
    if ((size = recv(sock, buffer, BUFSIZE, 0)) == SOCKET_ERROR) {
        printf("Error Receiving The Number Of Files. ID: %d\n", WSAGetLastError());
        return;
    }

    if (size >= BUFSIZE) {
        printf("Size Too Big.");
        return;
    }

    buffer[size] = '\0';
    const int file_num = strtol(buffer, NULL, 10);
    for (int i = 0; i < file_num; i++) {
        if ((size = recv(sock, buffer, BUFSIZE, 0)) == SOCKET_ERROR) {
            printf("Error Receiving The File's Name. ID: %d\n", WSAGetLastError());
            return;
        }
        if (size >= BUFSIZE) {
            printf("File Name Size Too Big.");
            continue;
        }
        buffer[size] = '\0';
        printf("%s\n", buffer);
    }

    printf("\nListed %d Files.", file_num);
}

int process_cmd(char input[BUFSIZE], const SOCKET sock) {
    char *cmd = get_method(input);
    if (cmd == NULL) {
        printf("Invalid Command.\n");
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
        if (send(sock, "exit\0", (int)strlen("exit\0"), 0) == SOCKET_ERROR) {
            printf("Error sending the data. ID: %d\n", WSAGetLastError());
            return 1;
        }
        return 0;
    } else if (strcmp(cmd, "shutdown") == 0) {
        free(cmd);
        if (send(sock, "shutdown\0", (int)strlen("shutdown\0"), 0) == SOCKET_ERROR) {
            printf("Error sending the data. ID: %d\n", WSAGetLastError());
            return 1;
        }
        return 0;
    } else {
        printf("Unknown Command.\n");
    }

    free(cmd);
    return 1;
}

void session(const SOCKET sock) {
    char buffer [BUFSIZE] = {0};
    int flag = 1;

    while (flag) {
        printf("> ");
        fgets(buffer, BUFSIZE, stdin);
        flag = process_cmd(buffer, sock);
    }
}

void client(char *ip, int port) {
    struct WSAData wsa_data;
    const int ret = WSAStartup(MAKEWORD(2, 2), &wsa_data);

    if (ret != 0) {
        printf("WSAStartup failed: %d\n", ret);
        return;
    }

    if (ip == NULL) {
        ip = IP;
    }
    if (port == 0) {
        port = PORT;
    }

    // TODO: Implement login.

    const SOCKET sock = open_client_socket(ip, port);

    printf("Session started with host: %s\n", ip);

    session(sock);

    closesocket(sock);
    WSACleanup();
}
