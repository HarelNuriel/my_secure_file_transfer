//
// Created by Harel on 3/12/26.
//

#include "auth.h"

static char *auth_path;
static char *passwd_path;

int set_auth_path(const char *path) {
    auth_path = get_path(path);
    return auth_path == NULL ? 0 : 1;
}

char* prepare_creds(const char *name, const char *passwd) {
    char *buffer = malloc(sizeof(char) * (CRED_SIZE + 1));
    if (buffer == NULL) {
        char log[BUFSIZE];
        snprintf(log, BUFSIZE, "Couldn't Allocate Memory. ID: %s\n", strerror(errno));
        write_log(log);
        return 0;
    }

    char *name_hash = sha256(name), *passwd_hash = sha256(passwd);
    snprintf(buffer, CRED_SIZE + 1, "%s:%s", name_hash, passwd_hash);

    free(name_hash);
    free(passwd_hash);
    return buffer;
}

int extract_privilege(char *buffer) {
    char *priv_str = buffer + CRED_SIZE + 1;
    return (int)strtol(priv_str, NULL, 10);
}

int validate_auth_user(const char *hash) {
    if (hash == NULL) {
        write_log("Hash is NULL.\n");
        return 0;
    }

    FILE *fp = fopen(passwd_path, "r");
    if (fp == NULL) {
        char buffer[BUFSIZE];
        snprintf(buffer, BUFSIZE, "Error Opening %s, ID: %s\n", passwd_path, strerror(errno));
        write_log(buffer);
        return 0;
    }

    char buffer[BUFSIZE] = {0};
    while (fgets(buffer, BUFSIZE, fp)) {
        if (strncmp(hash, buffer, CRED_SIZE) == 0 && strlen(buffer) > CRED_SIZE + 1) {
            buffer[strcspn(buffer, "\n")] = '\0';
            fclose(fp);
            return extract_privilege(buffer);
        }
    }

    fclose(fp);
    return INVALID_CREDS;
}

int add_user(const char *hash, const int privileges) {
    if (hash == NULL) {
        write_log("Invalid Username or Password.");
        free_auth();
        return 0;
    }

    FILE *fp = fopen(passwd_path, "a");
    if (fp == NULL) {
        char buffer[BUFSIZE];
        snprintf(buffer, BUFSIZE, "Couldn't Open %s. ID: %s\n", passwd_path, strerror(errno));
        write_log(buffer);
        return 0;
    }

    fprintf(fp, "%s:%d", hash, privileges);
    fprintf(fp, "\n");

    fclose(fp);
    return 1;
}

int auth_ready() {
    if (auth_path == NULL) {
        if (!set_auth_path("auth")) {
            char buffer[BUFSIZE];
            snprintf(buffer, BUFSIZE, "Couldn't Set Auth Path. ID: %s\n", strerror(errno));
            write_log(buffer);
            return 0;
        }
    }

    passwd_path = malloc(sizeof(char) * (strlen(auth_path) + strlen(PASSWD_FILE)) + 2);
    if (passwd_path == NULL) {
        char buffer[BUFSIZE];
        snprintf(buffer, BUFSIZE, "Couldn't Set Auth Path. ID: %s\n", strerror(errno));
        write_log(buffer);
        free(auth_path);
        return 0;
    }
    snprintf(passwd_path, strlen(auth_path) + strlen(PASSWD_FILE) + 2, "%s%s", auth_path, PASSWD_FILE);

    if (access(passwd_path, F_OK) != 0) {
        DIR *d = opendir(auth_path);
        if (d == NULL) {
            if (mkdir(auth_path, 0744) != 0) {
                char buffer[BUFSIZE];
                snprintf(buffer, BUFSIZE, "Error creating %s, ID: %s", auth_path, strerror(errno));
                write_log(buffer);
                free_auth();
                return 0;
            }
            d = opendir(auth_path);
        }
        FILE *fp = fopen(passwd_path, "w");
        if (fp == NULL) {
            char buffer[BUFSIZE];
            snprintf(buffer, BUFSIZE, "Error creating %s, ID: %s", passwd_path, strerror(errno));
            write_log(buffer);
            closedir(d);
            free_auth();
            return 0;
        }
        fclose(fp);
        closedir(d);

        char *hash = prepare_creds("admin", "admin");

        if (!add_user(hash, PRIV_ADMIN)) {
            char buffer[BUFSIZE];
            snprintf(buffer, BUFSIZE, "Could Not Create File With Default Credentials. ID: %s\n", strerror(errno));
            write_log(buffer);
            free(hash);
            free_auth();
            return 0;
        }
        free(hash);
    }

    return 1;
}

void free_auth() {
    free(passwd_path);
    free(auth_path);
}

int rm_user(char *name_hash) {
    // TODO: Implement.
}

int change_privileges(char *hash, int privileges) {
    // TODO: Implement.
}