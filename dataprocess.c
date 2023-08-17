#include "dataprocess.h"


uint8_t RX_CHECK_BUF[5]={0x11,0x22,0x33,0x79,0x70};
unsigned char key[] =
{
	0x2b, 0x7e, 0x15, 0x16,
	0x28, 0xae, 0xd2, 0xa6,
	0xab, 0xf7, 0x15, 0x88,
	0x09, 0xcf, 0x4f, 0x3c
};

/*Handshake with CPU*/
uint8_t ucHandshake(void)
{
	uint8_t ucData = 1;
	/*CRC check*/
	if(usDLdatacheck(USART_RX_BUF,0x05) == 1)
	{
		for(int i=0;i<5;i++)
		{
			/*Check the received buff*/
			if(USART_RX_BUF[i] != RX_CHECK_BUF[i])
			{
				memset(USART_RX_BUF,0,50);
				ucData=0;
				break;
			}
		}
	}
	else
	{
		ucData = 0;
	}
	return ucData;
}

/*Download data CRC check*/
uint16_t usDLdatacheck(uint8_t *pucDataBuff,uint32_t ulLength)
{
	uint16_t usCRC;
	/*CRC check*/
	usCRC = usCRCcheck(USART_DL_BUF,ulLength-2);
	if((USART_DL_BUF[ulLength-2] == (usCRC & 0xff)) && (USART_DL_BUF[ulLength-1] == (usCRC >> 8)))
	{
		return 1;
	}
	else
	{
		return 0;
	}
	
}

/*Write code to flash */
uint8_t ucCodeToFlash(uint32_t ulStartAddress,uint8_t* pucDataBuff,uint32_t ulDataLength)
{
	uint8_t ucSuccess = 0;
	/* erase and program the page*/
	if(ucFLASH_USER_PAGE_PROGRAM(ulStartAddress,pucDataBuff,ulDataLength) == 1)
	{
		ucSuccess = 1;
	}
	return ucSuccess;
}

/*CRC check algorithm*/
uint16_t usCRCcheck(uint8_t *pucData,uint32_t ulDatalen)
{
	uint16_t crc;
	uint16_t carry_flag;
	uint32_t i;
	uint16_t j;
	
	crc = 0xffff;
	for(i = 0;i < ulDatalen;i++)
	{
		crc ^= pucData[i];
		for(j = 0;j < 8;j++)
		{
			carry_flag = crc & 0x0001;
			crc >>= 1;
			if(carry_flag == 1)
			{
				crc ^= 0xa001;
			}
		}
	}
	return crc;

}

/*Decode code that is CRC checked right*/
void vDecode(uint8_t* pucData,uint32_t ulLength)
{
	AES (key);
	InvCipherfor(pucData,ulLength);
}

