/*
***********************************************************************************
*�ļ�����bsp_eeprom.c
*���ߣ�QFB
*�汾��V1.0
*���ڣ�2020-12-17
*˵����Ӳ��I2C�ӿڶ�дEEPROM����ʵ��
*ƽ̨��Ұ��M3������
***********************************************************************************
*/

#include "stm32f10x.h"    //ʵ��bsp_led.h���Ѱ���stm32f10x.hͷ�ļ������п���ע�͵�
#include "bsp_eeprom.h"


static __IO uint32_t I2C_TimeOut = USER_I2C_LONG_TIMEOUT;


//��ʼ������Ӳ��I2C
void I2C_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructrue;

    I2C_GPIO_APBPERIPHCLOCKCMD(I2C_GPIO_CLK, ENABLE);
    I2C_AFIO_APBPERIPHCLOCKCMD(I2C_AFIO_CLK, ENABLE);

    /*GPIO��ʼ��*/
    GPIO_InitStructrue.GPIO_Pin = I2C_SCL_GPIO_PIN;
    GPIO_InitStructrue.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructrue.GPIO_Speed = GPIO_Speed_50MHz; 
    GPIO_Init(I2C_SCL_GPIO_PORT,&GPIO_InitStructrue);  
    
    /*GPIO��ʼ��*/
    GPIO_InitStructrue.GPIO_Pin = I2C_SDA_GPIO_PIN;
    GPIO_InitStructrue.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructrue.GPIO_Speed = GPIO_Speed_50MHz; 
    GPIO_Init(I2C_SDA_GPIO_PORT,&GPIO_InitStructrue);  

}


void I2C_Mode_Config(void)
{
    //�����ʼ��
    I2C_InitTypeDef I2C_InitStructure;    
    
    I2C_InitStructure.I2C_ClockSpeed = I2C_SPEED;
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle =I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = I2C_OWN_ADDRESS_7;
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_Init(I2C_EEPROM,&I2C_InitStructure);
    
    //ʹ������
    I2C_Cmd(I2C_EEPROM,ENABLE);
}

void I2C_EEPROM_Init(void)
{
    I2C_GPIO_Config();
    I2C_Mode_Config();
}


//�����ʱ�ʹ�ӡ��ʱ�Ļ��ڣ����ڵ���
static uint32_t I2C_TimeOut_UserCallback(uint8_t ErrorCode)
{
    EEPROM_ERROR("I2C�ȴ���ʱ���������: %d",ErrorCode);
    return 0;
}


//�ȴ��������,��ΪҪ�����ļ��������������ã����Է���ǰ��
void EEPROM_WaitStandbyState(void)
{
    __IO uint16_t SR1_Temp = 0;
    do
    {
        I2C_GenerateSTART(I2C_EEPROM, ENABLE);
        //��ȡSR1�Ĵ���
        SR1_Temp = I2C_ReadRegister(I2C_EEPROM, I2C_Register_SR1);
        //����EEPROM������ַ������Ϊд
        I2C_Send7bitAddress(I2C_EEPROM, EEPROM_DEVICE_ADDRESS, I2C_Direction_Transmitter);
        //�ȴ���ַ�������
    } while (!(I2C_ReadRegister(I2C_EEPROM, I2C_Register_SR1) & 0x0002));  //I2C_FLAG_ADDR = 1��ʾ��ַ���ͳɹ���0��ʾ��ַ����δ���
        
    I2C_ClearFlag(I2C_EEPROM, I2C_FLAG_AF);    //���ack fail��־λ
    I2C_GenerateSTOP(I2C_EEPROM,ENABLE);


}


//���ֽ�д��
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

