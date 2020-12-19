#ifndef __BSP_SPI_FLASH_H
#define __BSP_SPI_FLASH_H

#include "stm32f10x.h"
#include "bsp_usart.h"
#include "stdio.h"


//SPI接口宏定义
//CSN GPIO宏定义
#define SPI_CSN_GPIO_PORT                   GPIOA
#define SPI_CSN_GPIO_PIN                    GPIO_Pin_4
#define SPI_CSN_GPIO_CLK                    RCC_APB2Periph_GPIOB
#define SPI_CSN_GPIO_APBPERIPHCLOCKCMD      RCC_APB2PeriphClockCmd

//CLK GPIO宏定义
#define SPI_CLK_GPIO_PORT                   GPIOA
#define SPI_CLK_GPIO_PIN                    GPIO_Pin_5
#define SPI_CLK_GPIO_CLK                    RCC_APB2Periph_GPIOB
#define SPI_CLK_GPIO_APBPERIPHCLOCKCMD      RCC_APB2PeriphClockCmd

//MISO GPIO宏定义
#define SPI_MISO_GPIO_PORT                   GPIOA
#define SPI_MISO_GPIO_PIN                    GPIO_Pin_6
#define SPI_MISO_GPIO_CLK                    RCC_APB2Periph_GPIOB
#define SPI_MISO_GPIO_APBPERIPHCLOCKCMD      RCC_APB2PeriphClockCmd

//MOSI GPIO宏定义
#define SPI_MOSI_GPIO_PORT                   GPIOA
#define SPI_MOSI_GPIO_PIN                    GPIO_Pin_7
#define SPI_MOSI_GPIO_CLK                    RCC_APB2Periph_GPIOB
#define SPI_MOSI_GPIO_APBPERIPHCLOCKCMD      RCC_APB2PeriphClockCmd


//SPI外设宏定义
#define SPI_W25X                            SPI1
#define SPI_AFIO_CLK                        RCC_APB2Periph_SPI1
#define SPI_AFIO_APBPERIPHCLOCKCMD          RCC_APB2PeriphClockCmd

#define SPI_CS_LOW()    GPIO_ResetBits(SPI_CSN_GPIO_PORT, SPI_CSN_GPIO_PIN);
#define SPI_CS_HIGH()    GPIO_SetBits(SPI_CSN_GPIO_PORT, SPI_CSN_GPIO_PIN);


//W25X16每页有256个字节
#define W25_PAGE_SIZE                    256
//W25X16 FLASH ID
#define W25_FLASH_ID                     0xEF3015
// //W25Q16 FLASH ID
// #define W25X16_FLASH_ID                     0xEF4015
// //W25Q128 FLASH ID
// #define W25X16_FLASH_ID                     0xEF4018
// //W25Q64 FLASH ID
// #define W25X16_FLASH_ID                     0xEF4017

#define W25_CMD_WRITE_ENABLE                0x06
#define W25_CMD_WRITE_DISENABLE             0x04
#define W25_CMD_READ_STATUS_REG             0x05
#define W25_CMD_WRITE_STATUS_REG            0x01
#define W25_CMD_READ_DATA                   0x03
#define W25_CMD_FAST_READ_DATA              0x0B
#define W25_CMD_FAST_READ_DUAL              0x3B
#define W25_CMD_PAGE_PROGRAM                0x02
#define W25_CMD_BLOCK_ERASE                 0xD8
#define W25_CMD_SECTOR_ERASE                0x20
#define W25_CMD_CHIP_ERASE                  0xC7
#define W25_CMD_POWER_DOWN                  0xB9    //进入掉电模式
#define W25_CMD_RELEASE_POWER_DOWN          0xAB    //唤醒
#define W25_CMD_READ_DEVICE_ID              0xAB
#define W25_CMD_READ_MANUFACTACT_DEVICE_ID      0x90
#define W25_CMD_READ_JEDEC_DEVICE_ID            0x9F

//BUSY标志位
#define WIP_FLAG        0x01
//空写
#define DUMMY_BYTE      0xFF


#define USER_SPI_FLAG_TIMEOUT        ((uint32_t)0x1000)         
#define USER_SPI_LONG_TIMEOUT        ((uint32_t)(USER_SPI_FLAG_TIMEOUT * 10))  //带参宏建议加（）

#define DEBUG_ON   0
// #define DEBUG_ON   1

#define INFO(fmt,arg...)    printf("INFO: "fmt" \n", ##arg)
#define ERROR(fmt,arg...)    printf("ERROR: "fmt" \n", ##arg)

#define DEBUG(fmt,arg,...)   do\
                                    {if(DEBUG_ON) \
                                    printf("DEBUG: [%d] "fmt" \n", _LINE_, ##arg);\
                                    }while(0)


void SPI_FLASH_Init(void);
void SPI_FLASH_SectorErase(uint32_t SectorAddr);
void SPI_FLASH_BulkErase(void);
void SPI_FLASH_PageWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
void SPI_FLASH_BufferWrite(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
void SPI_FLASH_BufferRead(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead);
uint32_t SPI_FLASH_ReadJedecDeviceID(void);
uint32_t SPI_FLASH_ReadDeviceID(void);
void SPI_FLASH_StartReadSequence(uint32_t ReadAddr);
void SPI_FLASH_PowerDown(void);
void SPI_FLASH_WakeUp(void);

uint8_t SPI_FLASH_ReadByte(void);
uint8_t SPI_FLASH_SendByte(uint8_t Byte);
uint16_t SPI_FLASH_SendHalfWord(uint16_t HalfWord);
void SPI_FLASH_WriteEnable(void);
void SPI_FLASH_WaitForWriteEnd(void);

uint32_t SPI_TimeOut_UserCallBack(uint8_t ErrorCode);

#endif

/*********************************************END OF FILE**********************/
   

