#pragma once

#include <stdint.h>
#include <stdint.h>
#include <Esent.h>

typedef unsigned short crc16_t;
/*
*Вычисление контрольной суммы из буфера buff, len - длина буфера
*/
uint_least32_t Crc32(const WCHAR *buf, size_t len);