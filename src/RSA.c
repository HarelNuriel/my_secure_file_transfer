//
// Created by Harel on 3/19/26.
//

#include "RSA.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int is_msb_set(const uint32_t x) {
    const int pos = sizeof(int) * 8 - 1;
    const unsigned int mask = 1 << pos;

    if (x & mask) {
        return 1;
    }

    return 0;
}

unsigned long long modular_exp(const uint64_t base, uint64_t exp,
                               const uint64_t mod) {
    if (mod == 1) {
        return 0;
    }

    __int128 result = 1, ext_base = base;
    while (exp > 0) {
        if (exp % 2 == 1) {
            result = (result * ext_base) % mod;
        }
        exp = exp >> 1;
        ext_base = (ext_base * ext_base) % mod;
    }

    return result;
}

uint64_t gcd(uint64_t x, uint64_t y) {
    if (x == 0)
        return y;
    if (y == 0)
        return x;

    int d = 0;
    while ((x & 1) == 0 && (y & 1) == 0) {
        if ((x & 1) == 0) {
            x = x >> 1;
        }
        if ((y & 1) == 0) {
            y = y >> 1;
        }
        d++;
    }

    while ((x & 1) == 0)
        x = x >> 1;
    while ((y & 1) == 0)
        y = y >> 1;

    while (x != y) {
        if (x > y) {
            x = x - y;
            while ((x & 1) == 0)
                x = x >> 1;
        } else {
            y = y - x;
            while ((y & 1) == 0)
                y = y >> 1;
        }
    }

    return ((uint64_t)1 << d) * x;
}

int rm_primality_test(const uint32_t buffer) {
    if ((buffer & 1) == 0) {
        return 0;
    }

    unsigned int k = 10, s = 0, found = 0;
    unsigned int d = 0;
    do {
        s++;
        d = (int)((buffer - 1) >> s);
        if ((d & 1) == 1) {
            found = 1;
        }
        if (d == 0) {
            return 0;
        }
    } while (!found);

    unsigned int a = 0;
    unsigned long long y = 0, x = 0;
    for (int i = 0; i < k; i++) {
        getrandom(&a, sizeof(int), 0);
        a = a % (buffer - 3) + 2;
        x = modular_exp(a, d, buffer);

        for (int j = 0; j < s; j++) {
            y = (x * x) % buffer;
            if (y == 1 && x != 1 && x != buffer - 1) {
                return 0;
            }
            x = y;
        }

        if (y != 1) {
            return 0;
        }
    }

    return 1;
}

ssize_t generate_prime(uint32_t *buffer) {
    ssize_t size = 0;
    int flag = 0;

    while (!flag) {
        size = getrandom(buffer, sizeof(int), 0);
        if (is_msb_set(*buffer)) {
            flag = rm_primality_test(*buffer);
        }
    }

    return size;
}

uint64_t euclidean_division(const uint64_t dividend, const uint64_t divisor) {
    return dividend / divisor;
}

uint64_t extended_gcd(const uint64_t x, const uint64_t y) {
    uint64_t old_r = x, r = y, r_temp = 0;
    long long old_s = 1, s = 0, s_temp = 0;
    long long quotient = 0;

    while (r != 0) {
        quotient = (long long)euclidean_division(old_r, r);
        r_temp = r;
        r = old_r - (quotient * r);
        old_r = r_temp;
        s_temp = s;
        s = old_s - (quotient * s);
        old_s = s_temp;
    }

    if (old_s < 0)
        old_s = y + old_s;
    return old_s;
}

void generate_keys(struct rsa_param *params) {
    generate_prime(&params->p);
    generate_prime(&params->q);
    uint64_t totient = (uint64_t)(params->q - 1) * (uint64_t)(params->p - 1);
    params->e = 17;

    while (gcd(totient, params->e) != 1) {
        generate_prime(&params->p);
        generate_prime(&params->q);
        totient = (uint64_t)(params->q - 1) * (uint64_t)(params->p - 1);
    }

    params->n = (uint64_t)params->q * (uint64_t)params->p;
    params->d = extended_gcd(params->e, totient);
}

struct enc_msg encrypt(char *message, ssize_t size, const struct key_pair key) {
    struct enc_msg msg;
    int block_bytes = RSA_SIZE / 8;
    const long len = (size + block_bytes - 1) / block_bytes;
    uint32_t *m;
    uint64_t *result = malloc(sizeof(uint64_t) * len);

    msg.len = len * sizeof(uint64_t);
    msg.msg = malloc(sizeof(char) * msg.len);

    if (size > RSA_SIZE / 8) {
        m = (uint32_t *)message;
        for (int i = 0; size > 0; i++) {
            uint32_t block = 0;
            if (size >= block_bytes) {
                block = *m;
            } else {
                memcpy(&block, m, size);
            }
            result[i] = modular_exp(block, key.exp, key.n);
            size -= RSA_SIZE / 8;
            m++;
        }
        memcpy(msg.msg, (char *)result, msg.len);
        free(result);
        return msg;
    }

    result[0] = modular_exp(*(uint32_t *)message, key.exp, key.n);
    memcpy(msg.msg, (char *)result, msg.len);

    free(result);
    return msg;
}

char *decrypt(char *message, ssize_t size, const struct key_pair key) {
    const long len = size / (RSA_SIZE / 4) + 1;
    uint64_t *m;
    uint32_t *result = malloc(sizeof(uint32_t) * len);

    if (size > RSA_SIZE / 4) {
        m = (uint64_t *)message;
        for (int i = 0; size > 0; i++) {
            result[i] = modular_exp(*m, key.exp, key.n);
            size -= RSA_SIZE / 4;
            m++;
        }
        return (char *)result;
    }

    result[0] = modular_exp(*(uint64_t *)message, key.exp, key.n);
    return (char *)result;
}
