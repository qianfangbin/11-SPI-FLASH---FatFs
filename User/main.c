/*
***********************************************************************************
*�ļ�����main.c
*���ߣ�QFB
*�汾��V1.0
*���ڣ�2020-12-19
*˵����SPI FLASH��д���ԣ�Flash�ͺţ�W25X16
*ƽ̨��Ұ��M3������
***********************************************************************************
*/

#include "stm32f10x.h"    //ʵ��bsp_led.h���Ѱ���stm32f10x.hͷ�ļ������п���ע�͵�
#include "bsp_led.h"
#include "bsp_usart.h"
#include "bsp_spi_flash.h"

//�궨��
#define WRITE_ADDRESS           0x000000
#define READ_ADDRESS            0x000000
#define SECTOR_ERASE_ADDRESS    0x000000


//�����������
uint8_t Buf_Write[256] = "C���Գ�����ƣ��ִ����� �ڶ��� ���ߣ�K.N.KING ";
uint8_t Buf_Read[256];

//��������
uint8_t SPI_FLASH_Test(void);
void Delay(__IO uint32_t nCount);


int main(void)
{
    __IO uint32_t JEDECDeviceID = 0, DeviceID = 0;

    LED_GPIO_Config();
    LED2_ON;
    
    USART_Config();
    printf("����һ��SPI����W25X16��д����!\n");
    
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
    //д֮ǰ��Ҫ�Ȳ������������һ��������4KB��Զ����Ҫд���256�ֽ�
    SPI_FLASH_SectorErase(SECTOR_ERASE_ADDRESS);

    printf("д�������:\n");
    printf("%s \n",Buf_Write);
   

    SPI_FLASH_BufferWrite(Buf_Write, WRITE_ADDRESS, 256);

    INFO(" д�ɹ�\n");
    
    SPI_FLASH_BufferRead(Buf_Read, READ_ADDRESS, 256);

    printf("����������:\n");    
    printf("%s \n", Buf_Read);
    

    INFO(" SPI FLASH W25X16��д���Գɹ�!\n");

    return 1;
}


void Delay(__IO uint32_t nCount)
{
  for(; nCount != 0; nCount--);
  
}

/*********************************************END OF FILE**********************/