//��ҳд�룬ÿ��д����ֽ������ܳ���8��ÿҳ8���ֽڣ�
uint32_t EEPROM_PageWrite(uint8_t* pBuffer, uint8_t WriteAddr, uint8_t NumByteToWrite)
{
    I2C_TimeOut = USER_I2C_LONG_TIMEOUT;
    while(I2C_GetFlagStatus(I2C_EEPROM, I2C_FLAG_BUSY))
    {
        if((I2C_TimeOut--) == 0)
            return I2C_TimeOut_UserCallback(4);
    }
    //���Ϳ�ʼ�ź�
    I2C_GenerateSTART(I2C_EEPROM, ENABLE);
    I2C_TimeOut = USER_I2C_FLAG_TIMEOUT;
    //���EV5������¼�
    while(!I2C_CheckEvent(I2C_EEPROM, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if((I2C_TimeOut--) == 0)
            return I2C_TimeOut_UserCallback(5);
    }
    //����������ַ������Ϊд
    I2C_Send7bitAddress(I2C_EEPROM, EEPROM_DEVICE_ADDRESS, I2C_Direction_Transmitter);
    I2C_TimeOut = USER_I2C_FLAG_TIMEOUT;
    //���EV6������¼�
    while(!I2C_CheckEvent(I2C_EEPROM, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
    {
        if((I2C_TimeOut--) == 0)
            return I2C_TimeOut_UserCallback(6);
    }
    //���������ڲ���ַ����д��
    I2C_SendData(I2C_EEPROM,WriteAddr);
    I2C_TimeOut = USER_I2C_FLAG_TIMEOUT;
    //���EV8������¼�
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

//�ѻ���������һ��д��
void EEPROM_BufferWrite(uint8_t* pBuffer, uint8_t WriteAddr, uint16_t NumByteToWrite)
{
    uint8_t NumOfPage = 0, NumOfSingle = 0, Addr = 0, count = 0, temp = 0;
    
    //д���ַ��ҳ��ַƫ��
    Addr = WriteAddr % EEPROM_PAGE_SIZE;
    //��Ҫ����ĵ�ַ�����ﵽ����ҳ��ַ
    count = EEPROM_PAGE_SIZE - Addr;

    NumOfPage = NumByteToWrite / EEPROM_PAGE_SIZE;
    NumOfSingle = NumByteToWrite % EEPROM_PAGE_SIZE;

    //���д���ַ��ҳ��ַ����
    if(Addr == 0)
    {   
        //���д����ֽ�������һҳ
        if(NumOfPage == 0)
        {
            EEPROM_PageWrite(pBuffer, WriteAddr, NumOfSingle);
            EEPROM_WaitStandbyState();
        }
        //���д����ֽ�������һҳ
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
                EEPROM_PageWrite(pBuffer, WriteAddr, count);
                EEPROM_WaitStandbyState();

                //��д����һҳ
                temp = NumOfSingle - count;
                pBuffer += count;
                WriteAddr += count;
                
                EEPROM_PageWrite(pBuffer, WriteAddr, temp);
                EEPROM_WaitStandbyState();
            }
            //�����ǰҳʣ�µĵ�ַ�ռ��㹻д���ֽ���
            else
            {
                EEPROM_PageWrite(pBuffer, WriteAddr, NumOfSingle);
                EEPROM_WaitStandbyState();
            }
        }
        //���д����ֽ�������һҳ
        else
        {
            NumByteToWrite -= count;
            NumOfPage = NumByteToWrite / EEPROM_PAGE_SIZE;
            NumOfSingle = NumByteToWrite % EEPROM_PAGE_SIZE;

            //�Ȱ���ǰ��count����ַд�����ݣ�������ַ�ͺ�ҳ��ַ������
            if(count != 0)
            {
                //�Ȱ�count����ַд������
                EEPROM_PageWrite(pBuffer, WriteAddr, count);
                EEPROM_WaitStandbyState();
                //�ǵõ�ַҪ����
                pBuffer += count;
                WriteAddr += count;
            }

            //����ҳ��ַд������
            while(NumOfPage--)
            {
                EEPROM_PageWrite(pBuffer, WriteAddr, EEPROM_PAGE_SIZE);
                EEPROM_WaitStandbyState();
                WriteAddr += EEPROM_PAGE_SIZE;
                pBuffer += EEPROM_PAGE_SIZE;
            }

            //������ж���Ĳ���һҳ�����ݣ�����д��
            if(NumOfSingle != 0)
            {
                EEPROM_PageWrite(pBuffer, WriteAddr, NumOfSingle);
                EEPROM_WaitStandbyState();
            } 
        }
    } 
}


//��EEPROM�����ݶ�ȡ���������У�
uint32_t EEPROM_BufferRead(uint8_t* pBuffer, uint8_t ReadAddr, uint16_t NumByteToRead)
{
    I2C_TimeOut = USER_I2C_LONG_TIMEOUT;

    //��֮ǰ�ȶ�ȡI2C����״̬ȷ�����У������ʱ�ʹ�ӡ��Ϣ
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

    I2C_Cmd(I2C_EEPROM,ENABLE);    //����ʡ��

    I2C_TimeOut = USER_I2C_FLAG_TIMEOUT;
    I2C_SendData(I2C_EEPROM,ReadAddr);
    while(!I2C_CheckEvent(I2C_EEPROM, I2C_EVENT_MASTER_BYTE_TRANSMITTED))
    {
        if((I2C_TimeOut--) == 0)
            return I2C_TimeOut_UserCallback(12);
    }

    
    //�ڶ��η�����ʼ�ź���ʽ��ʼ��
    I2C_GenerateSTART(I2C_EEPROM, ENABLE);
    I2C_TimeOut = USER_I2C_FLAG_TIMEOUT;
    while(!I2C_CheckEvent(I2C_EEPROM, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if((I2C_TimeOut--) == 0)
            return I2C_TimeOut_UserCallback(13);
    }
    //����������ַ������Ϊ��
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
        //���I2C�н��յ�������ô������ѭ��������ȴ�����ʱ����
        while(!I2C_CheckEvent(I2C_EEPROM, I2C_EVENT_MASTER_BYTE_RECEIVED))
        {
            if((I2C_TimeOut--) == 0)
                return I2C_TimeOut_UserCallback(15);
        }

        *pBuffer = I2C_ReceiveData(I2C_EEPROM);
        pBuffer++;
        NumByteToRead--;
    }

    //Ϊ��һ�ζ�������ǰʹ��ACK
    I2C_AcknowledgeConfig(I2C_EEPROM,ENABLE);   

    return 1;

}



/*********************************************END OF FILE****************************************/
