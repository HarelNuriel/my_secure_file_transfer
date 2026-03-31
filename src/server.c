#include "server.h"
#include "RSA.h"
#include "common_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/random.h>

static char *save_path;

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

    if (bind(sock, (struct sockaddr *)&sock_addr, sizeof(sock_addr)) ==
        SOCKET_ERROR) {
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

char *get_data_path(const char *rel_path) {
    char log_msg[BUFSIZE];
    size_t len = strlen(save_path) + strlen(rel_path) + 1;
    char *full_path = malloc(sizeof(*full_path) * len);
    if (full_path == NULL) {
        snprintf(log_msg, BUFSIZE, "Error Allocating Memmory. %s",
                 strerror(errno));
        write_log(log_msg);
        return NULL;
    }

    snprintf(full_path, len, "%s%s", save_path, rel_path);
    return full_path;
}

void recv_file_from_client(const int sock, char input[BUFSIZE]) {
    char *file_name = get_arg(input);
    char log_msg[BUFSIZE];
    if (file_name == NULL) {
        snprintf(log_msg, BUFSIZE, "Failed To Get The Argument.\n");
        write_log(log_msg);
        return;
    }

    char *full_path = get_data_path(file_name);
    free(file_name);
    if (full_path == NULL) {
        return;
    }

    snprintf(log_msg, BUFSIZE, "Writing To: %s.\n", full_path);
    write_log(log_msg);
    recv_file(sock, full_path);

    free(full_path);
}

void send_file_to_client(const int sock, char input[BUFSIZE]) {
    char *file_name = get_arg(input), log_msg[BUFSIZE];
    if (file_name == NULL) {
        snprintf(log_msg, BUFSIZE, "Failed To Get The Argument.\n");
        write_log(log_msg);
        return;
    }

    char *full_path = get_data_path(file_name);
    free(file_name);
    if (full_path == NULL) {
        return;
    }

    snprintf(log_msg, BUFSIZE, "Reading %s.\n", full_path);
    write_log(log_msg);
    send_file(sock, full_path);

    free(full_path);
}

char **get_dir_file_list(const char *dir, int *length) {
    char *path = get_data_path(dir),
         **files = malloc(sizeof(char *) * MIN_FILES), log_msg[BUFSIZE];
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

    DIR *d = opendir(path);
    struct dirent *entry;
    if (d == NULL) {
        snprintf(log_msg, BUFSIZE, "Error Opening Path. ID: %sn",
                 strerror(errno));
        write_log(log_msg);
        free(path);
        free(files);
        return NULL;
    }

    int i = 1;
    char **temp;
    while ((entry = readdir(d)) != NULL) {
        if (strncmp(entry->d_name, ".", 1) == 0 ||
            strncmp(entry->d_name, "..", 2) == 0 ||
            strncmp(entry->d_name, "auth", 4) == 0) {
            continue;
        }
        if (*length >= MIN_FILES * i) {
            i++;
            temp = realloc(files, sizeof(char *) * MIN_FILES * i);
            if (temp == NULL) {
                snprintf(log_msg, BUFSIZE,
                         "Error Reallocating Memory. ID: %s\n",
                         strerror(errno));
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
    char *dir = get_arg(input), buffer[BUFSIZE], log_msg[BUFSIZE];
    ;
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
        snprintf(log_msg, BUFSIZE,
                 "Error Sending The Number Of Files. ID: %s\n",
                 strerror(errno));
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
        if (send_packet(sock, files[i], (int)strlen(files[i])) ==
            SOCKET_ERROR) {
            snprintf(log_msg, BUFSIZE,
                     "Error Sending The Files Names. ID: %s\n",
                     strerror(errno));
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

int validate_privilege(const struct user *session, const int privilege) {
    unsigned int flag;
    if ((session->privilege & privilege) != privilege) {
        flag = htonl(INSUFFICIENT_PRIVILEGES);
        write_log("Sending INSUFFICIENT_PRIVILEGES.\n");
        send(session->sock, &flag, sizeof(int), 0);
        return 0;
    }
    flag = htonl(PRIVILEGES_OK);
    send(session->sock, &flag, sizeof(int), 0);
    return 1;
}

void proc_add_user(const int sock, char input[BUFSIZE]) {
    unsigned int flag;
    int argc = 0;
    char log_msg[BUFSIZE], **argv = get_args(input, &argc);
    if (argv == NULL || argc < 3) {
        snprintf(log_msg, BUFSIZE, "Invalid Arguments. %d\n", argc);
        write_log(log_msg);
        if (argv != NULL)
            free_double_pointer(argv, argc);
        return;
    }
    snprintf(log_msg, BUFSIZE, "Adding User %s:%s.\n", argv[0], argv[1]);
    write_log(log_msg);

    for (int i = 0; i < 3; i++) {
        if (argv[i] == NULL) {
            snprintf(log_msg, BUFSIZE, "Argument %d is invalid.\n", i);
            write_log(log_msg);
            free_double_pointer(argv, i - 1);
            return;
        }
    }

    int priv = (int)strtol(argv[2], NULL, 10);
    char *creds = prepare_creds(argv[0], argv[1]);

    if ((flag = add_user(creds, priv))) {
        snprintf(log_msg, BUFSIZE, "Added User %s Successfully.\n", argv[0]);
    } else {
        snprintf(log_msg, BUFSIZE, "Could not Add User %s.\n", argv[0]);
    }

    flag = htonl(flag);
    write_log(log_msg);
    send(sock, &flag, sizeof(int), 0);
    free(creds);
    free_double_pointer(argv, argc);
}

void proc_rm_user(const int sock, char input[BUFSIZE]) {
    char log_msg[BUFSIZE], *name = get_arg(input);
    unsigned int flag;
    if (name == NULL) {
        write_log("Invalid Argument.\n");
        return;
    }

    char *name_hash = sha256(name);
    // TODO: When added theads add thread_id var that will set the id val
    if ((flag = rm_user(name_hash, 0))) {
        snprintf(log_msg, BUFSIZE, "Remove User %s Successfully.\n", name);
    } else {
        snprintf(log_msg, BUFSIZE, "Could not Remove User %s.\n", name);
    }

    flag = htonl(flag);
    write_log(log_msg);
    send(sock, &flag, sizeof(int), 0);
    free(name_hash);
    free(name);
}

void proc_chmod(const int sock, char input[BUFSIZE]) {
    int argc = 0;
    char log_msg[BUFSIZE], **argv = get_args(input, &argc);
    unsigned int flag;
    if (argv == NULL || argc < 2) {
        snprintf(log_msg, BUFSIZE, "Invalid Arguments. %d\n", argc);
        write_log(log_msg);
        if (argv != NULL)
            free_double_pointer(argv, argc);
        return;
    }

    for (int i = 0; i < 2; i++) {
        if (argv[i] == NULL) {
            snprintf(log_msg, BUFSIZE, "Argument %d is invalid.\n", i);
            write_log(log_msg);
            free_double_pointer(argv, i - 1);
            return;
        }
    }

    int priv = (int)strtol(argv[1], NULL, 10);
    char *name_hash = sha256(argv[0]);
    // TODO: When added theads add thread_id var that will set the id val
    if ((flag = change_privileges(name_hash, priv, 0))) {
        snprintf(log_msg, BUFSIZE, "Changed User %s Privileges Successfully.\n",
                 argv[0]);
    } else {
        snprintf(log_msg, BUFSIZE, "Could not Change User %s Privileges.\n",
                 argv[0]);
    }

    flag = htonl(flag);
    write_log(log_msg);
    send(sock, &flag, sizeof(int), 0);
    free(name_hash);
    free_double_pointer(argv, argc);
}

int read_socket(const struct user *session) {
    ssize_t len;
    char buffer[BUFSIZE], log_msg[BUFSIZE];

    while (1) {
        if ((len = recv_packet(session->sock, buffer, BUFSIZE)) ==
            SOCKET_ERROR) {
            snprintf(log_msg, BUFSIZE, "Error receiving data. Error ID: %s\n",
                     strerror(errno));
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
            snprintf(log_msg, BUFSIZE, "Received Input: %s\n", buffer);
            write_log(log_msg);
            method = get_method(buffer);
        } else {
            unsigned int flag = htonl(UNKNOWN_CMD);
            send(session->sock, &flag, sizeof(int), 0);
            continue;
        }

        if (method == NULL) {
            snprintf(log_msg, BUFSIZE, "Unknown Command.\n");
            write_log(log_msg);
            unsigned int flag = htonl(UNKNOWN_CMD);
            send(session->sock, &flag, sizeof(int), 0);
            continue;
        }

        if (strcmp(method, "get") == 0) {
            if (!validate_privilege(session, PRIV_READ)) {
                continue;
            }
            send_file_to_client(session->sock, buffer);
        } else if (strcmp(method, "set") == 0) {
            if (!validate_privilege(session, PRIV_WRITE)) {
                continue;
            }
            recv_file_from_client(session->sock, buffer);
        } else if (strcmp(method, "ls") == 0) {
            if (!validate_privilege(session, PRIV_READ)) {
                continue;
            }
            ls(session->sock, buffer);
        } else if (strcmp(method, "add_user") == 0) {
            if (!validate_privilege(session, PRIV_EDIT_USERS)) {
                continue;
            }
            snprintf(log_msg, BUFSIZE, "Processing Command: %s\n", method);
            write_log(log_msg);
            proc_add_user(session->sock, buffer);
        } else if (strcmp(method, "rm_user") == 0) {
            if (!validate_privilege(session, PRIV_EDIT_USERS)) {
                continue;
            }
            proc_rm_user(session->sock, buffer);
        } else if (strcmp(method, "chmod") == 0) {
            if (!validate_privilege(session, PRIV_EDIT_USERS)) {
                continue;
            }
            proc_chmod(session->sock, buffer);
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

int setup(char *data_path) {
    if (!auth_ready()) {
        write_log("Error Setting Up Auth.\n");
        return 0;
    }
    write_log("Auth Setup Complete.\n");

    char *log_file_path =
        malloc(sizeof(char) * (strlen(LOG_PATH) + strlen(LOG_FILE)) + 1);
    snprintf(log_file_path, strlen(LOG_PATH) + strlen(LOG_FILE) + 1, "%s%s",
             LOG_PATH, LOG_FILE);
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

    char log_msg[BUFSIZE];
    if (data_path == NULL) {
        char *corr_path = getcwd(NULL, 0);
        if (corr_path == NULL) {
            snprintf(log_msg, BUFSIZE, "Couldn't Get CWD: %s", strerror(errno));
            write_log(log_msg);
            return 0;
        }
        size_t len = strlen(corr_path) + 2;
        save_path = malloc(sizeof(*save_path) * len);
        snprintf(save_path, len, "%s/", corr_path);
    } else {
        size_t len = strlen(data_path) + 2;
        save_path = malloc(sizeof(*save_path) * len);
        snprintf(save_path, len, "%s/", data_path);
        free(data_path);
    }

    return 1;
}

unsigned int auth_user(const int sock, struct user *user_session) {
    char hash[CRED_SIZE];
    int priv;
    unsigned int is_valid = htonl(INVALID_CREDS);

    if (recv_packet(sock, hash, CRED_SIZE) == SOCKET_ERROR) {
        snprintf(hash, CRED_SIZE, "Error Receiving Hash. ID: %s\n",
                 strerror(errno));
        write_log(hash);
        is_valid = htonl(0);
        send(sock, &is_valid, sizeof(int), 0);
        return SOCKET_ERROR;
    }

    if ((priv = validate_auth_user(hash)) > 0) {
        is_valid = htonl(VALID_CREDS);
    }

    char buffer[BUFSIZE];
    while (send(sock, &is_valid, sizeof(int), 0) == SOCKET_ERROR) {
        snprintf(buffer, BUFSIZE, "Error Sending Auth Validation. ID: %s\n",
                 strerror(errno));
        write_log(buffer);
    }

    return priv;
}

char *prepare_challenge() {
    unsigned int len;
    getrandom(&len, sizeof(int), 0);
    len = (len % 64) + 16;

    unsigned char *challenge = calloc(len + 1, sizeof(char));
    getrandom(challenge, len, 0);

    for (int i = 0; i < len; i++) {
        challenge[i] = (challenge[i] % 26) + 'a';
    }

    return (char *)challenge;
}

int handshake(struct user *session) {
    char log_msg[BUFSIZE];
    struct rsa_param user_rsa_params;
    generate_keys(&user_rsa_params);
    write_log("Generated Keys.\n");

    struct key_pair priv_key, pub_key;
    session->key.exp = user_rsa_params.d;
    session->key.n = user_rsa_params.n;
    pub_key.exp = user_rsa_params.e;
    pub_key.n = user_rsa_params.n;

    if (send(session->sock, &pub_key, sizeof(struct key_pair), 0) ==
        SOCKET_ERROR) {
        snprintf(log_msg, BUFSIZE, "Failed To Send Key Pair: %s.\n",
                 strerror(errno));
        write_log(log_msg);
        return 0;
    }

    // Challenge
    char buffer[BUFSIZE];
    char *challenge = prepare_challenge();
    struct enc_msg enc_challenge =
        encrypt(challenge, strlen(challenge), session->key);

    if (send_packet(session->sock, enc_challenge.msg, enc_challenge.len) ==
        SOCKET_ERROR) {
        snprintf(log_msg, BUFSIZE, "Failed To Send Challenge: %s.\n",
                 strerror(errno));
        write_log(log_msg);
        free(enc_challenge.msg);
        free(challenge);
        return 0;
    }
    free(enc_challenge.msg);

    int len;
    if ((len = recv_packet(session->sock, buffer, BUFSIZE)) == SOCKET_ERROR) {
        snprintf(log_msg, BUFSIZE, "Failed To Receive Answer: %s.\n",
                 strerror(errno));
        write_log(log_msg);
        free(challenge);
        return 0;
    }

    int has_passed = htonl(1);
    if (strncmp(challenge, buffer, len) != 0) {
        write_log("Client Failed Challenge.\n");
        free(challenge);
        has_passed = htonl(0);
        send(session->sock, &has_passed, sizeof(int), 0);
        return 0;
    }
    free(challenge);

    if (send(session->sock, &has_passed, sizeof(int), 0) == SOCKET_ERROR) {
        write_log("Failed To Send OK To The Client.\n");
        return 0;
    }

    return 1;
}

void server(char *ip, int port, char *data_path) {
    if (!setup(data_path)) {
        write_log("Couldn't Finish Setup.\n");
        return;
    }
    write_log("Finished Setup.\n");

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
    while (1) {
        i = 0;
        client = accept(sock, NULL, NULL);
        if (client == SOCKET_ERROR) {
            write_log("Client Socket Error.\n");
            continue;
        }

        struct user user_session;
        while ((user_session.privilege = auth_user(client, &user_session)) <=
                   0 &&
               i < 3) {
            i++;
        }
        if (user_session.privilege > 0) {
            user_session.sock = client;
            if (!handshake(&user_session)) {
                write_log("Client Failed The Challenge.\n");
            } else {
                flag = read_socket(&user_session);
                if (flag == -1) {
                    close_log_stream();
                    close(sock);
                    close(client);
                    break;
                }
            }
        }
        close(client);
    }

    if (save_path != NULL) {
        free(save_path);
    }
}
