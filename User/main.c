/*
***********************************************************************************
*�ļ�����main.c
*���ߣ�QFB
*�汾��V1.0
*���ڣ�2020-12-20
*˵����SPI FLASHʹ��FatFs��д���ԣ�Flash�ͺţ�W25X16
*ƽ̨��Ұ��M3������
***********************************************************************************
*/

#include "stm32f10x.h"    //ʵ��bsp_led.h���Ѱ���stm32f10x.hͷ�ļ������п���ע�͵�
#include "bsp_led.h"
#include "bsp_usart.h"
#include "bsp_spi_flash.h"
#include "ff.h"

FATFS fs;
FIL fnew;
FRESULT res_flash;
UINT fnum;
BYTE ReadBuffer[1024] = {0};
BYTE WriteBuffer[] = "--����һ��STM32 SPI FLASH ʹ��FatFs�ļ�ϵͳ��д���ԣ�SPI FLASH �ͺţ�W25X16JVSSIG --";


int main(void)
{
    LED_GPIO_Config();
    LED2_ON;
    USART_Config();
    printf("****** ����һ��SPI FLASH �ļ�ϵͳʵ�� ******\r\n");
    res_flash = f_mount(&fs, "1:", 1);
    
    if(res_flash == FR_NO_FILESYSTEM)
    {
        printf("\r\n--FLASH ��û���ļ�ϵͳ��������ʽ����\r\n");
        
        res_flash = f_mkfs("1:0", 0, 0);

        if(res_flash == FR_OK)
        {
            printf("\r\n--FLASH �Ѹ�ʽ���ɹ���\r\n");
            res_flash = f_mount(NULL, "1:", 1);
            res_flash = f_mount(&fs, "1:", 1);
        }
        else
        {
            LED1_ON;
            printf("\r\n--��ʽ��ʧ�ܣ�\r\n");
        }
        
    }
    else if(res_flash != FR_OK)
    {
        printf("\r\n--SPI FLASH�����ļ�ϵͳʧ�� %d \r\n", res_flash);
        printf("\r\n--����ԭ��SPI FLASH ��ʼ��ʧ�ܣ�\r\n");
        while(1);
    }
    else
    {
        printf("\r\n--SPI FLASH�����ļ�ϵͳ�ɹ������Խ��ж�д���� \r\n");
    }



    //д����
    printf("\r\n--���������ļ�д����� \r\n");

    res_flash = f_open(&fnew, "1:testfile.txt",FA_CREATE_ALWAYS | FA_WRITE );

    if(res_flash == FR_OK)
    {
        printf("\r\n--��/����testfile.txt�ɹ������ļ�д��������... \r\n");

        res_flash = f_write(&fnew, WriteBuffer, sizeof(WriteBuffer), &fnum);

        if(res_flash == FR_OK)
        {
            printf("\r\n--�ļ�д��ɹ���д���ֽ�����%d \r\n", fnum);
            printf("\r\n--д������ݣ�\r\n %s \r\n", WriteBuffer);
        }
        else
        {
            printf("\r\n--�ļ�д��ʧ�� %d \r\n", res_flash);
        }

        f_close(&fnew);
    }
    else
    {
        LED1_ON;
         printf("\r\n--��/�����ļ�ʧ�� ��\r\n");
    }


    //������
	printf("\r\n-- ���������ļ���ȡ����... \r\n");
	res_flash = f_open(&fnew, "1:testfile.txt",FA_OPEN_EXISTING | FA_READ); 	 
	if(res_flash == FR_OK)
	{
		LED3_ON;
		printf("\r\n--���ļ��ɹ���\r\n");
		res_flash = f_read(&fnew, ReadBuffer, sizeof(ReadBuffer), &fnum); 
    if(res_flash==FR_OK)
    {
      printf("\r\n--�ļ���ȡ�ɹ�,�����ֽ����ݣ�%d\r\n",fnum);
      printf("\r\n--��ȡ�õ��ļ�����Ϊ��\r\n%s \r\n", ReadBuffer);	
    }
    else
    {
      printf("\r\n �ļ���ȡʧ�ܣ�(%d)\n",res_flash);
    }		
	}
	else
	{
		LED1_ON;
		printf("\r\n�������ļ�ʧ�ܡ�\r\n");
	}
	/* ���ٶ�д���ر��ļ� */
	f_close(&fnew);	
  
	/* ����ʹ���ļ�ϵͳ��ȡ�������ļ�ϵͳ */
	f_mount(NULL,"1:",1);
  
  /* ������ɣ�ͣ�� */
	while(1)
	{
	}    

}



/*********************************************END OF FILE**********************/
