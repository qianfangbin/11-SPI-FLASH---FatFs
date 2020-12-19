/*
***********************************************************************************
*文件名：bsp_eeprom.c
*作者：QFB
*版本：V1.0
*日期：2020-12-17
*说明：硬件I2C接口读写EEPROM函数实现
*平台：野火M3开发板
***********************************************************************************
*/

#include "stm32f10x.h"    //实际bsp_led.h中已包含stm32f10x.h头文件，此行可以注释掉
#include "bsp_eeprom.h"


static __IO uint32_t I2C_TimeOut = USER_I2C_LONG_TIMEOUT;


//初始化配置硬件I2C
void I2C_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructrue;

    I2C_GPIO_APBPERIPHCLOCKCMD(I2C_GPIO_CLK, ENABLE);
    I2C_AFIO_APBPERIPHCLOCKCMD(I2C_AFIO_CLK, ENABLE);

    /*GPIO初始化*/
    GPIO_InitStructrue.GPIO_Pin = I2C_SCL_GPIO_PIN;
    GPIO_InitStructrue.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructrue.GPIO_Speed = GPIO_Speed_50MHz; 
    GPIO_Init(I2C_SCL_GPIO_PORT,&GPIO_InitStructrue);  
    
    /*GPIO初始化*/
    GPIO_InitStructrue.GPIO_Pin = I2C_SDA_GPIO_PIN;
    GPIO_InitStructrue.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructrue.GPIO_Speed = GPIO_Speed_50MHz; 
    GPIO_Init(I2C_SDA_GPIO_PORT,&GPIO_InitStructrue);  

}


void I2C_Mode_Config(void)
{
    //外设初始化
    I2C_InitTypeDef I2C_InitStructure;    
    
    I2C_InitStructure.I2C_ClockSpeed = I2C_SPEED;
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle =I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = I2C_OWN_ADDRESS_7;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init(I2C_EEPROM,&I2C_InitStructure);
    
    //使能外设
    I2C_Cmd(I2C_EEPROM,ENABLE);
}

void I2C_EEPROM_Init(void)
{
    I2C_GPIO_Config();
    I2C_Mode_Config();
}


//如果超时就打印超时的环节，用于调试
static uint32_t I2C_TimeOut_UserCallback(uint8_t ErrorCode)
{
    EEPROM_ERROR("I2C等待超时！错误代码: %d",ErrorCode);
    return 0;
}


//等待操作完成,因为要供本文件中其它函数调用，所以放在前面
void EEPROM_WaitStandbyState(void)
{
    __IO uint16_t SR1_Temp = 0;
    do
    {
        I2C_GenerateSTART(I2C_EEPROM, ENABLE);
        //读取SR1寄存器
        SR1_Temp = I2C_ReadRegister(I2C_EEPROM, I2C_Register_SR1);
        //发送EEPROM器件地址并配置为写
        I2C_Send7bitAddress(I2C_EEPROM, EEPROM_DEVICE_ADDRESS, I2C_Direction_Transmitter);
        //等待地址发送完成
    } while (!(I2C_ReadRegister(I2C_EEPROM, I2C_Register_SR1) & 0x0002));  //I2C_FLAG_ADDR = 1表示地址发送成功；0表示地址发送未完成
        
    I2C_ClearFlag(I2C_EEPROM, I2C_FLAG_AF);    //清除ack fail标志位
    I2C_GenerateSTOP(I2C_EEPROM,ENABLE);


}


