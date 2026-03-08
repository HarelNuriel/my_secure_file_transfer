#include "server.h"

int open_server_socket(const int port) {
    const int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in sock_addr = {0};

    if (sock == INVALID_SOCKET) {
        printf("Error at socket call.\n");
        return INVALID_SOCKET;
    }

    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(port);
    sock_addr.sin_addr.s_addr = inet_addr(IP);

    if (bind(sock, (struct sockaddr*) &sock_addr, sizeof(sock_addr)) == SOCKET_ERROR) {
        printf(("Error Binding.\n"));
        return INVALID_SOCKET;
    }

    if (listen(sock, 1) == SOCKET_ERROR) {
        printf(("Error Listening.\n"));
        return INVALID_SOCKET;
    }
    printf("Listening on %d\n", port);

    return sock;
}

void recv_file_from_client(const int sock, char input[BUFSIZE]) {
    char *file_name = get_arg(input);
    if (file_name == NULL) {
        printf("Failed To Get The Argument.\n");
        return;
    }

    printf("Writing To: %s.\n", file_name);
    recv_file(sock, file_name);

    free(file_name);
}

void send_file_to_client(const int sock, char input[BUFSIZE]) {
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
    char *cwd = getcwd(NULL, 0);
    if (cwd == NULL) {
        return NULL;
    }

    const size_t dir_len = strlen(cwd) + strlen(dir) + 4;
    char *full_dir = malloc(sizeof(char) * dir_len);
    if (full_dir == NULL) {
        return NULL;
    }
    memset(full_dir, 0, dir_len);

    strcat(full_dir, cwd);
    strcat(full_dir, "/");
    strcat(full_dir, dir);
    strcat(full_dir, "/\0");

    free(cwd);
    return full_dir;
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
        printf("Error Allocating Memory (2).\n");
        free(files);
        return NULL;
    }

    DIR* d = opendir(path);
    struct dirent *entry;
    if (d == NULL) {
        printf("Error Opening Path. ID: %s", strerror(errno));
        free(path);
        free(files);
        return NULL;
    }

    int i = 1;
    char **temp;
    while ((entry = readdir(d)) != NULL) {
        if (*length >= MIN_FILES * i) {
            i++;
            temp = realloc(files, sizeof(char*) * MIN_FILES * i);
            if (temp == NULL) {
                printf("Error Reallocating Memory. ID: %s", strerror(errno));
                free(path);
                free_double_pointer(files, *length);
                return NULL;
            }
            files = temp;
        }
        files[*length] = malloc(sizeof(char) * strlen(entry->d_name) + 1);
        strcpy(files[(*length)++], entry->d_name);
    }

    closedir(d);
    free(path);
    return files;
}

// TODO: Fix TCP stream merging
void ls(const int sock, char input[BUFSIZE]) {
    int num_of_files, flag = -1, len;
    char *dir = get_arg(input);
    if (dir == NULL) {
        dir = ".";
        flag = 0;
    }
    char **files = get_dir_file_list(dir, &num_of_files), buffer[BUFSIZE];
    if (files == NULL) {
        printf("Error Allocating Memory (2).\n");
        if (flag != 0) {
            free(dir);
        }
        return;
    }

    len = sprintf(buffer, "%d", num_of_files);
    if (send(sock, buffer, len, 0) == SOCKET_ERROR) {
        printf("Error Sending The Number Of Files. ID: %s", strerror(errno));
        free_double_pointer(files, num_of_files);
        if (flag != 0) {
            free(dir);
        }
        return;
    }

    for (int i = 0; i < num_of_files; i++) {
        if (send(sock, files[i], (int)strlen(files[i]), 0) == SOCKET_ERROR) {
            printf("Error Sending The Files Names. ID: %s", strerror(errno));
            free_double_pointer(files, num_of_files);
            if (flag != 0) {
                free(dir);
            }
            return;
        }
    }

    free_double_pointer(files, num_of_files);
    if (flag != 0) {
        free(dir);
    }
}

int read_socket(const int sock) {
    char buffer [BUFSIZE];
    ssize_t len;

    while (1) {
        if ((len = recv(sock, buffer, BUFSIZE, 0)) == SOCKET_ERROR) {
            printf("Error receiving data. Error id: %s\n", strerror(errno));
            continue;
        }
        if (len == 0) {
            return 0;
        }

        char *method;
        // TODO: Accept commands larger than BUFSIZE.
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
            free(method);
            return -1;
        } else {
            printf("Unknown Command.\n");
        }

        free(method);
    }
}


void server(int port) {
    if (port == 0) {
        port = PORT;
    }
    const int sock = open_server_socket(port);
    int client = INVALID_SOCKET;
    if (sock == INVALID_SOCKET) {
        printf("Error Setting Up The Socket\n");
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
            close(sock);
            close(client);
            break;
        }
        close(client);
    }
}