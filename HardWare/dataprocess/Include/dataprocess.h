#ifndef __DATAPROCESS_H
#define __DATAPROCESS_H
#include <stdio.h>
#include <string.h>
#include "usart.h"
#include "stmflash.h"
#include "stm32l4xx_hal.h"
#include "aes.h"

typedef enum {
	IAP_HANDSHAKE_SUCCESS,    /*0*/
	IAP_ERROR_HANDSHAKE,   		/*1*/   
	IAP_ERROR_DATA_CRC,			  /*2*/
  IAP_ERROR_WAIT,						/*3*/
	IAP_ERROR_ADDRESS,				/*4*/
	IAP_ERROR_COPY,						/*5*/
	IAP_SUCCESS,							/*6*/
	LAST_CODE_RUN,						/*7*/
  IAP_ERROR_FLASH_WRITE,		/*8*/	
	IAP_PERPACK_SUCCESS 			/*9*/
} eIapRunStatus;						

typedef enum {
	DEVICE_MODE_IAP_WAITING,        
    DEVICE_MODE_IAP_GOING,
} eDeviceRunState;

extern eDeviceRunState eDeviceState;

typedef struct {
	uint8_t  ucLastVersion[5];
	uint8_t  ucCurrentVersion[5];
    uint8_t  ucPackTotalNum;
	uint8_t  ucPackCurrentNum;
	uint8_t  ucPackRestNum;
	uint16_t ulPackSize;
	uint8_t  ucPackCRCState;
	uint8_t  ucPackErrorNum;
	uint8_t  ucPackRightNum;
	
	
} xPackHandlerType;
extern xPackHandlerType xPackHandle;

uint8_t ucHandshake(void);    //Handshake with CPU
uint16_t usDLdatacheck(uint8_t *pucDataBuff,uint32_t ulLength);//Download data CRC check
uint8_t ucCodeToFlash(uint32_t ulStartAddress,uint8_t *pucDataBuff,uint32_t ulDataLength);//Write code to flash 
uint16_t usCRCcheck(uint8_t *pucData,uint32_t ulDatalen);//CRC check algorithm
void vDecode(uint8_t *pucData,uint32_t ulLength);//Decode code that is CRC checked right
void vIapProcess(eIapRunStatus eError);//IAP run states process
#endif


