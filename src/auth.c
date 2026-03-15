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
    // TODO: Later on when hash is implemented replace length with a const value
    char *buffer = malloc(sizeof(char) * (strlen(name) + strlen(passwd) + 3));
    if (buffer == NULL) {
        char log[BUFSIZE];
        snprintf(log, BUFSIZE, "Couldn't Allocate Memory. ID: %s\n", strerror(errno));
        write_log(log);
        return 0;
    }

    // TODO: Later on when hash is implemented replace length with a const value
    strncpy(buffer, name, strlen(name) + 1);
    strcat(buffer, ":");
    strncat(buffer, passwd, strlen(passwd) + 1);
    strcat(buffer, "\n");

    return buffer;
}

int add_user(const char *name, const char *passwd) {
    if (name == NULL || passwd == NULL) {
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

    char *creds = prepare_creds(name, passwd);
    if (creds == NULL) {
        write_log("Error Preparing Credentials.");
        return 0;
    }

    fprintf(fp, "%s", creds);

    fclose(fp);
    free(creds);
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
    strcpy(passwd_path, auth_path);
    strcat(passwd_path, PASSWD_FILE);

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

        if (!add_user("admin", "admin")) {
            char buffer[BUFSIZE];
            snprintf(buffer, BUFSIZE, "Could Not Create File With Default Credentials. ID: %s\n", strerror(errno));
            write_log(buffer);
            free_auth();
            return 0;
        }
    }

    return 1;
}

void free_auth() {
    free(passwd_path);
    free(auth_path);
}

int validate_auth_user(const char *name, const char *passwd) {
    char *creds = prepare_creds(name, passwd);
    if (creds == NULL) {
        write_log("Error Preparing Credentials.");
        return 0;
    }

    FILE *fp = fopen(passwd_path, "r");
    if (fp == NULL) {
        char buffer[BUFSIZE];
        snprintf(buffer, BUFSIZE, "Error Opening %s, ID: %s", passwd_path, strerror(errno));
        write_log(buffer);
        free(creds);
        return 0;
    }

    char buffer[BUFSIZE];
    while (fgets(buffer, BUFSIZE, fp)) {
        // TODO: After implementing hash replace with strncmp
        if (strcmp(creds, buffer) == 0) {
            fclose(fp);
            free(creds);
            return VALID_CREDS;
        }
    }

    free(creds);
    fclose(fp);
    return INVALID_CREDS;
}