#pragma once

#include <stdint.h>
#include <stdint.h>
#include <Esent.h>

typedef unsigned short crc16_t;
/*
*���������� ����������� ����� �� ������ buff, len - ����� ������
*/
uint_least32_t Crc32(const WCHAR *buf, size_t len);