//按字节写入
uint32_t EEPROM_ByteWrite(uint8_t* pBuffer, uint8_t WriteAddr)
{
    I2C_GenerateSTART(I2C_EEPROM, ENABLE);
    
    I2C_TimeOut = USER_I2C_FLAG_TIMEOUT;
    while(!I2C_CheckEvent(I2C_EEPROM, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if((I2C_TimeOut--) == 0)
            return I2C_TimeOut_UserCallback(0);
    }

    I2C_TimeOut = USER_I2C_FLAG_TIMEOUT;
    I2C_Send7bitAddress(I2C_EEPROM, EEPROM_DEVICE_ADDRESS, I2C_Direction_Transmitter);
    while(!I2C_CheckEvent(I2C_EEPROM, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    {
        if((I2C_TimeOut--) == 0)
            return I2C_TimeOut_UserCallback(1);
    }

    I2C_TimeOut = USER_I2C_FLAG_TIMEOUT;
    I2C_SendData(I2C_EEPROM,WriteAddr);
    while(!I2C_CheckEvent(I2C_EEPROM, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
        if((I2C_TimeOut--) == 0)
            return I2C_TimeOut_UserCallback(2);
    }

    I2C_TimeOut = USER_I2C_FLAG_TIMEOUT;
    I2C_SendData(I2C_EEPROM,*pBuffer);
    while(!I2C_CheckEvent(I2C_EEPROM, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
        if((I2C_TimeOut--) == 0)
            return I2C_TimeOut_UserCallback(3);
    }

    I2C_GenerateSTOP(I2C_EEPROM,ENABLE);

    return 1;

}

//按页写入，每次写入的字节数不能超过8（每页8个字节）
uint32_t EEPROM_PageWrite(uint8_t* pBuffer, uint8_t WriteAddr, uint8_t NumByteToWrite)
{
    I2C_TimeOut = USER_I2C_LONG_TIMEOUT;
    while(I2C_GetFlagStatus(I2C_EEPROM, I2C_FLAG_BUSY))
    {
        if((I2C_TimeOut--) == 0)
            return I2C_TimeOut_UserCallback(4);
    }
    //发送开始信号
    I2C_GenerateSTART(I2C_EEPROM, ENABLE);
    I2C_TimeOut = USER_I2C_FLAG_TIMEOUT;
    //检查EV5并清除事件
    while(!I2C_CheckEvent(I2C_EEPROM, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if((I2C_TimeOut--) == 0)
            return I2C_TimeOut_UserCallback(5);
    }
    //发送器件地址并配置为写
    I2C_Send7bitAddress(I2C_EEPROM, EEPROM_DEVICE_ADDRESS, I2C_Direction_Transmitter);
    I2C_TimeOut = USER_I2C_FLAG_TIMEOUT;
    //检查EV6并清除事件
    while(!I2C_CheckEvent(I2C_EEPROM, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    {
        if((I2C_TimeOut--) == 0)
            return I2C_TimeOut_UserCallback(6);
    }
    //发送器件内部地址用于写入
    I2C_SendData(I2C_EEPROM,WriteAddr);
    I2C_TimeOut = USER_I2C_FLAG_TIMEOUT;
    //检查EV8并清除事件
    while(!I2C_CheckEvent(I2C_EEPROM, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
        if((I2C_TimeOut--) == 0)
            return I2C_TimeOut_UserCallback(7);
    }

    while(NumByteToWrite--)
    {
        I2C_SendData(I2C_EEPROM,*pBuffer);
        pBuffer++;
        I2C_TimeOut = USER_I2C_FLAG_TIMEOUT;
        while(!I2C_CheckEvent(I2C_EEPROM, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
        {
            if((I2C_TimeOut--) == 0)
                return I2C_TimeOut_UserCallback(8);
        }
        
    }

    I2C_GenerateSTOP(I2C_EEPROM,ENABLE);

    return 1;

}

//把缓冲区数据一次写入
void EEPROM_BufferWrite(uint8_t* pBuffer, uint8_t WriteAddr, uint16_t NumByteToWrite)
{
    uint8_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;
    
    //写入地址和页地址偏移
    Addr = WriteAddr % EEPROM_PAGE_SIZE;
    //需要补充的地址数量达到对齐页地址
    count = EEPROM_PAGE_SIZE - Addr;

    NumOfPage = NumByteToWrite / EEPROM_PAGE_SIZE;
    NumOfSingle = NumByteToWrite % EEPROM_PAGE_SIZE;

    //如果写入地址和页地址对齐
    if(Addr == 0)
    {   
        //如果写入的字节数不足一页
        if(NumOfPage == 0)
        {
            EEPROM_PageWrite(pBuffer, WriteAddr, NumOfSingle);
            EEPROM_WaitStandbyState();
        }
        //如果写入的字节数大于一页
        else
        {   
            while(NumOfPage--)
            {
                EEPROM_PageWrite(pBuffer, WriteAddr, EEPROM_PAGE_SIZE);
                EEPROM_WaitStandbyState();
                WriteAddr += EEPROM_PAGE_SIZE;
                pBuffer += EEPROM_PAGE_SIZE;
            }

            if(NumOfSingle != 0)
            {
                EEPROM_PageWrite(pBuffer, WriteAddr, NumOfSingle);
                EEPROM_WaitStandbyState();
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
                EEPROM_PageWrite(pBuffer, WriteAddr, count);
                EEPROM_WaitStandbyState();

                //再写入下一页
                temp = NumOfSingle - count;
                pBuffer += count;
                WriteAddr += count;
                
                EEPROM_PageWrite(pBuffer, WriteAddr, temp);
                EEPROM_WaitStandbyState();
            }
            //如果当前页剩下的地址空间足够写入字节数
            else
            {
                EEPROM_PageWrite(pBuffer, WriteAddr, NumOfSingle);
                EEPROM_WaitStandbyState();
            }
        }
        //如果写入的字节数大于一页
        else
        {
            NumByteToWrite -= count;
            NumOfPage = NumByteToWrite / EEPROM_PAGE_SIZE;
            NumOfSingle = NumByteToWrite % EEPROM_PAGE_SIZE;

            //先把最前面count个地址写入数据，后续地址就和页地址对齐了
            if(count != 0)
            {
                //先把count个地址写入数据
                EEPROM_PageWrite(pBuffer, WriteAddr, count);
                EEPROM_WaitStandbyState();
                //记得地址要增加
                pBuffer += count;
                WriteAddr += count;
            }

            //把整页地址写入数据
            while(NumOfPage--)
            {
                EEPROM_PageWrite(pBuffer, WriteAddr, EEPROM_PAGE_SIZE);
                EEPROM_WaitStandbyState();
                WriteAddr += EEPROM_PAGE_SIZE;
                pBuffer += EEPROM_PAGE_SIZE;
            }

            //如果还有多余的不满一页的数据，继续写入
            if(NumOfSingle != 0)
            {
                EEPROM_PageWrite(pBuffer, WriteAddr, NumOfSingle);
                EEPROM_WaitStandbyState();
            } 
        }
    } 
}


//把EEPROM中内容读取到缓冲区中，
uint32_t EEPROM_BufferRead(uint8_t* pBuffer, uint8_t ReadAddr, uint16_t NumByteToRead)
{
    I2C_TimeOut = USER_I2C_LONG_TIMEOUT;

    //读之前先读取I2C总线状态确定空闲，如果超时就打印消息
    while(I2C_GetFlagStatus(I2C_EEPROM,I2C_FLAG_BUSY))
    {
        if(I2C_TimeOut--)
            return I2C_TimeOut_UserCallback(9);
    }

    I2C_GenerateSTART(I2C_EEPROM, ENABLE);
    I2C_TimeOut = USER_I2C_FLAG_TIMEOUT;
    while(!I2C_CheckEvent(I2C_EEPROM, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if((I2C_TimeOut--) == 0)
            return I2C_TimeOut_UserCallback(10);
    }

    I2C_TimeOut = USER_I2C_FLAG_TIMEOUT;
    I2C_Send7bitAddress(I2C_EEPROM, EEPROM_DEVICE_ADDRESS, I2C_Direction_Transmitter);
    while(!I2C_CheckEvent(I2C_EEPROM, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    {
        if((I2C_TimeOut--) == 0)
            return I2C_TimeOut_UserCallback(11);
    }

    I2C_Cmd(I2C_EEPROM,ENABLE);    //可以省略

    I2C_TimeOut = USER_I2C_FLAG_TIMEOUT;
    I2C_SendData(I2C_EEPROM,ReadAddr);
    while(!I2C_CheckEvent(I2C_EEPROM, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
        if((I2C_TimeOut--) == 0)
            return I2C_TimeOut_UserCallback(12);
    }

    
    //第二次发送起始信号正式开始读
    I2C_GenerateSTART(I2C_EEPROM, ENABLE);
    I2C_TimeOut = USER_I2C_FLAG_TIMEOUT;
    while(!I2C_CheckEvent(I2C_EEPROM, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if((I2C_TimeOut--) == 0)
            return I2C_TimeOut_UserCallback(13);
    }
    //发送器件地址并设置为读
    I2C_TimeOut = USER_I2C_FLAG_TIMEOUT;
    I2C_Send7bitAddress(I2C_EEPROM, EEPROM_DEVICE_ADDRESS, I2C_Direction_Receiver);
    while(!I2C_CheckEvent(I2C_EEPROM, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED))
    {
        if((I2C_TimeOut--) == 0)
            return I2C_TimeOut_UserCallback(14);
    }

    while(NumByteToRead)
    {
        if(NumByteToRead == 1)
        {
            I2C_AcknowledgeConfig(I2C_EEPROM,DISABLE);
            I2C_GenerateSTOP(I2C_EEPROM,ENABLE);
        }

        I2C_TimeOut = USER_I2C_LONG_TIMEOUT;
        //如果I2C有接收到数据那么就跳过循环，否则等待，超时提醒
        while(!I2C_CheckEvent(I2C_EEPROM, I2C_EVENT_MASTER_BYTE_RECEIVED))
        {
            if((I2C_TimeOut--) == 0)
                return I2C_TimeOut_UserCallback(15);
        }

        *pBuffer = I2C_ReceiveData(I2C_EEPROM);
        pBuffer++;
        NumByteToRead--;
    }

    //为下一次读操作提前使能ACK
    I2C_AcknowledgeConfig(I2C_EEPROM,ENABLE);   

    return 1;

}



/*********************************************END OF FILE****************************************/
