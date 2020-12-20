/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2014        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */
#include "ff.h"
#include "bsp_spi_flash.h"

/* Definitions of physical drive number for each drive */
#define ATA		        0	/* reserve for SD card */
#define SPI_FLASH		1	/* fro spi flash on board */


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat = STA_NOINIT;
	//int result;

	switch (pdrv) {
	case ATA :
        /*SD CARD*/
		      break;

	case SPI_FLASH :
        if(SPI_FLASH_ReadJedecDeviceID() == W25_FLASH_ID)
        {
            stat &= ~STA_NOINIT;    // device ID check pass, return 0

        }
        else
        {
            stat = STA_NOINIT;  // ID check fail, return 1
        }
        break;
    default:
        stat = STA_NOINIT;
        break;

	}
	return stat;
}


/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat = STA_NOINIT;
	//int result;
    uint16_t i;

	switch (pdrv) {
	case ATA :
        /*SD CARD*/
		      break;

	case SPI_FLASH :
		SPI_FLASH_Init();
        i = 500;
        while(--i);
        SPI_FLASH_WakeUp();
        stat = disk_status(SPI_FLASH);
        break;

    default:
        stat = STA_NOINIT;
        break;

	}
	return stat;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address in LBA */
	UINT count		/* Number of sectors to read  max = 2MBYTE/4096 = 512 */
)
{
	DRESULT res = RES_PARERR;
	//int result;

	switch (pdrv) {
	case ATA :
		/*SD CARD*/
		      break;

	case SPI_FLASH :
        SPI_FLASH_BufferRead(buff, sector<<12, count<<12);  // sector size = 4096 = 2^12 bytes
        res = RES_OK;
        break;
		
    default:
        res = RES_PARERR;
        break;
	
	}

	return res;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address in LBA */
	UINT count			/* Number of sectors to write */
)
{
	DRESULT res = RES_PARERR;
	//int result;

	switch (pdrv) {
	case ATA :
		/*SD CARD*/
		      break;

	case SPI_FLASH :
        SPI_FLASH_SectorErase(sector<<12);  //make sure erase operation before write
        SPI_FLASH_BufferWrite((uint8_t *)buff, sector<<12, count<<12);  // sector size = 4096 = 2^12 bytes
        res = RES_OK;
        break;
		
    default:
        res = RES_PARERR;
        break;
	
	}

	return res;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res = RES_PARERR;
	//int result;

	switch (pdrv) {
	case ATA :
		/*SD CARD*/
		      break;

	case SPI_FLASH :
        switch(cmd)
        {
            case GET_SECTOR_COUNT:
                *(DWORD *) buff = 512;
                break;
            case GET_SECTOR_SIZE:
                *(DWORD *) buff = 4096;
                break;
            case GET_BLOCK_SIZE:
                *(DWORD *) buff = 1;
                break;
        }
        res = RES_OK;
        break;
		
    default:
        res = RES_PARERR;
        break;
	
	}
	return res;

}
#endif

__weak DWORD get_fattime(void)
{
    return ((DWORD)(2020 - 1980) << 25)     //YEAR
                    | ((DWORD)12 << 21)      //MONTH
                    | ((DWORD)20 << 16)     //DAY
                    | ((DWORD)7 << 11)     //HOUR
                    | ((DWORD)20 << 5)     //MINUTE
                    | ((DWORD)30 << 1);     //SECOND
}
