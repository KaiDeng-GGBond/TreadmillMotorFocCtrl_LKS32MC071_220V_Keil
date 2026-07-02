#ifndef UART_H
#define UART_H

#include <stdint.h>
#include "sysdef.h"

void uart_1_send_byte(uint8_t data);
void uart_1_send_string(char *str);
void uart_1_send_buffer(uint8_t *buf, uint16_t len);
void uart_1_printf(const char *fmt, ...);
void uart_1_dma_init(void);
void uart_1_dma_tx_irq_handler(void);
int uart_1_send_buffer_dma(const uint8_t *buf, uint16_t len);

#if CERRENT_LOOP_BAND_WIDTH_TEST
/************************************ UART 发送电流环带宽测试数据 ******************************************/
void uart_send_bw_test_data(void);
void uart_send_bw_test_data_csv(void);
#endif

#if IF_OPENLOOP
/************************************ UART 发送hall学习数据 ******************************************/
void uart_send_hall_learning_data(void);
void uart_send_hall_learning_data_binary(void);
void uart_send_hall_learning_data_csv(void);
void uart_send_hall_learning_data_compact(void);
void uart_send_angles_only(void);
#endif
#endif
