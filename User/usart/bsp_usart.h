#ifndef __BSP_USART_H
#define __BSP_USART_H

#include "stm32f10x.h"
#include "stdio.h"

//����1-USART1
//���ź궨��
#define USART_TX_GPIO_PORT              GPIOA
#define USART_TX_GPIO_PIN               GPIO_Pin_9
#define USART_RX_GPIO_PORT              GPIOA
#define USART_RX_GPIO_PIN               GPIO_Pin_10

#define USART_GPIO_CLK                  RCC_APB2Periph_GPIOA
#define USART_GPIO_APBPERIPHCLOCKCMD    RCC_APB2PeriphClockCmd

//��������궨��
#define USART                           USART1
#define USART_AFIO_CLK                  RCC_APB2Periph_USART1  //����1����ʹ�õ���APB2ʱ��
#define USART_AFIO_APBPERIPHCLOCKCMD    RCC_APB2PeriphClockCmd
#define USART_BAUDRATE                  115200


// //�����жϺ궨��
// #define USART_IRQ                       USART1_IRQn
// #define USART_IRQHANDLER                USART1_IRQHandler


//����2-USART2
//���Ҫʹ�ô���2������ʹ�����º궨��
//���ź궨��
// #define USART_TX_GPIO_PORT              GPIOA
// #define USART_TX_GPIO_PIN               GPIO_Pin_2
// #define USART_RX_GPIO_PORT              GPIOA
// #define USART_RX_GPIO_PIN               GPIO_Pin_3

// #define USART_GPIO_CLK                  RCC_APB2Periph_GPIOA
// #define USART_GPIO_APBPERIPHCLOCKCMD    RCC_APB2PeriphClockCmd

// //��������궨��
// #define USART                           USART1
// #define USART_AFIO_CLK                  RCC_APB1Periph_USART2   //����2��3��4��5����ʹ�õ���APB1ʱ��
// #define USART_AFIO_APBPERIPHCLOCKCMD    RCC_APB1PeriphClockCmd
// #define USART_BAUDRATE                  115200

// //�����жϺ궨��
// #define USART_IRQ                       USART2_IRQn
// #define USART_IRQHANDLER                USART2_IRQHandler



void USART_Config(void);

void USART_SendByte(USART_TypeDef *pUSARTx, uint8_t ch);
void USART_SendString(USART_TypeDef *pUSARTx, char *str);
void USART_SendHalfWord(USART_TypeDef *pUSARTx, uint16_t ch);

#endif

/*********************************************END OF FILE**********************/
   

