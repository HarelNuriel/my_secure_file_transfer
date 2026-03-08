#include "server.h"

SOCKET open_server_socket(const int port) {
    const SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in sock_addr = {0};

    if (sock == INVALID_SOCKET) {
        printf("Error at socket call.\n");
        return INVALID_SOCKET;
    }

    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(port);
    sock_addr.sin_addr.s_addr = inet_addr(IP);

    if (bind(sock, (SOCKADDR*) &sock_addr, sizeof(sock_addr)) == SOCKET_ERROR) {
        printf(("Error Binding.\n"));
        return INVALID_SOCKET;
    }

    printf("Listening on %d\n", port);
    if (listen(sock, 1) == SOCKET_ERROR) {
        printf(("Error Listening.\n"));
        return INVALID_SOCKET;
    }

    return sock;
}

void recv_file_from_client(const SOCKET sock, char input[BUFSIZE]) {
    char *file_name = get_arg(input);
    if (file_name == NULL) {
        printf("Failed To Get The Argument.\n");
        return;
    }

    printf("Writing To: %s.\n", file_name);
    recv_file(sock, file_name);

    free(file_name);
}

void send_file_to_client(const SOCKET sock, char input[BUFSIZE]) {
    char *file_name = get_arg(input);
    if (file_name == NULL) {
        printf("Failed To Get The Argument.\n");
        return;
    }

    printf("Reading %s.\n", file_name);
    send_file(sock, file_name);

    free(file_name);
}

char* get_path(const char* dir) {
    char *path = malloc(sizeof(char) * BUFSIZE);
    DWORD ret = GetCurrentDirectory(BUFSIZE, path);
    if (ret == 0) {
        printf("Getting Directory Name Failed (1).\n");
        free(path);
        return NULL;
    }
    if (ret > BUFSIZE) {
        char *temp = realloc(path, sizeof(char) * ret);
        if (temp == NULL) {
            printf("Error Reallocating Memory.\n");
            free(path);
            return NULL;
        }
        path = temp;
    }
    ret = GetCurrentDirectory(ret, path);
    if (ret == 0) {
        printf("Getting Directory Name Failed (1).\n");
        free(path);
        return NULL;
    }

    long path_len = ret + strlen(dir) + 4;
    if (path_len > BUFSIZE) {
        char *temp = realloc(path, sizeof(char) * (path_len + 1));
        if (temp == NULL) {
            free(path);
            printf("Error Reallocating Memory.\n");
            return NULL;
        }
        path = temp;
    }
    strcat(path, "\\");
    strcat(path, dir);
    strcat(path, "\\*");

    return path;
}

char** get_dir_file_list(const char* dir, int *length) {
    char *path = get_path(dir), **files = malloc(sizeof(char*) * MIN_FILES);
    *length = 0;
    if (files == NULL) {
        if (path != NULL) {
            free(path);
        }
        printf("Error Allocating Memory (1).\n");
        return NULL;
    }
    if (path == NULL) {
        free(files);
        return NULL;
    }

    WIN32_FIND_DATA file_data;
    HANDLE file_handle = FindFirstFile(path, &file_data);
    if (file_handle == INVALID_HANDLE_VALUE) {
        printf("Error: Could Not Open Directory.\n");
        free_double_pointer(files, *length);
        free(path);
        return NULL;
    }

    files[(*length)] = malloc(sizeof(char) * (strlen(file_data.cFileName) + 1));
    if (files[*length] == NULL) {
        printf("Error Allocating Memory (2).\n");
        free_double_pointer(files, *length);
        free(path);
        FindClose(file_handle);
        return NULL;
    }
    strcpy(files[(*length)++], file_data.cFileName);

    int i = 1;
    while (FindNextFile(file_handle, &file_data) != 0) {
        if (*length >= MIN_FILES * i) {
            i++;
            char **f_temp = realloc(files, sizeof(char *) * (MIN_FILES * i));
            if (f_temp == NULL) {
                free_double_pointer(files, *length);
                free(path);
                printf("Memory Reallocation Error.\n");
                FindClose(file_handle);
                return NULL;
            }
            files = f_temp;
        }
        files[(*length)] = malloc(sizeof(char) * (strlen(file_data.cFileName) + 1));
        if (files[*length] == NULL) {
            printf("Error Allocating Memory (3).\n");
            free_double_pointer(files, *length);
            free(path);
            FindClose(file_handle);
            return NULL;
        }
        strcpy(files[(*length)++], file_data.cFileName);
    }

    FindClose(file_handle);
    free(path);
    return files;
}

// TODO: Fix TCP stream merging
void ls(SOCKET sock, char input[BUFSIZE]) {
    int num_of_files, len;
    const char *dir = get_arg(input);
    if (dir == NULL) {
        dir = ".";
    }
    char **files = get_dir_file_list(dir, &num_of_files), buffer[BUFSIZE];

    len = sprintf(buffer, "%d", num_of_files);
    if (send(sock, buffer, len, 0) == SOCKET_ERROR) {
        printf("Error Sending The Number Of Files. ID: %d", WSAGetLastError());
        free_double_pointer(files, num_of_files);
        return;
    }

    for (int i = 0; i < num_of_files; i++) {
        if (send(sock, files[i], (int)strlen(files[i]), 0) == SOCKET_ERROR) {
            printf("Error Sending The Files Names. ID: %d", WSAGetLastError());
            free_double_pointer(files, num_of_files);
            return;
        }
    }

    free_double_pointer(files, num_of_files);
}

int read_socket(const SOCKET sock) {
    char buffer [BUFSIZE];
    int len;

    while (1) {
        if ((len = recv(sock, buffer, BUFSIZE, 0)) == SOCKET_ERROR) {
            printf("Error receiving data. Error id: %d\n", WSAGetLastError());
            continue;
        }
        if (len == 0) {
            return 0;
        }

        char *method;
        if (len < BUFSIZE) {
            buffer[len] = '\0';
            printf("Received command: %s\n", buffer);
            method = get_method(buffer);
        } else {
            continue;
        }

        if (method == NULL) {
            printf("Unknown Command.\n");
            continue;
        }

        if (strcmp(method, "get") == 0) {
            send_file_to_client(sock, buffer);
        } else if (strcmp(method, "set") == 0) {
            recv_file_from_client(sock, buffer);
        } else if (strcmp(method, "ls") == 0) {
            ls(sock, buffer);
        } else if (strcmp(method, "exit") == 0) {
            free(method);
            return 0;
        } else if (strcmp(method, "shutdown") == 0) {
            return -1;
        } else {
            printf("Unknown Command.\n");
        }

        free(method);
    }
}


void server(int port) {
    struct WSAData wsa_data;
    const int ret = WSAStartup(MAKEWORD(2, 2), &wsa_data);

    if (ret != 0) {
        printf("WSAStartup failed: %d\n", ret);
        return;
    }

    if (port == 0) {
        port = PORT;
    }
    const SOCKET sock = open_server_socket(port);
    SOCKET client = INVALID_SOCKET;
    if (sock == INVALID_SOCKET) {
        printf("Error Setting Up The Socket\n");
        WSACleanup();
        return;
    }

    int flag;
    while (1) {
        client = accept(sock, NULL, NULL);
        if (client == SOCKET_ERROR) {
            printf("Client Socket Error.\n");
            continue;
        }
        // TODO: auth client and start secure session.
        flag = read_socket(client);
        if (flag == -1) {
            closesocket(sock);
            closesocket(client);
            WSACleanup();
            break;
        }
        closesocket(client);
    }
}