#include "stmflash.h"





static uint32_t GetPage(uint32_t Address);//Get page num of current address
static uint32_t GetBank(uint32_t Address);//Get bank num of current address


uint8_t ulCopy_Code_Flash(uint32_t ulLength)
{
	uint8_t ucCopy_Success = 1;
	uint64_t ulData_H;
	uint64_t ulData_L;
	uint64_t DATA_COPY;
	
	/* Unlock the Flash to enable the flash control register access *************/
	HAL_FLASH_Unlock();
	/* Erase the user Flash area
    (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/
	if(ucFLASH_ERASE(FLASH_USER_START_ADDR,ulLength) != 1)
	{
		ucCopy_Success = 0;
	}
	if(ucCopy_Success)
	{
		uint32_t ulAddress = FLASH_USER_START_ADDR;
		uint32_t ulAddress1 = FLASH_DL_START_ADDR;
	
		while (ulAddress < (FLASH_USER_START_ADDR + ulLength))
		{
			ulData_L = *(__IO uint32_t *)ulAddress1;
			ulData_H = *(__IO uint32_t *)(ulAddress1+4);
			
			DATA_COPY = ulData_H << 32 | ulData_L;
			
			if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, ulAddress, DATA_COPY) == HAL_OK)
			{
			  ulAddress = ulAddress + 8;
			  ulAddress1 = ulAddress1 + 8;
			}
			else
			{
			  /* Error occurred while writing data in Flash memory.
				 User can add here some code to deal with this error */
				ucCopy_Success = 0;
				break;
			}
		}
	}
	/* Lock the Flash to disable the flash control register access (recommended
     to protect the FLASH memory against possible unwanted operation) *********/
	HAL_FLASH_Lock();
	
	return ucCopy_Success;
}



/*FLASH page erase and program*/
uint8_t ucFLASH_USER_PAGE_PROGRAM(uint32_t ulStartAddress,uint8_t* pbuff,uint32_t ulLength)
{
	uint8_t ucProgram_Success = 1;
	/* Unlock the Flash to enable the flash control register access *************/
	HAL_FLASH_Unlock();
	/* Erase the user Flash area
    (area defined by FLASH_USER_START_ADDR and FLASH_USER_END_ADDR) ***********/

	if(ucFLASH_ERASE(ulStartAddress,ulLength) != 1)
	{
		ucProgram_Success = 0;
	}
	if(ucProgram_Success)
	{
		if(ucFLASH_PROGRAM(ulStartAddress,pbuff,ulLength) != 1)
		{
			ucProgram_Success = 0;
		}
	}
	/* Lock the Flash to disable the flash control register access (recommended
     to protect the FLASH memory against possible unwanted operation) *********/
	HAL_FLASH_Lock();
	
	return ucProgram_Success;
}

/*FLASH ERASE*/
uint8_t ucFLASH_ERASE(uint32_t ulStartAddr,uint32_t ulLength)
{
	uint32_t FirstPage = 0, NbOfPages = 0, BankNumber = 0;
	uint32_t PAGEError = 0;
	uint32_t ulSRerror;
	uint32_t ulUserEndAddress;
	FLASH_EraseInitTypeDef EraseInitStruct;
	
	/* Clear OPTVERR bit set on virgin samples */
	__HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_OPTVERR); 
	
	/*Clear SR ERRORS*/
	ulSRerror = (FLASH->SR & FLASH_FLAG_SR_ERRORS);
	__HAL_FLASH_CLEAR_FLAG(ulSRerror);
	
	/*Get end address*/
	ulUserEndAddress = ulStartAddr + ulLength;
	/*Check if the data overrun*/
	if(ulUserEndAddress > FLASH_USER_END_ADDR)
	{
		return 0;
	}
	/* Get the 1st page to erase */
	FirstPage = GetPage(ulStartAddr);
	/* Get the number of pages to erase from 1st page */
	NbOfPages = GetPage(ulUserEndAddress) - FirstPage + 1;
	/* Get the bank */
	BankNumber = GetBank(ulStartAddr);
	/* Fill EraseInit structure*/
	EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
	EraseInitStruct.Banks       = BankNumber;
	EraseInitStruct.Page        = FirstPage;
	EraseInitStruct.NbPages     = NbOfPages;
	
	if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK)
	{
		return 0;
	}
	return 1;
}

