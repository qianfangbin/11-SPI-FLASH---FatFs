#ifndef __BSP_EEPROM_H
#define __BSP_EEPROM_H

#include "stm32f10x.h"
#include "bsp_usart.h"
// #include "bsp_i2c.h"
#include "stdio.h"


//I2C接口宏定义
//GPIO宏定义
#define I2C_SCL_GPIO_PORT              GPIOB
#define I2C_SCL_GPIO_PIN               GPIO_Pin_6
#define I2C_SDA_GPIO_PORT              GPIOB
#define I2C_SDA_GPIO_PIN               GPIO_Pin_7

#define I2C_GPIO_CLK                  RCC_APB2Periph_GPIOB
#define I2C_GPIO_APBPERIPHCLOCKCMD    RCC_APB2PeriphClockCmd

//外设宏定义
#define I2C_EEPROM                    I2C1
#define I2C_AFIO_CLK                  RCC_APB1Periph_I2C1
#define I2C_AFIO_APBPERIPHCLOCKCMD    RCC_APB1PeriphClockCmd

#define I2C_SPEED                     400000    //400Khz,实测800K也可以正常工作
#define I2C_OWN_ADDRESS_7             0x0A     //随意定义，不可与device地址相同


//AT24C01/02每页有8个字节
#define EEPROM_PAGE_SIZE        8

// AT24C04/08/16每页有16个字节
// #define EEPROM_PAGE_SIZE        16

//AT24C01/02器件地址为1010 000
#define EEPROM_DEVICE_ADDRESS   0xA0

#define USER_I2C_FLAG_TIMEOUT        ((uint32_t)0x1000)         //实际调试该值只要大于15，IIC基本都可以正常工作
#define USER_I2C_LONG_TIMEOUT        ((uint32_t)(USER_I2C_FLAG_TIMEOUT * 10))  //带参宏建议加（）

#define DEBUG_ON   0
// #define DEBUG_ON   1

#define EEPROM_INFO(fmt,arg...)    printf("EEPROM INFO: "fmt" \n", ##arg)
#define EEPROM_ERROR(fmt,arg...)    printf("EEPROM ERROR: "fmt" \n", ##arg)

#define EEPROM_DEBUG(fmt,arg,...)   do\
                                    {if(DEBUG_ON) \
                                    printf("EEPROM DEBUG: [%d] "fmt" \n", _LINE_, ##arg);\
                                    }while(0)


//按字节写入
uint32_t EEPROM_ByteWrite(uint8_t* pBuffer, uint8_t WriteAddr);

//按页写入，每页8个字节
uint32_t EEPROM_PageWrite(uint8_t* pBuffer, uint8_t WriteAddr, uint8_t NumByteToWrite);

//把缓冲区数据一次写入
void EEPROM_BufferWrite(uint8_t* pBuffer, uint8_t WriteAddr, uint16_t NumByteToWrite);

//把EEPROM中内容读取到缓冲区中
uint32_t EEPROM_BufferRead(uint8_t* pBuffer, uint8_t ReadAddr, uint16_t NumByteToRead);

//等待EEPROM内部擦写完成
void EEPROM_WaitStandbyState(void);


void I2C_EEPROM_Init(void);


#endif

/*********************************************END OF FILE**********************/
   

