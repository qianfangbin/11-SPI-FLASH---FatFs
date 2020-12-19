/*
***********************************************************************************
*�ļ�����bsp_eeprom.c
*���ߣ�QFB
*�汾��V1.0
*���ڣ�2020-12-17
*˵����Ӳ��SPI�ӿڶ�дFLASH����ʵ��
*ƽ̨��Ұ��M3������
***********************************************************************************
*/

#include "stm32f10x.h"    //ʵ��bsp_led.h���Ѱ���stm32f10x.hͷ�ļ������п���ע�͵�
#include "bsp_spi_flash.h"


static __IO uint32_t SPI_FLASH_TimeOut = USER_SPI_LONG_TIMEOUT;


//GPIO�������ʼ��
void SPI_FLASH_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructrue;
    SPI_InitTypeDef SPI_Initstructure;
    //ʹ��GPIO��ʱ��
    SPI_CSN_GPIO_APBPERIPHCLOCKCMD(SPI_CSN_GPIO_CLK, ENABLE);   //����SPI���Ŷ���PORT B������һ��ʱ�Ӽ���
    //ʹ��SPI����ʱ��
    SPI_AFIO_APBPERIPHCLOCKCMD(SPI_AFIO_CLK, ENABLE);

      
    /*CLK GPIO���ã�ʹ�ø���IOģʽ*/
    GPIO_InitStructrue.GPIO_Pin = SPI_CLK_GPIO_PIN;
    GPIO_InitStructrue.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructrue.GPIO_Speed = GPIO_Speed_50MHz; 
    GPIO_Init(SPI_CLK_GPIO_PORT,&GPIO_InitStructrue);  

    /*MISO GPIO���ã�ʹ�ø���IOģʽ*/
    GPIO_InitStructrue.GPIO_Pin = SPI_MISO_GPIO_PIN;
    GPIO_Init(SPI_MISO_GPIO_PORT,&GPIO_InitStructrue);

    /*MOSI GPIO���ã�ʹ�ø���IOģʽ*/
    GPIO_InitStructrue.GPIO_Pin = SPI_MOSI_GPIO_PIN;
    GPIO_Init(SPI_MOSI_GPIO_PORT,&GPIO_InitStructrue);

    /*CSN GPIO���ã�ʹ����ͨIOģʽ*/
    GPIO_InitStructrue.GPIO_Pin = SPI_CSN_GPIO_PIN;
    GPIO_InitStructrue.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(SPI_CSN_GPIO_PORT,&GPIO_InitStructrue);
    //Ƭѡ����
    SPI_CS_HIGH();  

    /*SPI��������*/
    SPI_Initstructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;  // SPI_CLK=72/4=18MHZ
    SPI_Initstructure.SPI_CPHA = SPI_CPHA_2Edge;
    SPI_Initstructure.SPI_CPOL = SPI_CPOL_High;
    SPI_Initstructure.SPI_CRCPolynomial = 7;    //����û�п���CRC���������ü���
    SPI_Initstructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_Initstructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_Initstructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_Initstructure.SPI_Mode = SPI_Mode_Master;
    SPI_Initstructure.SPI_NSS = SPI_NSS_Soft;

    SPI_Init(SPI_W25X, &SPI_Initstructure);

    SPI_Cmd(SPI_W25X, ENABLE);

}

//��������
void SPI_FLASH_SectorErase(uint32_t SectorAddr)
{
    SPI_FLASH_WriteEnable();

    SPI_FLASH_WaitForWriteEnd();

    SPI_CS_LOW();

    SPI_FLASH_SendByte(W25_CMD_SECTOR_ERASE);

    SPI_FLASH_SendByte((SectorAddr & 0xFF0000) >>16);
    SPI_FLASH_SendByte((SectorAddr & 0xFF00) >>8);
    SPI_FLASH_SendByte(SectorAddr & 0xFF);

    SPI_CS_HIGH();

    SPI_FLASH_WaitForWriteEnd();

}

//��Ƭ����
void SPI_FLASH_BulkErase(void)
{
    SPI_FLASH_WriteEnable();

    SPI_CS_LOW();

    SPI_FLASH_SendByte(W25_CMD_CHIP_ERASE);
    
    SPI_CS_HIGH();

    SPI_FLASH_WaitForWriteEnd();

}

