#include "server.h"

#include "auth.h"

int open_server_socket(const int port, const char *ip) {
    const int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in sock_addr = {0};
    char log_msg[BUFSIZE];

    if (sock == INVALID_SOCKET) {
        snprintf(log_msg, BUFSIZE, "Error at socket call.\n");
        write_log(log_msg);
        return INVALID_SOCKET;
    }

    sock_addr.sin_family = AF_INET;
    sock_addr.sin_port = htons(port);
    sock_addr.sin_addr.s_addr = inet_addr(ip);

    if (bind(sock, (struct sockaddr*) &sock_addr, sizeof(sock_addr)) == SOCKET_ERROR) {
        snprintf(log_msg, BUFSIZE, "Error Binding. ID: %s\n", strerror(errno));
        write_log(log_msg);
        return INVALID_SOCKET;
    }

    if (listen(sock, 1) == SOCKET_ERROR) {
        snprintf(log_msg, BUFSIZE, "Error Listening.\n");
        write_log(log_msg);
        return INVALID_SOCKET;
    }
    snprintf(log_msg, BUFSIZE, "Listening on %d\n", port);
    write_log(log_msg);

    return sock;
}

void recv_file_from_client(const int sock, char input[BUFSIZE]) {
    char *file_name = get_arg(input);
    char log_msg[BUFSIZE];
    if (file_name == NULL) {
        snprintf(log_msg, BUFSIZE, "Failed To Get The Argument.\n");
        write_log(log_msg);
        return;
    }

    snprintf(log_msg, BUFSIZE, "Writing To: %s.\n", file_name);
    write_log(log_msg);
    recv_file(sock, file_name);

    free(file_name);
}

void send_file_to_client(const int sock, char input[BUFSIZE]) {
    char *file_name = get_arg(input), log_msg[BUFSIZE];;
    if (file_name == NULL) {
        snprintf(log_msg, BUFSIZE, "Failed To Get The Argument.\n");
        write_log(log_msg);
        return;
    }

    snprintf(log_msg, BUFSIZE, "Reading %s.\n", file_name);
    write_log(log_msg);
    send_file(sock, file_name);

    free(file_name);
}

