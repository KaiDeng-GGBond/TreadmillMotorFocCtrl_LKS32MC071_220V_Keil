#include "uart.h"
#include "lks32mc07x_uart.h"
#include "lks32mc07x_dma.h"
#include <stdarg.h>
#include <string.h>
#include <stdio.h>


/************************************ UART发送 ******************************************/
/**
  * @brief		功能描述:   UART1发送数据
  * @param		data：要发生1Byte数据
  * @example	uart_1_send(0x12);	//串口0发送0x12一字节数据
  * @retval		none
  */
void uart_1_send_byte(uint8_t data)
{
    //等待发送缓冲区空（硬件状态）
    while (!(UART1->STT & BIT0));
	UART_SendData(UART1 , data);
}

void uart_1_send_string(char *str)
{
    while (*str)
    {
        uart_1_send_byte(*str++);
    }
}

// 发送指定长度数据
void uart_1_send_buffer(uint8_t *buf, uint16_t len)
{
    while (len--)
    {
        uart_1_send_byte(*buf++);
    }
}

void uart_1_printf(const char *fmt, ...)
{
    char buf[128];   // 视情况调大
    va_list args;

    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    uart_1_send_string(buf);
}

/************************************ UART DMA发送 ******************************************/
#define UART1_TX_DMA_CHANNEL          DMA_CHN0
#define UART1_TX_DMA_IRQ_FLAG         DMA_IF_CH0
#define UART1_TX_DMA_BUFFER_SIZE      (256u)

static volatile uint8_t g_uart1_dma_tx_busy = 0;
static uint8_t g_uart1_dma_tx_buffer[UART1_TX_DMA_BUFFER_SIZE];

void uart_1_dma_init(void)
{
    DMA_InitTypeDef DMA_InitStruct;

    DMA_StructInit(&DMA_InitStruct);
    DMA_InitStruct.EN = ENABLE;
    DMA_InitStruct.IRQ_EN = ENABLE;
    DMA_InitStruct.SBTW = DMA_BYTE_TRANS;
    DMA_InitStruct.DBTW = DMA_BYTE_TRANS;
    DMA_InitStruct.SINC = ENABLE;
    DMA_InitStruct.DINC = DISABLE;
    DMA_InitStruct.CIRC = DISABLE;
    DMA_InitStruct.RMODE = ENABLE; /* UART按“每次请求搬1字节”工作 */
    DMA_InitStruct.TMS = 1;
    DMA_InitStruct.SADR = (u32)g_uart1_dma_tx_buffer;
    DMA_InitStruct.DADR = (u32)&UART1->BUFF;
    DMA_InitStruct.REN.B.UART1_TX_EN = ENABLE;
    DMA_Init(UART1_TX_DMA_CHANNEL, &DMA_InitStruct);

    DMA_ClearIRQFlag(UART1_TX_DMA_IRQ_FLAG);
    UART1->RE = TX_BUF_DMA_RE;
    g_uart1_dma_tx_busy = 0;
}

uint8_t uart_1_dma_tx_is_busy(void)
{
    return g_uart1_dma_tx_busy;
}

void uart_1_dma_tx_irq_handler(void)
{
    if (DMA_GetIRQFlag(UART1_TX_DMA_IRQ_FLAG))
    {
        DMA_ClearIRQFlag(UART1_TX_DMA_IRQ_FLAG);
        DMA_CHx_EN(UART1_TX_DMA_CHANNEL, DISABLE);
        g_uart1_dma_tx_busy = 0;
    }
}