//��ҳд�룬ÿ��д���������ҪС��ҳ��С256���ֽ���
//���ñ�����֮ǰ��Ҫ��������
void SPI_FLASH_PageWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
    SPI_FLASH_WriteEnable();

    SPI_CS_LOW();

    SPI_FLASH_SendByte(W25_CMD_PAGE_PROGRAM);

    SPI_FLASH_SendByte((WriteAddr & 0xFF0000) >>16);
    SPI_FLASH_SendByte((WriteAddr & 0xFF00) >>8);
    SPI_FLASH_SendByte(WriteAddr & 0xFF);

    if(NumByteToWrite > W25_PAGE_SIZE)
    {
        NumByteToWrite = W25_PAGE_SIZE;
        ERROR(" SPI FLASH PAGE WRITE TOO LARGE!");
    }

    while (NumByteToWrite--)
    {
        SPI_FLASH_SendByte(*pBuffer);

        pBuffer++;
    }
    
    SPI_CS_HIGH();

    SPI_FLASH_WaitForWriteEnd();
}

//����д��������
//���ñ�����֮ǰ��Ҫ��������
void SPI_FLASH_BufferWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
    uint8_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;
    
    //д���ַ��ҳ��ַƫ��
    Addr = WriteAddr % W25_PAGE_SIZE;
    //��Ҫ����ĵ�ַ�����ﵽ����ҳ��ַ
    count = W25_PAGE_SIZE - Addr;

    NumOfPage = NumByteToWrite / W25_PAGE_SIZE;
    NumOfSingle = NumByteToWrite % W25_PAGE_SIZE;

    //���д���ַ��ҳ��ַ����
    if(Addr == 0)
    {   
        //���д����ֽ�������һҳ
        if(NumOfPage == 0)
        {
            SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumOfSingle);
            
        }
        //���д����ֽ�������һҳ
        else
        {   
            while(NumOfPage--)
            {
                SPI_FLASH_PageWrite(pBuffer, WriteAddr, W25_PAGE_SIZE);
                
                WriteAddr += W25_PAGE_SIZE;
                pBuffer += W25_PAGE_SIZE;
            }

            if(NumOfSingle != 0)
            {
                SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumOfSingle);
                
            }
        }
    }

    //���д���ַ��ҳ��ַû�ж���
    else
    {   
        //���Ҫд����ֽ�������һҳ
        if(NumOfPage == 0)
        {   
            //���Ҫд����ֽ������ڴ�����ĵ�ַ������ô������ֽ�Ҫд����һҳ
            if(NumOfSingle > count)
            {   
                //�Ȱ�ǰ��һҳ�ĵ�ַд������
                SPI_FLASH_PageWrite(pBuffer, WriteAddr, count);
                

                //��д����һҳ
                temp = NumOfSingle - count;
                pBuffer += count;
                WriteAddr += count;
                
                SPI_FLASH_PageWrite(pBuffer, WriteAddr, temp);
                
            }
            //�����ǰҳʣ�µĵ�ַ�ռ��㹻д���ֽ���
            else
            {
                SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumOfSingle);
                
            }
        }
        //���д����ֽ�������һҳ
        else
        {
            NumByteToWrite -= count;
            NumOfPage = NumByteToWrite / W25_PAGE_SIZE;
            NumOfSingle = NumByteToWrite % W25_PAGE_SIZE;

            //�Ȱ���ǰ��count����ַд�����ݣ�������ַ�ͺ�ҳ��ַ������
            if(count != 0)
            {
                //�Ȱ�count����ַд������
                SPI_FLASH_PageWrite(pBuffer, WriteAddr, count);
                
                //�ǵõ�ַҪ����
                pBuffer += count;
                WriteAddr += count;
            }

            //����ҳ��ַд������
            while(NumOfPage--)
            {
                SPI_FLASH_PageWrite(pBuffer, WriteAddr, W25_PAGE_SIZE);
                
                WriteAddr += W25_PAGE_SIZE;
                pBuffer += W25_PAGE_SIZE;
            }

            //������ж���Ĳ���һҳ�����ݣ�����д��
            if(NumOfSingle != 0)
            {
                SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumOfSingle);
                
            } 
        }
    } 
}



//һ��������ȡ�������
void SPI_FLASH_BufferRead(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead)
{
    SPI_CS_LOW();

    SPI_FLASH_SendByte(W25_CMD_READ_DATA);

    SPI_FLASH_SendByte((ReadAddr & 0xFF0000) >>16);
    SPI_FLASH_SendByte((ReadAddr & 0xFF00) >>8);
    SPI_FLASH_SendByte(ReadAddr & 0xFF);

    while(NumByteToRead--)
    {
        *pBuffer = SPI_FLASH_SendByte(DUMMY_BYTE);
        pBuffer++;
    }

    SPI_CS_HIGH();
}


//��ȡSPI FLASH JEDEC DEVICE ID
uint32_t SPI_FLASH_ReadJedecDeviceID(void)
{
    uint32_t temp=0, temp1=0, temp2=0, temp3=0;

    SPI_CS_LOW();

    SPI_FLASH_SendByte(W25_CMD_READ_JEDEC_DEVICE_ID);

    temp1 = SPI_FLASH_SendByte(DUMMY_BYTE);

    temp2 = SPI_FLASH_SendByte(DUMMY_BYTE);

    temp3 = SPI_FLASH_SendByte(DUMMY_BYTE);

    SPI_CS_HIGH();

    temp = (temp1 <<16) | (temp2 << 8) | temp3;

    return temp;

}