/*IAP run states process*/
void vIapProcess(eIapRunStatus eError)
{
	uint8_t i=0;
	uint16_t usCRC;
	
	if(xPackHandle.ucPackCRCState == 0x01)
	{
		USART_TX_BUF[i++] = xPackHandle.ucCurrentVersion[0];
		USART_TX_BUF[i++] = xPackHandle.ucCurrentVersion[1];
		USART_TX_BUF[i++] = xPackHandle.ucCurrentVersion[2];
		USART_TX_BUF[i++] = xPackHandle.ucCurrentVersion[3];
		USART_TX_BUF[i++] = xPackHandle.ucPackTotalNum;
		USART_TX_BUF[i++] = xPackHandle.ucPackCurrentNum;
		USART_TX_BUF[i++] = xPackHandle.ucPackRestNum;
		USART_TX_BUF[i++] = xPackHandle.ulPackSize >> 8;
		USART_TX_BUF[i++] = xPackHandle.ulPackSize & 0xFF;
		switch(eError)
		{	
			case IAP_ERROR_FLASH_WRITE:
			{
				USART_TX_BUF[i++] = 0x00;
				USART_TX_BUF[i++] = IAP_ERROR_FLASH_WRITE;
			}
			break;
			
			case IAP_PERPACK_SUCCESS:
			{
				USART_TX_BUF[i++] = 0x01;
				USART_TX_BUF[i++] = IAP_PERPACK_SUCCESS;
			}
			break;	
			default: break;
		}
	}
	else
	{
		switch(eError)
		{
			case IAP_HANDSHAKE_SUCCESS:
			{
				uint32_t ulData;
				ulData = *(__IO uint32_t *)(FLASH_STATE_ADDR+4);				
				
				USART_TX_BUF[i++] = 0x11;
				USART_TX_BUF[i++] = 0x11;
				USART_TX_BUF[i++] = IAP_HANDSHAKE_SUCCESS;
				USART_TX_BUF[i++] = (uint8_t)(ulData >> 24);
				USART_TX_BUF[i++] = (uint8_t)(ulData >> 16);
				USART_TX_BUF[i++] = (uint8_t)(ulData >> 8);
				USART_TX_BUF[i++] = (uint8_t)(ulData & 0xFF);
			}
			break;
		
			case IAP_ERROR_HANDSHAKE:
			{
				USART_TX_BUF[i++] = 0x11;
				USART_TX_BUF[i++] = 0x11;
				USART_TX_BUF[i++] = IAP_ERROR_HANDSHAKE;
			}
			break;
			
			case IAP_ERROR_DATA_CRC:
			{
				USART_TX_BUF[i++] = 0x11;
				USART_TX_BUF[i++] = 0x11;
				USART_TX_BUF[i++] = IAP_ERROR_DATA_CRC;
			}
			break;
			
			case IAP_ERROR_ADDRESS:
			{
				USART_TX_BUF[i++] = 0x11;
				USART_TX_BUF[i++] = 0x11;
				USART_TX_BUF[i++] = IAP_ERROR_ADDRESS;
			}
			break;
			
			case IAP_ERROR_WAIT:
			{
				USART_TX_BUF[i++] = 0x11;
				USART_TX_BUF[i++] = 0x11;
				USART_TX_BUF[i++] = IAP_ERROR_WAIT;
			}
			break;
			
			case IAP_SUCCESS:
			{
				uint32_t ulData;
				ulData = *(__IO uint32_t *)(FLASH_STATE_ADDR+4);				
				
				USART_TX_BUF[i++] = 0x11;
				USART_TX_BUF[i++] = 0x11;
				USART_TX_BUF[i++] = IAP_SUCCESS;
				USART_TX_BUF[i++] = (uint8_t)(ulData >> 24);
				USART_TX_BUF[i++] = (uint8_t)(ulData >> 16);
				USART_TX_BUF[i++] = (uint8_t)(ulData >> 8);
				USART_TX_BUF[i++] = (uint8_t)(ulData & 0xFF);	
			}
			break;
			
			case IAP_ERROR_COPY:
			{
				USART_TX_BUF[i++] = 0x11;
				USART_TX_BUF[i++] = 0x11;
				USART_TX_BUF[i++] = IAP_ERROR_COPY;
			}
			break;
			
			case LAST_CODE_RUN:
			{
				USART_TX_BUF[i++] = 0x11;
				USART_TX_BUF[i++] = 0x11;
				USART_TX_BUF[i++] = LAST_CODE_RUN;
			}
			break;
			
			default: break;
		}
	}
	xPackHandle.ucPackCRCState = 0x00;
	/*Combine with CRC data*/
	usCRC = usCRCcheck(USART_TX_BUF,i);
	USART_TX_BUF[i++] = usCRC & 0xFF;
	USART_TX_BUF[i++] = usCRC >> 8;
	USART_TX_SIZE = i;
	USART_TX_CNT = 0;
	/*ENABLE TX*/
	vMBPortSerialEnable(FALSE,TRUE);
	HAL_Delay( 5 );
}



