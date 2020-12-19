/*
***********************************************************************************
*文件名：main.c
*作者：QFB
*版本：V1.0
*日期：2020-12-19
*说明：SPI FLASH读写测试，Flash型号：W25X16
*平台：野火M3开发板
***********************************************************************************
*/

#include "stm32f10x.h"    //实际bsp_led.h中已包含stm32f10x.h头文件，此行可以注释掉
#include "bsp_led.h"
#include "bsp_usart.h"
#include "bsp_spi_flash.h"

//宏定义
#define WRITE_ADDRESS           0x000000
#define READ_ADDRESS            0x000000
#define SECTOR_ERASE_ADDRESS    0x000000


//数组变量定义
uint8_t Buf_Write[256] = "C语言程序设计：现代方法 第二版 作者：K.N.KING ";
uint8_t Buf_Read[256];

//函数声明
uint8_t SPI_FLASH_Test(void);
void Delay(__IO uint32_t nCount);


int main(void)
{
    __IO uint32_t JEDECDeviceID = 0, DeviceID = 0;

    LED_GPIO_Config();
    LED2_ON;
    
    USART_Config();
    printf("这是一个SPI外设W25X16读写测试!\n");
    
    SPI_FLASH_Init();

    JEDECDeviceID = SPI_FLASH_ReadJedecDeviceID();
    
    Delay(200);
    
    DeviceID = SPI_FLASH_ReadDeviceID();
    
    printf("JEDECDeviceID = %X DeviceID = %X \n", JEDECDeviceID, DeviceID);

    if(SPI_FLASH_Test() == 1)
    {
        LED1_ON;
    }
    else
    {
        LED1_OFF;
    }

    while(1);    
	
}


uint8_t SPI_FLASH_Test(void)
{
    //写之前需要先擦除，这里擦除一个扇区有4KB，远大于要写入的256字节
    SPI_FLASH_SectorErase(SECTOR_ERASE_ADDRESS);

    printf("写入的数据:\n");
    printf("%s \n",Buf_Write);
   

    SPI_FLASH_BufferWrite(Buf_Write, WRITE_ADDRESS, 256);

    INFO(" 写成功\n");
    
    SPI_FLASH_BufferRead(Buf_Read, READ_ADDRESS, 256);

    printf("读出的数据:\n");    
    printf("%s \n", Buf_Read);
    

    INFO(" SPI FLASH W25X16读写测试成功!\n");

    return 1;
}


void Delay(__IO uint32_t nCount)
{
  for(; nCount != 0; nCount--);
  
}

/*********************************************END OF FILE**********************/
