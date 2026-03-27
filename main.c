#include "client.h"
#include "server.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *data_path;

char *handle_flags(FILE **log_stream, int *port, char **argv, const int argc) {
    char *ip = NULL;

    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0) {
            if (i + 1 < argc) {
                *port = (int)strtol(argv[i + 1], NULL, 10);
                i++;
            } else {
                printf("Error: -p flag requires a port number.\n");
            }
        } else if (strcmp(argv[i], "-t") == 0) {
            if (i + 1 < argc) {
                ip = malloc(sizeof(char) * (strlen(argv[i + 1]) + 1));
                if (ip != NULL) {
                    strcpy(ip, argv[i + 1]);
                }
                i++;
            } else {
                printf("Error: -t flag requires an IP address.\n");
            }
        } else if (strcmp(argv[i], "-l") == 0) {
            if (i + 1 < argc) {
                *log_stream = fopen(argv[i + 1], "w");
                set_log_stream(*log_stream);
            }
            i++;
        } else if (strncmp(argv[i], "-s", 2) == 0) {
            if (i + 1 < argc) {
                size_t len = strlen(argv[i + 1]) + 1;
                data_path = malloc(sizeof(*data_path) * len);
                snprintf(data_path, len, "%s", argv[i + 1]);
            }
            i++;
        }
    }

    return ip;
}

int main(const int argc, char **argv) {
    char *help =
        "Secure File Transfer:\nUsage: sft [mode] [flags]."
        "\n\nModes:\n\tserver\t\tSetup a sft server.\n\tclient\t\tSetup a sft "
        "client."
        "\n\nFlags:\n\t-help [-h]\t\tPrint help message."
        "\n\t-p [port number]\tPort number to listen/connect to (default 1234)."
        "\n\t-t [target address]\tIP address to connect to (default 127.0.0.1)."
        "\n\t-l [Log File]\t\tLog file to write logs into."
        "\n\t-s [save path]\t\tDefines where the server saves its files.";

    // Handle empty execution
    if (argc < 2) {
        printf("%s", help);
        return 0;
    }

    // Handle top-level help request
    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "-help") == 0) {
        printf("%s", help);
        return 0;
    }

    int port = 0;
    FILE *log_stream = NULL;
    char *ip = handle_flags(&log_stream, &port, argv, argc);

    if (strcmp(argv[1], "server") == 0) {
        server(ip, port, data_path);
    } else if (strcmp(argv[1], "client") == 0) {
        client(ip, port);
    } else {
        printf("Error: Invalid mode.\n\n%s", help);
    }

    if (log_stream != NULL) {
        fclose(log_stream);
    }

    return 0;
}
