// Created by Stefan Bauwens 2026
#ifndef RESENDER_H

#define RESENDER_H
#include <pebble.h>

typedef struct {
    uint32_t key;
    int value;
} KeyIntPair;

bool send_int(uint32_t key, int value);

void send_retrying_int(KeyIntPair * KeyIntPair);

#endif // RESENDER_H
