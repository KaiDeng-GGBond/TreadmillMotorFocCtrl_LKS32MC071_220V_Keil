#ifndef __VOFA_H
#define __VOFA_H

#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>

void vofa_firewater_printf(const char *fmt, ...);
int vofa_justfloat_printf(const float* channel_data, uint8_t channel_count, int (*send_bytes_function)(const uint8_t*, uint16_t));
int vofa_justfloat(const uint8_t* data, uint16_t length);
void vofa_pack_and_send(void);


#endif
