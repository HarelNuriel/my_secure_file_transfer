//
// Created by Harel on 3/15/26.
//

#include "sha256.h"

const unsigned int init_vars[8] = {
    0x6a09e667,
    0xbb67ae85,
    0x3c6ef372,
    0xa54ff53a,
    0x510e527f,
    0x9b05688c,
    0x1f83d9ab,
    0x5be0cd19
};

const unsigned int k[SHA256_SIZE] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

unsigned int right_rot(const unsigned int num, const int rot_val) {
    return (num >> rot_val) | (num << (32 - rot_val));
}

char* format(unsigned int h[]) {
    char *hash = malloc(sizeof(char) * SHA256_SIZE + 1), *temp = hash;

    for (int i = 0; i < 8; i++) {
        sprintf(temp, "%08x", h[i]);
        temp += 8;
    }

    return hash;
}

unsigned char* padding(const char *str, size_t *len) {
    if (strlen(str) % SHA256_SIZE < 56) {
        *len = (strlen(str) / SHA256_SIZE + 1) * SHA256_SIZE;
    } else {
        *len = (strlen(str) / SHA256_SIZE + 2) * SHA256_SIZE;
    }

    unsigned char *padded_str = malloc(sizeof(char) * (*len)), *temp = padded_str;
    memset(padded_str, 0, *len);

    memcpy(padded_str, str, strlen(str));
    padded_str[strlen(str)] = 128;

    const unsigned long str_len = strlen(str) * 8;
    temp += *len - 8;
    for (int i = 0; i < 8; i++) {
        temp[i] = (str_len >> ((7 - i) * 8)) & 0xff;
    }

    return padded_str;
}

void schedule(const unsigned char *chunk, unsigned int w[]) {
    for (int i = 0; i < 16; i++) {
        w[i] =  (unsigned int)chunk[i * 4 + 3] |
                (unsigned int)chunk[i * 4 + 2] << 8 |
                (unsigned int)chunk[i * 4 + 1] << 16 |
                (unsigned int)chunk[i * 4] << 24;
    }

    unsigned int s0 = 0, s1 = 0;
    for (int i = 16; i < 64; i++) {
        s0 = right_rot(w[i - 15], 7) ^ right_rot(w[i - 15], 18) ^ w[i - 15] >> 3;
        s1 = right_rot(w[i - 2], 17) ^ right_rot(w[i - 2], 19) ^ w[i - 2] >> 10;
        w[i] = w[i - 16] + s0 + w[i - 7] + s1;
    }
}

void compression(unsigned int h[], const unsigned int w[]) {
    unsigned int s1 = 0, ch = 0, temp1 = 0, s0 = 0, maj = 0, temp2 = 0;

    for (int i = 0; i < 64; i++) {
        s1 = right_rot(h[4], 6) ^ right_rot(h[4], 11) ^ right_rot(h[4], 25);
        ch = (h[4] & h[5]) ^ ((~ h[4]) & h[6]);
        temp1 = h[7] + s1 + ch + k[i] + w[i];
        s0 = right_rot(h[0], 2) ^ right_rot(h[0], 13) ^ right_rot(h[0], 22);
        maj = (h[0] & h[1]) ^ (h[0] & h[2]) ^ (h[1] & h[2]);
        temp2 = s0 + maj;

        h[7] = h[6];
        h[6] = h[5];
        h[5] = h[4];
        h[4] = h[3] + temp1;
        h[3] = h[2];
        h[2] = h[1];
        h[1] = h[0];
        h[0] = temp1 + temp2;
    }
}

char* sha256(const char *plain_text) {
    size_t len;
    unsigned char *padded_str = padding(plain_text, &len), *temp = padded_str;
    unsigned int *h = malloc(sizeof(int) * 8), working_vars[8],  w[SHA256_SIZE];
    for (int i = 0; i < 8; i++) {
            h[i] = init_vars[i];
    }

    for (int i = 0; i < len / SHA256_SIZE; i++) {
        for (int j = 0; j < 8; j++) {
            working_vars[j] = h[j];
        }
        schedule(temp, w);
        temp += SHA256_SIZE;
        compression(working_vars, w);
        for (int j = 0; j < 8; j++) {
            h[j] += working_vars[j];
        }
    }

    char *hash = format(h);

    free(h);
    free(padded_str);
    return hash;
}