/*FLASH WRITE*/
uint8_t ucFLASH_PROGRAM(uint32_t ulStartAddr,uint8_t* pbuff,uint32_t ulLength)
{
	uint8_t  ucBytesRest;
	uint16_t usDoubleWordNum;
	uint32_t ulAddress;
	uint64_t DataWrite;
	
	/*DoubleWord NUM*/
	usDoubleWordNum = ulLength / 8;
	/*Bytes not enough for doubleword*/
	ucBytesRest = ulLength % 8;
	
	ulAddress = ulStartAddr;
	
	while (ulAddress < (ulStartAddr + ulLength))
	{
		if((usDoubleWordNum == 0) && (ucBytesRest != 0))
		{
			DataWrite = ulByteToDoubleword(pbuff,ucBytesRest);
		}
		else
		{
			usDoubleWordNum--;
			DataWrite = ulByteToDoubleword(pbuff,8);
		}
		
		if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, ulAddress, DataWrite) == HAL_OK)
		{
		  ulAddress = ulAddress + 8;
		  pbuff += 8;
		}
		else
		{
		  /* Error occurred while writing data in Flash memory.
			 User can add here some code to deal with this error */
			return 0;
		}
	}
	return 1;
}

/**
  * @brief  Gets the page of a given address
  * @param  Addr: Address of the FLASH Memory
  * @retval The page of a given address
  */
static uint32_t GetPage(uint32_t Addr)
{
  uint32_t page = 0;
  
  if (Addr < (FLASH_BASE + FLASH_BANK_SIZE))
  {
    /* Bank 1 */
    page = (Addr - FLASH_BASE) / FLASH_PAGE_SIZE;
  }
  else
  {
    /* Bank 2 */
    page = (Addr - (FLASH_BASE + FLASH_BANK_SIZE)) / FLASH_PAGE_SIZE;
  }
  
  return page;
}


/**
  * @brief  Gets the bank of a given address
  * @param  Addr: Address of the FLASH Memory
  * @retval The bank of a given address
  */
static uint32_t GetBank(uint32_t Addr)
{
  uint32_t bank = 0;
  
  if (READ_BIT(SYSCFG->MEMRMP, SYSCFG_MEMRMP_FB_MODE) == 0)
  {
  	/* No Bank swap */
    if (Addr < (FLASH_BASE + FLASH_BANK_SIZE))
    {
      bank = FLASH_BANK_1;
    }
    else
    {
      bank = FLASH_BANK_2;
    }
  }
  else
  {
  	/* Bank swap */
    if (Addr < (FLASH_BASE + FLASH_BANK_SIZE))
    {
      bank = FLASH_BANK_2;
    }
    else
    {
      bank = FLASH_BANK_1;
    }
  }
  
  return bank;
}

/*bytes to doubleword writed to flash*/
uint64_t ulByteToDoubleword(uint8_t* data,uint8_t ucLen)
{
	uint64_t DoubleWord;
	uint64_t llData[8];
	
	for(int i=0;i<ucLen;i++)
	{
		llData[i] = *data++;
	}
	
	if(ucLen < 8)
	{
		for(int i=ucLen;i<8;i++)
		{
			llData[i] = 0xFF;
		}
	}
	
	DoubleWord = llData[0] | llData[1]<<8 | llData[2]<<16 | llData[3]<<24 | llData[4]<<32 | llData[5]<<40 | llData[6]<<48 | llData[7]<<56;
	
	return DoubleWord;
}