int uart_1_send_buffer_dma(const uint8_t *buf, uint16_t len)
{
    if ((buf == NULL) || (len == 0u) || (len > UART1_TX_DMA_BUFFER_SIZE))
    {
        return -1;
    }

    /* 上位机数据链路采用“忙则丢帧”的策略，避免串口发送反向阻塞控制任务。 */
    if (g_uart1_dma_tx_busy != 0u)
    {
        return 0;
    }

    memcpy(g_uart1_dma_tx_buffer, buf, len);

    if (len == 1u)
    {
        while (!(UART1->STT & BIT0))
        {
        }
        UART_SendData(UART1, g_uart1_dma_tx_buffer[0]);
        return 1;
    }

    g_uart1_dma_tx_busy = 1u;

    /* 先手动塞入第1字节，保证UART立刻起发；
     * 剩余字节由“发送缓冲区空”的DMA请求继续搬运。 */
    while (!(UART1->STT & BIT0))
    {
    }
    UART_SendData(UART1, g_uart1_dma_tx_buffer[0]);

    DMA_CHx_EN(UART1_TX_DMA_CHANNEL, DISABLE);
    DMA_ClearIRQFlag(UART1_TX_DMA_IRQ_FLAG);
    UART1_TX_DMA_CHANNEL->SADR = (u32)&g_uart1_dma_tx_buffer[1];
    UART1_TX_DMA_CHANNEL->DADR = (u32)&UART1->BUFF;
    do {
        UART1_TX_DMA_CHANNEL->CTMS = (u32)(len - 1u);
    } while (UART1_TX_DMA_CHANNEL->CTMS != (u32)(len - 1u));
    UART1_TX_DMA_CHANNEL->REN = DMA_REQ_UART1_TX;
    UART1->RE = TX_BUF_DMA_RE;
    DMA_CHx_EN(UART1_TX_DMA_CHANNEL, ENABLE);

    return len;
}

#if CERRENT_LOOP_BAND_WIDTH_TEST
#include "foc.h"
extern BW_TEST cur_bw_test;
/************************************ UART 发送电流环带宽测试数据 ******************************************/
void uart_send_bw_test_data(void)
{
    uart_1_send_string("\r\n========================================\r\n");
    uart_1_send_string("       Current Loop BW Test Data\r\n");
    uart_1_send_string("========================================\r\n");

    uart_1_printf("Total Points: %d\r\n\r\n", cur_bw_test.index);

    for(uint16_t i = 0; i < cur_bw_test.index; i++)
    {
        uart_1_printf("[%03d] Ref=%d, Fb=%d\r\n",
                      i,
                      cur_bw_test.ref[i],
                      cur_bw_test.fb[i]);
    }

    uart_1_send_string("========================================\r\n");
}

void uart_send_bw_test_data_csv(void)
{
    uart_1_send_string("\r\nIndex,Ref,Fb\r\n");

    for(uint16_t i = 0; i < cur_bw_test.index; i++)
    {
        uart_1_printf("%u;%d;%d;\r\n",
                      i,
                      cur_bw_test.ref[i],
                      cur_bw_test.fb[i]);
    }
}
#endif

#if IF_OPENLOOP
/************************************ UART 发送hall学习数据 ******************************************/

extern uint8_t gOpenLoopHallBuffer[OPENLOOP_BUFFER_SIZE][6];	// Hall值环形缓冲区 (每组6个Hall值)
extern uint16_t gOpenLoopAngleBuffer[OPENLOOP_BUFFER_SIZE][6];	// 角度环形缓冲区 (每组6个角度值)
extern uint8_t gBufferWritePointer;		// 写指针
extern uint16_t gAngleTemp[6];			//临时存角度
extern uint8_t gHallTemp[6];				//临时存hall值
extern uint8_t gOpenLoopStudyPointer;		//临时存放的指针
extern uint8_t gBufferValidGroups;			// 有效数据组数
extern uint8_t gBufferOverflowFlag;		// 是否发生过溢出（环形覆盖）
extern uint8_t gTrig7;


// 在你的 uart.c 文件中添加以下代码

// 声明外部变量（在 Hall.c 或其他文件中定义）
extern uint8_t gOpenLoopHallBuffer[OPENLOOP_BUFFER_SIZE][6];
extern uint16_t gOpenLoopAngleBuffer[OPENLOOP_BUFFER_SIZE][6];
extern uint8_t gBufferValidGroups;
extern uint8_t gBufferOverflowFlag;
extern uint8_t gTrig7;  // 触发发送标志

/**
  * @brief  发送Hall自学习数据（文本格式）
  */
void uart_send_hall_learning_data(void)
{
    if(gBufferValidGroups == 0)
    {
        uart_1_send_string("\r\nNo Hall learning data available!\r\n");
        return;
    }
    
    uart_1_send_string("\r\n========================================\r\n");
    uart_1_send_string("       Hall Learning Data Report\r\n");
    uart_1_send_string("========================================\r\n");
    uart_1_printf("Total Groups: %d\r\n", gBufferValidGroups);
    uart_1_printf("Buffer Overflow: %d\r\n\r\n", gBufferOverflowFlag);
    
    for(uint8_t group = 0; group < gBufferValidGroups; group++)
    {
        uart_1_printf("Group[%d]:\r\n", group);
        
        for(uint8_t point = 0; point < 6; point++)
        {
            uart_1_printf("  Point%d: Hall=0x%02X, Angle=%d\r\n", 
                         point, 
                         gOpenLoopHallBuffer[group][point],
                         gOpenLoopAngleBuffer[group][point]);
        }
        uart_1_send_string("\r\n");
    }
    
    uart_1_send_string("========================================\r\n");
}

