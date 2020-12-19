/*
***********************************************************************************
*文件名：bsp_usart.c
*作者：QFB
*版本：V1.0
*日期：2020-12-17
*说明：串口USART初始化以及收发实现
*平台：野火M3开发板
***********************************************************************************
*/

#include "stm32f10x.h"    //实际bsp_led.h中已包含stm32f10x.h头文件，此行可以注释掉
#include "bsp_usart.h"

//初始化NVIC为中断做准备,配置好串口中断的优先级
// void NVIC_Configuration(void)
// {
//     NVIC_InitTypeDef NVIC_InitStructure;

//     NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

//     NVIC_InitStructure.NVIC_IRQChannel = USART_IRQ;
//     NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
//     NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
//     NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

//     NVIC_Init(&NVIC_InitStructure);

// }


void USART_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructrue;

    USART_InitTypeDef USART_InitStructure;

    USART_GPIO_APBPERIPHCLOCKCMD(USART_GPIO_CLK, ENABLE);

    USART_AFIO_APBPERIPHCLOCKCMD(USART_AFIO_CLK, ENABLE);

    /*TX GPIO初始化*/
    GPIO_InitStructrue.GPIO_Pin = USART_TX_GPIO_PIN;
    GPIO_InitStructrue.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructrue.GPIO_Speed = GPIO_Speed_50MHz; 
    GPIO_Init(USART_TX_GPIO_PORT,&GPIO_InitStructrue);  
    
    /*RX GPIO初始化*/
    GPIO_InitStructrue.GPIO_Pin = USART_RX_GPIO_PIN;
    GPIO_InitStructrue.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(USART_RX_GPIO_PORT,&GPIO_InitStructrue);  
    
    //UASART外设初始化
    USART_InitStructure.USART_BaudRate = USART_BAUDRATE;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(USART,&USART_InitStructure);
    
    // //配置好串口中断优先级
    // NVIC_Configuration();

    // //使能串口中断
    // USART_ITConfig(USART, USART_IT_RXNE, ENABLE);

    //使能串口
    USART_Cmd(USART,ENABLE);
    
}

//重定向C库函数printf到串口，实现串口发送
int fputc(int ch, FILE *f)                  //FILE关键字是定义在stdio.h中
{
    //发送一个字节数据到串口
    USART_SendData(USART,(uint8_t) ch);
    //等待串口发送寄存器为空，即表示完成
    while(USART_GetFlagStatus(USART,USART_FLAG_TXE) == RESET)
        ;
    return (ch);

}

//重定向C库函数scanf、getchar到串口，实现串口接收,建议使用getchar，实时性更高
int fgetc(FILE *f)
{   
    //等待串口接收数据
    while(USART_GetFlagStatus(USART, USART_FLAG_RXNE) == RESET)
        ;
    return (int)USART_ReceiveData(USART);
}

//发送一个字符
void USART_SendByte(USART_TypeDef *pUSARTx, uint8_t ch)
{
    USART_SendData(pUSARTx,ch);

    while(USART_GetFlagStatus(USART, USART_FLAG_TXE) == RESET)
        ;
}

//发送字符串
void USART_SendString(USART_TypeDef *pUSARTx, char *str)
{
    uint8_t k = 0;
    do
    {
        USART_SendByte(pUSARTx, *(str + k ));
        k++;

    } while (*(str + k ) != '\0');
    
    while(USART_GetFlagStatus(USART,USART_FLAG_TC) == RESET)
        ;
}


//发送一个16位数
void USART_SendHalfWord(USART_TypeDef *pUSARTx, uint16_t ch)
{
    uint8_t temp_h, temp_l;
    //取出高8位
    temp_h = (ch & 0xff00) >> 8;
    //取出低8位
    temp_l = ch & 0x00ff;
    //先发送低8位
    USART_SendData(pUSARTx,temp_l);
    while(USART_GetFlagStatus(USART, USART_FLAG_TXE) == RESET)
        ;
    //再发送高8位
    USART_SendData(pUSARTx,temp_h);
    while(USART_GetFlagStatus(USART, USART_FLAG_TXE) == RESET)
        ;
}


/*********************************************END OF FILE****************************************/
