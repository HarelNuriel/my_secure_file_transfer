//
// Created by Harel on 3/4/2026.
//
#pragma once

#ifndef SECURE_FILE_TRANSFER_CLIENT_H
#define SECURE_FILE_TRANSFER_CLIENT_H

#include "common_utils.h"

#define BUFSIZE 1024
#define IP "127.0.0.1\0"
#define PORT 1234

void client(char *ip, int port);

#endif //SECURE_FILE_TRANSFER_CLIENT_H