/**
  * @brief  以二进制格式发送Hall数据（适合上位机解析）
  */
void uart_send_hall_learning_data_binary(void)
{
    if(gBufferValidGroups == 0) return;
    
    // 发送帧头 (4字节)
    uint8_t header[4];
    header[0] = 0xAA;
    header[1] = 0x55;
    header[2] = 0x01;  // 命令字：Hall数据
    header[3] = gBufferValidGroups;  // 数据组数
    uart_1_send_buffer_dma(header, 4);
    
    // 发送每组数据
    for(uint8_t group = 0; group < gBufferValidGroups; group++)
    {
        // 发送6个Hall值
        uart_1_send_buffer_dma(&gOpenLoopHallBuffer[group][0], 6);
        
        // 发送6个角度值（低字节在前）
        for(uint8_t i = 0; i < 6; i++)
        {
            uint16_t angle = gOpenLoopAngleBuffer[group][i];
            uint8_t angle_bytes[2];
            angle_bytes[0] = angle & 0xFF;
            angle_bytes[1] = (angle >> 8) & 0xFF;
            uart_1_send_buffer_dma(angle_bytes, 2);
        }
    }
    
    // 发送帧尾
    uint8_t footer[2] = {0x0D, 0x0A};
    uart_1_send_buffer_dma(footer, 2);
}

/**
  * @brief  发送CSV格式数据（可用Excel打开）
  */
void uart_send_hall_learning_data_csv(void)
{
    if(gBufferValidGroups == 0) return;
    
    // 发送CSV表头
    uart_1_send_string("Group,Hall0,Hall1,Hall2,Hall3,Hall4,Hall5,Angle0,Angle1,Angle2,Angle3,Angle4,Angle5\r\n");
    
    for(uint8_t group = 0; group < gBufferValidGroups; group++)
    {
        uart_1_printf("%d", group);
        
        // 发送Hall值
        for(uint8_t i = 0; i < 6; i++)
        {
            uart_1_printf(",%d", gOpenLoopHallBuffer[group][i]);
        }
        
        // 发送角度值
        for(uint8_t i = 0; i < 6; i++)
        {
            uart_1_printf(",%d", gOpenLoopAngleBuffer[group][i]);
        }
        
        uart_1_send_string("\r\n");
    }
}

/**
  * @brief  紧凑格式发送（一行一组数据，用空格分隔）
  */
void uart_send_hall_learning_data_compact(void)
{
    if(gBufferValidGroups == 0)
    {
        uart_1_send_string("NO_DATA\r\n");
        return;
    }
    
    uart_1_printf("GROUPS=%d\r\n", gBufferValidGroups);
    
    for(uint8_t group = 0; group < gBufferValidGroups; group++)
    {
        // 发送组号
        uart_1_printf("%d:", group);
        
        // 发送6个Hall值
        for(uint8_t i = 0; i < 6; i++)
        {
            uart_1_printf(" %d", gOpenLoopHallBuffer[group][i]);
        }
        
        // 发送6个角度值
        for(uint8_t i = 0; i < 6; i++)
        {
            uart_1_printf(" %d", gOpenLoopAngleBuffer[group][i]);
        }
        
        uart_1_send_string("\r\n");
    }
}

/**
  * @brief  调试用：只发送角度数据（验证用）
  */
void uart_send_angles_only(void)
{
    if(gBufferValidGroups == 0) return;
    
    uart_1_send_string("\r\n=== Angles Data ===\r\n");
    
    for(uint8_t group = 0; group < gBufferValidGroups; group++)
    {
        uart_1_printf("Group%d: ", group);
        for(uint8_t i = 0; i < 6; i++)
        {
            uart_1_printf("%d ", gOpenLoopAngleBuffer[group][i]);
        }
        uart_1_send_string("\r\n");
    }
}
#endif
