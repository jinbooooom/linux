#ifndef __JINBO_MD5_HPP__
#define __JINBO_MD5_HPP__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

void md5(const uint8_t *initial_msg, size_t initial_len, uint8_t *digest);

void md5str(uint8_t *result, char *resultstr);

#endif