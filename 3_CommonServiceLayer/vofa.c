#include "vofa.h"
#include "uart.h"
#include "lks32mc07x_uart.h"
#include "lks32mc07x_mcpwm.h"
#include "hall.h"
#include "TaskScheduler.h"
#include "AdcCalc.h"
#include "foc.h"
#include "sysdef.h"

/* vofa两种数据协议的发送函数 */
/**
 * @brief 发送FireWater协议格式的数据（可变参数版本）
 * @param fmt 格式字符串，类似printf，但不需要手动加\n
 * @param ... 可变参数列表
 * @note 此函数自动在末尾添加换行符，符合FireWater协议要求
 */
void vofa_firewater_printf(const char *fmt, ...)
{
    char buffer[256];  // 可根据需要调整大小
    int len;
    
    // 创建可变参数列表
    va_list args;
    va_start(args, fmt);
    
    // 格式化字符串到缓冲区
    len = vsnprintf(buffer, sizeof(buffer) - 2, fmt, args);
    
    va_end(args);
    
    // 确保不超过缓冲区大小
    if (len < 0 || len >= (int)sizeof(buffer) - 2) {
        // 处理截断情况，可以添加错误处理
        len = sizeof(buffer) - 2;
    }
    
    // 添加FireWater协议要求的换行符
    buffer[len] = '\n';
    buffer[len + 1] = '\0';
    
    // 通过原有stm32f446_uart4_printf发送（需要确保它支持%s格式）
    uart_1_printf("%s", buffer);
}


/**
 * @brief 发送JustFloat协议格式的数据
 * @param channel_data 浮点型数据数组指针
 * @param channel_count 通道数量（数组长度）
 * @param send_bytes_function 底层字节发送函数指针
 * @return 成功发送返回1，失败返回0
 * @note 此函数假设系统为小端字节序（x86、ARM等常见平台均为小端）
示例：发送3个通道的数据
float sensor_data[3] = {1.23f, 4.56f, 7.89f};

假设的串口发送函数
int stm32f446_uart4_send_buffer(const uint8_t* data, uint16_t length) {
    // 这里调用你的具体串口发送函数，例如HAL_UART_Transmit
    for(int i = 0; i < length; i++) {
        // 发送data[i]到串口
    }
    return length;
}

void send_sensor_data(void) {
    send_justfloat(sensor_data, 3, stm32f446_uart4_send_buffer);	// 发送3个通道的数据
}
*/

static const uint8_t JUSTFLOAT_TAIL[4] = {0x00, 0x00, 0x80, 0x7F};/* JustFloat 协议帧尾定义 */

int vofa_justfloat_printf(const float* channel_data, uint8_t channel_count, int (*send_bytes_function)(const uint8_t*, uint16_t))
{
    /* 参数有效性检查 */
    if (channel_data == NULL || send_bytes_function == NULL || channel_count == 0) {
        return 0;
    }
    
    /* 计算总数据长度：通道数据 + 协议帧尾 */
    uint16_t data_length = channel_count * sizeof(float);
    uint16_t total_length = data_length + sizeof(JUSTFLOAT_TAIL);
    
    /* 构造发送缓冲区（避免修改原数据） */
    uint8_t send_buffer[total_length];
    
    /* 复制浮点数据到缓冲区（内存拷贝，保持二进制原样） */
    memcpy(send_buffer, channel_data, data_length);
    
    /* 添加JustFloat协议帧尾 */
    memcpy(send_buffer + data_length, JUSTFLOAT_TAIL, sizeof(JUSTFLOAT_TAIL));
    
    /* 通过用户提供的底层函数发送数据 */
    return send_bytes_function(send_buffer, total_length) > 0;
}

// 适配函数：将stm32f446_uart4_send_buffer包装成符合int (*)(const uint8_t*, uint16_t)的类型
int vofa_justfloat(const uint8_t* data, uint16_t length)
{
	return uart_1_send_buffer_dma(data, length);
}

#define UART_TX_BUF_SIZE (UART_TX_FLOAT_NUM * 4)
float vofa_justfloat_data[9] = {0};
extern void GPIO_ToggleP212(void);
void vofa_pack_and_send(void)
{
	vofa_justfloat_printf(vofa_justfloat_data , 5 , uart_1_send_buffer_dma);//uart_1_send_buffer_dma <->vofa_justfloat
}