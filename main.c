#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "client.h"
#include "server.h"

char *handle_flags(int *port, char **argv, const int argc) {
    char *ip = NULL;

    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0) {
            if (i + 1 < argc) {
                *port = (int)strtol(argv[i + 1], NULL, 10);
                i++;
            } else {
                printf("Error: -p flag requires a port number.\n");
            }
        }
        else if (strcmp(argv[i], "-t") == 0) {
            if (i + 1 < argc) {
                ip = malloc(sizeof(char) * (strlen(argv[i + 1]) + 1));
                if (ip != NULL) {
                    strcpy(ip, argv[i + 1]);
                }
                i++;
            } else {
                printf("Error: -t flag requires an IP address.\n");
            }
        }
    }

    return ip;
}

int main(const int argc, char **argv) {
    char *help = "Secure File Transfer:\nUsage: sft [mode] [flags]."
                 "\n\nModes:\n\tserver\tSetup a sft server.\n\tclient\tSetup a sft client."
                 "\n\nFlags:\n\t-help [-h]\tPrint Help Message.\n\t-p\tPort Number to listen/connect to (default 1234)."
                 "\n\t-t\tIP address to connect to (default 127.0.0.1).\n";

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
    char *ip = handle_flags(&port, argv, argc);

    if (strcmp(argv[1], "server") == 0) {
        server(ip, port);
    } else if (strcmp(argv[1], "client") == 0) {
        client(ip, port);
    } else {
        printf("Error: Invalid mode.\n\n%s", help);
    }

    return 0;
}