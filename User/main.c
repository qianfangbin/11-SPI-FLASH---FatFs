/*
***********************************************************************************
*文件名：main.c
*作者：QFB
*版本：V1.0
*日期：2020-12-20
*说明：SPI FLASH使用FatFs读写测试，Flash型号：W25X16
*平台：野火M3开发板
***********************************************************************************
*/

#include "stm32f10x.h"    //实际bsp_led.h中已包含stm32f10x.h头文件，此行可以注释掉
#include "bsp_led.h"
#include "bsp_usart.h"
#include "bsp_spi_flash.h"
#include "ff.h"

FATFS fs;
FIL fnew;
FRESULT res_flash;
UINT fnum;
BYTE ReadBuffer[1024] = {0};
BYTE WriteBuffer[] = "--这是一个STM32 SPI FLASH 使用FatFs文件系统读写测试，SPI FLASH 型号：W25X16JVSSIG --";


int main(void)
{
    LED_GPIO_Config();
    LED2_ON;
    USART_Config();
    printf("****** 这是一个SPI FLASH 文件系统实验 ******\r\n");
    res_flash = f_mount(&fs, "1:", 1);
    
    if(res_flash == FR_NO_FILESYSTEM)
    {
        printf("\r\n--FLASH 还没有文件系统，即将格式化！\r\n");
        
        res_flash = f_mkfs("1:0", 0, 0);

        if(res_flash == FR_OK)
        {
            printf("\r\n--FLASH 已格式化成功！\r\n");
            res_flash = f_mount(NULL, "1:", 1);
            res_flash = f_mount(&fs, "1:", 1);
        }
        else
        {
            LED1_ON;
            printf("\r\n--格式化失败！\r\n");
        }
        
    }
    else if(res_flash != FR_OK)
    {
        printf("\r\n--SPI FLASH挂载文件系统失败 %d \r\n", res_flash);
        printf("\r\n--可能原因：SPI FLASH 初始化失败！\r\n");
        while(1);
    }
    else
    {
        printf("\r\n--SPI FLASH挂载文件系统成功，可以进行读写测试 \r\n");
    }



    //写测试
    printf("\r\n--即将进行文件写入测试 \r\n");

    res_flash = f_open(&fnew, "1:testfile.txt",FA_CREATE_ALWAYS | FA_WRITE );

    if(res_flash == FR_OK)
    {
        printf("\r\n--打开/创建testfile.txt成功，向文件写入数据中... \r\n");

        res_flash = f_write(&fnew, WriteBuffer, sizeof(WriteBuffer), &fnum);

        if(res_flash == FR_OK)
        {
            printf("\r\n--文件写入成功，写入字节数：%d \r\n", fnum);
            printf("\r\n--写入的数据：\r\n %s \r\n", WriteBuffer);
        }
        else
        {
            printf("\r\n--文件写入失败 %d \r\n", res_flash);
        }

        f_close(&fnew);
    }
    else
    {
        LED1_ON;
         printf("\r\n--打开/创建文件失败 ！\r\n");
    }


    //读测试
	printf("\r\n-- 即将进行文件读取测试... \r\n");
	res_flash = f_open(&fnew, "1:testfile.txt",FA_OPEN_EXISTING | FA_READ); 	 
	if(res_flash == FR_OK)
	{
		LED3_ON;
		printf("\r\n--打开文件成功。\r\n");
		res_flash = f_read(&fnew, ReadBuffer, sizeof(ReadBuffer), &fnum); 
    if(res_flash==FR_OK)
    {
      printf("\r\n--文件读取成功,读到字节数据：%d\r\n",fnum);
      printf("\r\n--读取得的文件数据为：\r\n%s \r\n", ReadBuffer);	
    }
    else
    {
      printf("\r\n 文件读取失败：(%d)\n",res_flash);
    }		
	}
	else
	{
		LED1_ON;
		printf("\r\n！！打开文件失败。\r\n");
	}
	/* 不再读写，关闭文件 */
	f_close(&fnew);	
  
	/* 不再使用文件系统，取消挂载文件系统 */
	f_mount(NULL,"1:",1);
  
  /* 操作完成，停机 */
	while(1)
	{
	}    

}



/*********************************************END OF FILE**********************/