//��ȡSPI FLASH DEVICE ID
uint32_t SPI_FLASH_ReadDeviceID(void)
{
    uint32_t temp=0;

    SPI_CS_LOW();

    SPI_FLASH_SendByte(W25_CMD_READ_DEVICE_ID);

    SPI_FLASH_SendByte(DUMMY_BYTE);

    SPI_FLASH_SendByte(DUMMY_BYTE);

    SPI_FLASH_SendByte(DUMMY_BYTE);

    temp = SPI_FLASH_SendByte(DUMMY_BYTE);

    SPI_CS_HIGH();

    return temp;
}

//��ʼ����˳���ȡFLASH,����Ӷ�ȡ��ֵ���
void SPI_FLASH_StartReadSequence(uint32_t ReadAddr)
{
    SPI_CS_LOW();
    SPI_FLASH_SendByte(W25_CMD_READ_DATA);
    SPI_FLASH_SendByte((ReadAddr & 0xFF0000) >>16);
    SPI_FLASH_SendByte((ReadAddr & 0xFF00) >>8);
    SPI_FLASH_SendByte(ReadAddr & 0xFF);

}


//����ģʽ
void SPI_FLASH_PowerDown(void)
{
    SPI_CS_LOW();
    SPI_FLASH_SendByte(W25_CMD_POWER_DOWN);
    SPI_CS_HIGH();

}


//����
void SPI_FLASH_WakeUp(void)
{
    SPI_CS_LOW();
    SPI_FLASH_SendByte(W25_CMD_RELEASE_POWER_DOWN);
    SPI_CS_HIGH();

}

//��ȡһ���ֽ�����
uint8_t SPI_FLASH_ReadByte(void)
{
    return (SPI_FLASH_SendByte(DUMMY_BYTE));

}

//����һ���ֽ�����
uint8_t SPI_FLASH_SendByte(uint8_t Byte)
{
    SPI_FLASH_TimeOut = USER_SPI_FLAG_TIMEOUT;
    while (!SPI_I2S_GetFlagStatus(SPI_W25X, SPI_I2S_FLAG_TXE))
    {
        if((SPI_FLASH_TimeOut--) == 0)
            return SPI_TimeOut_UserCallBack(0);
    }

    SPI_I2S_SendData(SPI_W25X, Byte);

    SPI_FLASH_TimeOut = USER_SPI_FLAG_TIMEOUT;
    while(!SPI_I2S_GetFlagStatus(SPI_W25X, SPI_I2S_FLAG_RXNE))
    {
        if((SPI_FLASH_TimeOut--) == 0)
            return SPI_TimeOut_UserCallBack(1);
    }

    return SPI_I2S_ReceiveData(SPI_W25X);

}

//����2���ֽ�����
uint16_t SPI_FLASH_SendHalfWord(uint16_t HalfWord)
{
    SPI_FLASH_TimeOut = USER_SPI_FLAG_TIMEOUT;
    while (!SPI_I2S_GetFlagStatus(SPI_W25X, SPI_I2S_FLAG_TXE))
    {
        if((SPI_FLASH_TimeOut--) == 0)
            return SPI_TimeOut_UserCallBack(2);
    }

    SPI_I2S_SendData(SPI_W25X, HalfWord);

    SPI_FLASH_TimeOut = USER_SPI_FLAG_TIMEOUT;
    while(!SPI_I2S_GetFlagStatus(SPI_W25X, SPI_I2S_FLAG_RXNE))
    {
        if((SPI_FLASH_TimeOut--) == 0)
            return SPI_TimeOut_UserCallBack(3);
    }

    return SPI_I2S_ReceiveData(SPI_W25X);


}



void SPI_FLASH_WriteEnable(void)
{
    SPI_CS_LOW();

    SPI_FLASH_SendByte(W25_CMD_WRITE_ENABLE);

    SPI_CS_HIGH();

}


void SPI_FLASH_WaitForWriteEnd(void)
{
    uint8_t status;

    SPI_CS_LOW();

    SPI_FLASH_SendByte(W25_CMD_READ_STATUS_REG);

    do
    {
        status = SPI_FLASH_SendByte(DUMMY_BYTE);
    } while (status & WIP_FLAG);
    
    SPI_CS_HIGH();
}


uint32_t SPI_TimeOut_UserCallBack(uint8_t ErrorCode)
{
    ERROR("SPI�ȴ���ʱ��ERROR CODE : %d", ErrorCode);
    return 0;
}





/*********************************************END OF FILE****************************************/
