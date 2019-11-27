#pragma once

#include <stdint.h>

typedef unsigned short crc16_t;

unsigned short Crc16(unsigned char * pcBlock, unsigned short len, crc16_t crc_init);

uint_least32_t Crc32(unsigned char *buf, size_t len, uint_least32_t crc_init);