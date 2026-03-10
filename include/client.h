//
// Created by Harel on 3/4/2026.
//
#pragma once

#ifndef SECURE_FILE_TRANSFER_CLIENT_H
#define SECURE_FILE_TRANSFER_CLIENT_H

#include "common_utils.h"

void client(char *ip, int port);

static char log_msg[BUFSIZE] = {0};
static char buffer[BUFSIZE] = {0};

#endif //SECURE_FILE_TRANSFER_CLIENT_H