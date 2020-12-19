#ifndef __LED_H
#define __LED_H

#include "stm32f10x.h"

#define LED1_GPIO_PORT    GPIOC
#define LED1_GPIO_CLK     RCC_APB2Periph_GPIOC
#define LED1_GPIO_PIN     GPIO_Pin_3

#define LED2_GPIO_PORT    GPIOC
#define LED2_GPIO_CLK     RCC_APB2Periph_GPIOC
#define LED2_GPIO_PIN     GPIO_Pin_4

#define LED3_GPIO_PORT    GPIOC
#define LED3_GPIO_CLK     RCC_APB2Periph_GPIOC
#define LED3_GPIO_PIN     GPIO_Pin_5

#define ON   0      //低电平灯亮
#define OFF  1      //高电平灯灭


/*使用库函数控制IO，利用带参宏的方式*/
#define LED1(a)      if(a) GPIO_SetBits(LED1_GPIO_PORT,LED1_GPIO_PIN);\
                    else GPIO_ResetBits(LED1_GPIO_PORT,LED1_GPIO_PIN)
                        
#define LED2(a)      if(a) GPIO_SetBits(LED2_GPIO_PORT,LED2_GPIO_PIN);\
                    else GPIO_ResetBits(LED2_GPIO_PORT,LED2_GPIO_PIN)
                        
#define LED3(a)      if(a) GPIO_SetBits(LED3_GPIO_PORT,LED3_GPIO_PIN);\
                    else GPIO_ResetBits(LED3_GPIO_PORT,LED3_GPIO_PIN)
                        
/*直接操作寄存器控制IO，实际和库函数实现方法一样*/      
#define digitHi(p,i)            {p->BSRR = i;} 
#define digitLo(p,i)            {p->BRR = i;} 
#define digitToggle(p,i)        {p->ODR ^= i;} 
                    
#define LED1_ON                 digitLo(LED1_GPIO_PORT,LED1_GPIO_PIN)
#define LED1_OFF                digitHi(LED1_GPIO_PORT,LED1_GPIO_PIN)
#define LED1_TOGGLE             digitToggle(LED1_GPIO_PORT,LED1_GPIO_PIN)

#define LED2_ON                 digitLo(LED2_GPIO_PORT,LED2_GPIO_PIN)
#define LED2_OFF                digitHi(LED2_GPIO_PORT,LED2_GPIO_PIN)
#define LED2_TOGGLE             digitToggle(LED2_GPIO_PORT,LED2_GPIO_PIN)

#define LED3_ON                 digitLo(LED3_GPIO_PORT,LED3_GPIO_PIN)
#define LED3_OFF                digitHi(LED3_GPIO_PORT,LED3_GPIO_PIN)
#define LED3_TOGGLE             digitToggle(LED3_GPIO_PORT,LED3_GPIO_PIN)

/*在头文件中声明函数方便移植*/
void LED_GPIO_Config(void);

#endif

/*********************************************END OF FILE**********************/
   

