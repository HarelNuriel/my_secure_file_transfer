#include <stdio.h>
#include <string.h>
#include "client.h"
#include "server.h"

void handle_flags(int *port, char *ip, char **argv, const int argc) {
    if (argc == 2) {
        return;
    }

    if (argc > 5) {
        if (strcmp(argv[4], "-t") == 0) {
            strcat(ip, argv[5]);
            *port = (int)strtol(argv[3], NULL, 10);
        } else {
            strcat(ip, argv[3]);
            *port = (int)strtol(argv[5], NULL, 10);
        }

        return;
    }

    if (argc == 4) {
        if (strcmp(argv[2], "-p") == 0) {
            *port = (int)strtol(argv[3], NULL, 10);
        } else {
            strcat(ip, argv[3]);
        }
    }
}

int main(const int argc, char **argv) {
    char *help = "Secure File Transfer:\nUsage: sft [mode] [flags]."
                 "\n\nModes:\n\tserver\tSetup a sft server.\n\tclient\tSetup a sft client."
                 "\n\nFlags:\n\t-help [-h]\tPrint Help Message.\n\t-p\tPort Number to listen/connect to (default 1234)."
                 "\n\t-t\tIP address to connect to (default 127.0.0.1).\0";
    if (argc == 0) {
        printf("%s", help);
        return 0;
    }
    int port = 0;
    char *ip = NULL;

    handle_flags(&port, ip, argv, argc);

    if (strcmp(argv[1], "server") == 0) {
        server(port);
    } else if (strcmp(argv[1], "client") == 0) {
        client(ip, port);
    } else {
        printf("Error: Invalid mode.\n\n%s", help);
    }

    return 0;
}