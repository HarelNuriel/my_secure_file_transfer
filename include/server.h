//
// Created by Harel on 3/4/2026.
//
#pragma once

#ifndef SECURE_FILE_TRANSFER_SERVER_H
#define SECURE_FILE_TRANSFER_SERVER_H

#include "auth.h"
#include "common_utils.h"

#define MIN_FILES 16
#define LOG_PATH "/var/log/server/"
#define LOG_FILE "server.log"

void server(char *ip, int port, char *data_path);

#endif // SECURE_FILE_TRANSFER_SERVER_H
