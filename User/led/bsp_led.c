/*
***********************************************************************************
*文件名：bsp_led.c
*作者：QFB
*版本：V1.0
*日期：2020-12-13
*说明：LED控制底层函数实现
*平台：野火M3开发板
***********************************************************************************
*/

#include "stm32f10x.h"    //实际bsp_led.h中已包含stm32f10x.h头文件，此行可以注释掉
#include "bsp_led.h"

void LED_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructrue;
    
    RCC_APB2PeriphClockCmd(LED1_GPIO_CLK | LED2_GPIO_CLK | LED3_GPIO_CLK, ENABLE);
    
    /*此处配置好GPIO_Mode后给void GPIO_Init(GPIO_TypeDef* GPIOx, GPIO_InitTypeDef* GPIO_InitStruct)调用配置CRL/CRH寄存器*/
    GPIO_InitStructrue.GPIO_Mode = GPIO_Mode_Out_PP;
    /*此处配置好GPIO_Speed后给void GPIO_Init(GPIO_TypeDef* GPIOx, GPIO_InitTypeDef* GPIO_InitStruct)调用配置CRL/CRH寄存器*/
    GPIO_InitStructrue.GPIO_Speed = GPIO_Speed_50MHz;   
    
    /*LED1 GPIO初始化*/
    GPIO_InitStructrue.GPIO_Pin = LED1_GPIO_PIN;
    GPIO_Init(LED1_GPIO_PORT,&GPIO_InitStructrue);  //GPIO_Init之前必须全部配置好引脚、模式、速度
    
    /*LED2 GPIO初始化*/
    GPIO_InitStructrue.GPIO_Pin = LED2_GPIO_PIN;    
    GPIO_Init(LED2_GPIO_PORT,&GPIO_InitStructrue);
    
    /*LED3 GPIO初始化*/
    GPIO_InitStructrue.GPIO_Pin = LED3_GPIO_PIN;    
    GPIO_Init(LED3_GPIO_PORT,&GPIO_InitStructrue);
    
    /*关闭全部LED灯*/
    GPIO_SetBits(LED1_GPIO_PORT,LED1_GPIO_PIN);    
    GPIO_SetBits(LED2_GPIO_PORT,LED2_GPIO_PIN);    
    GPIO_SetBits(LED3_GPIO_PORT,LED3_GPIO_PIN);
    
}


/*********************************************END OF FILE****************************************/