char** get_dir_file_list(const char* dir, int *length) {
    char *path = get_path(dir), **files = malloc(sizeof(char*) * MIN_FILES), log_msg[BUFSIZE];
    *length = 0;
    if (files == NULL) {
        if (path != NULL) {
            free(path);
        }
        snprintf(log_msg, BUFSIZE, "Error Allocating Memory (1).\n");
        write_log(log_msg);
        return NULL;
    }
    if (path == NULL) {
        snprintf(log_msg, BUFSIZE, "Error Allocating Memory (2).\n");
        write_log(log_msg);
        free(files);
        return NULL;
    }

    DIR* d = opendir(path);
    struct dirent *entry;
    if (d == NULL) {
        snprintf(log_msg, BUFSIZE, "Error Opening Path. ID: %sn", strerror(errno));
        write_log(log_msg);
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
                snprintf(log_msg, BUFSIZE, "Error Reallocating Memory. ID: %s\n", strerror(errno));
                write_log(log_msg);
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

void ls(const int sock, char input[BUFSIZE]) {
    int num_of_files, flag = -1, len;
    char *dir = get_arg(input), buffer[BUFSIZE], log_msg[BUFSIZE];;
    if (dir == NULL) {
        dir = ".";
        flag = 0;
    }
    char **files = get_dir_file_list(dir, &num_of_files);
    if (files == NULL) {
        snprintf(log_msg, BUFSIZE, "Error Allocating Memory (2).\n");
        write_log(log_msg);
        if (flag != 0) {
            free(dir);
        }
        return;
    }

    len = snprintf(buffer, BUFSIZE, "%d", num_of_files);
    if (send_packet(sock, buffer, len) == SOCKET_ERROR) {
        snprintf(log_msg, BUFSIZE, "Error Sending The Number Of Files. ID: %s\n", strerror(errno));
        write_log(log_msg);
        free_double_pointer(files, num_of_files);
        if (flag != 0) {
            free(dir);
        }
        return;
    }

    for (int i = 0; i < num_of_files; i++) {
        snprintf(log_msg, BUFSIZE, "Sending: %s\n", files[i]);
        write_log(log_msg);
        if (send_packet(sock, files[i], (int)strlen(files[i])) == SOCKET_ERROR) {
            snprintf(log_msg, BUFSIZE, "Error Sending The Files Names. ID: %s\n", strerror(errno));
            write_log(log_msg);
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
    ssize_t len;
    char buffer[BUFSIZE], log_msg[BUFSIZE];

    while (1) {
        if ((len = recv_packet(sock, buffer, BUFSIZE)) == SOCKET_ERROR) {
            snprintf(log_msg, BUFSIZE, "Error receiving data. Error ID: %s\n", strerror(errno));
            write_log(log_msg);
            continue;
        }
        if (len == 0) {
            return 0;
        }

        char *method;
        // TODO: Accept commands larger than BUFSIZE.
        if (len < BUFSIZE) {
            buffer[len] = '\0';
            snprintf(log_msg, BUFSIZE, "Received command: %s\n", buffer);
            write_log(log_msg);
            method = get_method(buffer);
        } else {
            continue;
        }

        if (method == NULL) {
            snprintf(log_msg, BUFSIZE, "Unknown Command.\n");
            write_log(log_msg);
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

int setup() {
    if (!auth_ready()) {
        write_log("Error Setting Up Auth.\n");
        return 0;
    }

    char *log_file_path = malloc(sizeof(char) * (strlen(LOG_PATH) + strlen(LOG_FILE)) + 1);
    snprintf(log_file_path, strlen(LOG_PATH) + strlen(LOG_FILE) + 1, "%s%s", LOG_PATH, LOG_FILE);
    if (access(log_file_path, F_OK) != 0) {
        DIR *d = opendir(LOG_PATH);
        if (d == NULL) {
            if (mkdir(LOG_PATH, 0744) != 0) {
                printf("Error creating %s, ID: %s", LOG_PATH, strerror(errno));
                free(log_file_path);
                return 0;
            }
            d = opendir(LOG_PATH);
        }
        FILE *fp = fopen(log_file_path, "w");
        if (fp == NULL) {
            printf("Error creating %s, ID: %s", log_file_path, strerror(errno));
            free(log_file_path);
            return 0;
        }
        fclose(fp);
        closedir(d);
    }

    FILE *log_file = fopen(log_file_path, "w");
    if (log_file == NULL) {
        free(log_file_path);
        return 0;
    }
    set_log_stream(log_file);
    free(log_file_path);

    return 1;
}

// TODO: After hash implementation refactor with known lengths.
unsigned int auth_user(const int sock) {
    char hash[CRED_SIZE];
    unsigned int is_valid = htonl(INVALID_CREDS);

    if (recv_packet(sock, hash, CRED_SIZE) == SOCKET_ERROR) {
        snprintf(hash, CRED_SIZE, "Error Receiving Hash. ID: %s\n", strerror(errno));
        write_log(hash);
        is_valid = htonl(0);
        send(sock, &is_valid, sizeof(int), 0);
        return SOCKET_ERROR;
    }

    if (validate_auth_user(hash) == VALID_CREDS) {
        is_valid = htonl(VALID_CREDS);
    }

    char buffer[BUFSIZE];
    while (send(sock, &is_valid, sizeof(int), 0) == SOCKET_ERROR) {
        snprintf(buffer, BUFSIZE, "Error Sending Auth Validation. ID: %s\n", strerror(errno));
        write_log(buffer);
    }

    return ntohl(is_valid);
}

void server(char *ip, int port) {
    if (!setup()) {
        write_log("Couldn't Finish Setup.\n");
        return;
    }

    if (port == 0) {
        port = PORT;
    }
    if (ip == NULL) {
        ip = malloc(sizeof(char) * strlen(IP) + 1);
        if (ip == NULL) {
            write_log("Error Allocating ip Memory.\n");
            return;
        }
        strcpy(ip, IP);
    }

    const int sock = open_server_socket(port, ip);
    int client = INVALID_SOCKET;
    free(ip);

    if (sock == INVALID_SOCKET) {
        write_log("Error Setting Up The Socket\n");
        return;
    }

    int flag, i;
    unsigned int is_valid;
    while (1) {
        i = 0;
        client = accept(sock, NULL, NULL);
        if (client == SOCKET_ERROR) {
            write_log("Client Socket Error.\n");
            continue;
        }
        // TODO: auth client and start secure session.
        while ((is_valid = auth_user(client)) != VALID_CREDS && i < 3) {
            i++;
        }
        if (is_valid == VALID_CREDS) {
            flag = read_socket(client);
            if (flag == -1) {
                close_log_stream();
                close(sock);
                close(client);
                break;
            }
        }
        close(client);
    }
}