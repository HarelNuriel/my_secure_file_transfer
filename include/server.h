//
// Created by Harel on 3/4/2026.
//
#pragma once

#ifndef SECURE_FILE_TRANSFER_SERVER_H
#define SECURE_FILE_TRANSFER_SERVER_H

#include <stdio.h>
#include <winsock2.h>
#include "common_utils.h"
#include <windows.h>

#define BUFSIZE 1024
#define IP "127.0.0.1\0"
#define PORT 1234
#define MIN_FILES 16

void server(int port);

#endif //SECURE_FILE_TRANSFER_SERVER_H