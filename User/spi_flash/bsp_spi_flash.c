/*
***********************************************************************************
*文件名：bsp_eeprom.c
*作者：QFB
*版本：V1.0
*日期：2020-12-17
*说明：硬件SPI接口读写FLASH函数实现
*平台：野火M3开发板
***********************************************************************************
*/

#include "stm32f10x.h"    //实际bsp_led.h中已包含stm32f10x.h头文件，此行可以注释掉
#include "bsp_spi_flash.h"


static __IO uint32_t SPI_FLASH_TimeOut = USER_SPI_LONG_TIMEOUT;


//GPIO和外设初始化
void SPI_FLASH_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructrue;
    SPI_InitTypeDef SPI_Initstructure;
    //使能GPIO的时钟
    SPI_CSN_GPIO_APBPERIPHCLOCKCMD(SPI_CSN_GPIO_CLK, ENABLE);   //由于SPI引脚都在PORT B，开启一次时钟即可
    //使能SPI外设时钟
    SPI_AFIO_APBPERIPHCLOCKCMD(SPI_AFIO_CLK, ENABLE);

      
    /*CLK GPIO配置，使用复用IO模式*/
    GPIO_InitStructrue.GPIO_Pin = SPI_CLK_GPIO_PIN;
    GPIO_InitStructrue.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructrue.GPIO_Speed = GPIO_Speed_50MHz; 
    GPIO_Init(SPI_CLK_GPIO_PORT,&GPIO_InitStructrue);  

    /*MISO GPIO配置，使用复用IO模式*/
    GPIO_InitStructrue.GPIO_Pin = SPI_MISO_GPIO_PIN;
    GPIO_Init(SPI_MISO_GPIO_PORT,&GPIO_InitStructrue);

    /*MOSI GPIO配置，使用复用IO模式*/
    GPIO_InitStructrue.GPIO_Pin = SPI_MOSI_GPIO_PIN;
    GPIO_Init(SPI_MOSI_GPIO_PORT,&GPIO_InitStructrue);

    /*CSN GPIO配置，使用普通IO模式*/
    GPIO_InitStructrue.GPIO_Pin = SPI_CSN_GPIO_PIN;
    GPIO_InitStructrue.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(SPI_CSN_GPIO_PORT,&GPIO_InitStructrue);
    //片选拉高
    SPI_CS_HIGH();  

    /*SPI外设配置*/
    SPI_Initstructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;  // SPI_CLK=72/4=18MHZ
    SPI_Initstructure.SPI_CPHA = SPI_CPHA_2Edge;
    SPI_Initstructure.SPI_CPOL = SPI_CPOL_High;
    SPI_Initstructure.SPI_CRCPolynomial = 7;    //程序并没有开启CRC，随意设置即可
    SPI_Initstructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_Initstructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_Initstructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_Initstructure.SPI_Mode = SPI_Mode_Master;
    SPI_Initstructure.SPI_NSS = SPI_NSS_Soft;

    SPI_Init(SPI_W25X, &SPI_Initstructure);

    SPI_Cmd(SPI_W25X, ENABLE);

}

//扇区擦除
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

//整片擦除
void SPI_FLASH_BulkErase(void)
{
    SPI_FLASH_WriteEnable();

    SPI_CS_LOW();

    SPI_FLASH_SendByte(W25_CMD_CHIP_ERASE);
    
    SPI_CS_HIGH();

    SPI_FLASH_WaitForWriteEnd();

}

//按页写入，每次写入的数据需要小于页大小256个字节数
//调用本函数之前先要擦除扇区
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

//连续写入多个数据
//调用本函数之前先要擦除扇区
void SPI_FLASH_BufferWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
    uint8_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;
    
    //写入地址和页地址偏移
    Addr = WriteAddr % W25_PAGE_SIZE;
    //需要补充的地址数量达到对齐页地址
    count = W25_PAGE_SIZE - Addr;

    NumOfPage = NumByteToWrite / W25_PAGE_SIZE;
    NumOfSingle = NumByteToWrite % W25_PAGE_SIZE;

    //如果写入地址和页地址对齐
    if(Addr == 0)
    {   
        //如果写入的字节数不足一页
        if(NumOfPage == 0)
        {
            SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumOfSingle);
            
        }
        //如果写入的字节数大于一页
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

    //如果写入地址和页地址没有对齐
    else
    {   
        //如果要写入的字节数不足一页
        if(NumOfPage == 0)
        {   
            //如果要写入的字节数大于待补充的地址数，那么多余的字节要写到下一页
            if(NumOfSingle > count)
            {   
                //先把前面一页的地址写入数据
                SPI_FLASH_PageWrite(pBuffer, WriteAddr, count);
                

                //再写入下一页
                temp = NumOfSingle - count;
                pBuffer += count;
                WriteAddr += count;
                
                SPI_FLASH_PageWrite(pBuffer, WriteAddr, temp);
                
            }
            //如果当前页剩下的地址空间足够写入字节数
            else
            {
                SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumOfSingle);
                
            }
        }
        //如果写入的字节数大于一页
        else
        {
            NumByteToWrite -= count;
            NumOfPage = NumByteToWrite / W25_PAGE_SIZE;
            NumOfSingle = NumByteToWrite % W25_PAGE_SIZE;

            //先把最前面count个地址写入数据，后续地址就和页地址对齐了
            if(count != 0)
            {
                //先把count个地址写入数据
                SPI_FLASH_PageWrite(pBuffer, WriteAddr, count);
                
                //记得地址要增加
                pBuffer += count;
                WriteAddr += count;
            }

            //把整页地址写入数据
            while(NumOfPage--)
            {
                SPI_FLASH_PageWrite(pBuffer, WriteAddr, W25_PAGE_SIZE);
                
                WriteAddr += W25_PAGE_SIZE;
                pBuffer += W25_PAGE_SIZE;
            }

            //如果还有多余的不满一页的数据，继续写入
            if(NumOfSingle != 0)
            {
                SPI_FLASH_PageWrite(pBuffer, WriteAddr, NumOfSingle);
                
            } 
        }
    } 
}



//一次连续读取多个数据
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


//读取SPI FLASH JEDEC DEVICE ID
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

//读取SPI FLASH DEVICE ID
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

//开始连续顺序读取FLASH,后面接读取赋值语句
void SPI_FLASH_StartReadSequence(uint32_t ReadAddr)
{
    SPI_CS_LOW();
    SPI_FLASH_SendByte(W25_CMD_READ_DATA);
    SPI_FLASH_SendByte((ReadAddr & 0xFF0000) >>16);
    SPI_FLASH_SendByte((ReadAddr & 0xFF00) >>8);
    SPI_FLASH_SendByte(ReadAddr & 0xFF);

}


//掉电模式
void SPI_FLASH_PowerDown(void)
{
    SPI_CS_LOW();
    SPI_FLASH_SendByte(W25_CMD_POWER_DOWN);
    SPI_CS_HIGH();

}


//唤醒
void SPI_FLASH_WakeUp(void)
{
    SPI_CS_LOW();
    SPI_FLASH_SendByte(W25_CMD_RELEASE_POWER_DOWN);
    SPI_CS_HIGH();

}

//读取一个字节数据
uint8_t SPI_FLASH_ReadByte(void)
{
    return (SPI_FLASH_SendByte(DUMMY_BYTE));

}

//发送一个字节数据
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

//发送2个字节数据
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
    ERROR("SPI等待超时，ERROR CODE : %d", ErrorCode);
    return 0;
}





/*********************************************END OF FILE****************************************/
