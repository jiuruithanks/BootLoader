#include "iap.h" 

uint8_t ucJumpToLoad = 0;//Ready to jump to app code status
uint8_t ucPageWriteState[15];//pack and iap message buff writed to flash page 
uint16_t usTimeOut = 200;//Time out for IAP when no data received
uint16_t usRunCount;//IAP wait time count
uint32_t ulData32 = 0;//data read from flash to check the IAP status
iapfun jump2app; // Jump to app
eDeviceRunState eDeviceState = DEVICE_MODE_IAP_WAITING;//Device state during IAP
eIapRunStatus eIapStatus = IAP_ERROR_WAIT;//IAP state during IAP
xPackHandlerType xPackHandle;//Pack received buff to handle

/*Jump to app code*/
void iap_load_app(uint32_t appxaddr)
{
    SysTick->CTRL = 0;                                //关键代码
    HAL_DeInit();                                     //可选
    HAL_NVIC_DisableIRQ(SysTick_IRQn);                //可选
    HAL_NVIC_ClearPendingIRQ(SysTick_IRQn);           //可选
    HAL_NVIC_DisableIRQ(LPUART1_IRQn);                //可选
    HAL_NVIC_ClearPendingIRQ(LPUART1_IRQn);           //可选
    
    //The second word in the user code area is the program start address (reset address)
    jump2app=(iapfun)*(vu32*)(appxaddr+4);
    //Initialize the app stack pointer (the first word in the user code area is used to store the stack top address)		
    __set_MSP(*(vu32*)appxaddr);

    jump2app();									
}		 


/**
* @brief  Set the Device in sleep or stop state.
* @param  None
* @retval None
*/
void vDeviceSLeep( void )
{
if((LPUART1->CR1 & USART_CR1_TE) == 0)

{
    LPUART1->CR1 |= USART_CR1_UESM;
    LPUART1->CR3 |= (USART_CR3_UCESM | USART_CR3_WUS);
    
    /*Enter Stop0 */
    HAL_PWREx_EnterSTOP0Mode(PWR_STOPENTRY_WFI);
    
    /*Return form stop0, reconfig the clock                                                                                                                                                                                                                                                                                                          */            
    vSystemClockConfig16MHz(  );
}
else    
{
    HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI); 
}  
}

//Device IAP run control function
void vDeviceRunControl(void)
{
uint32_t ulLength;

/*Check whether receive some datas and receive over*/
ulLength = ulRx_State_Check();
/*Data received over*/
if(ulLength)
{
    switch(eDeviceState)
    {
        /*Wait for IAP START*/
        case DEVICE_MODE_IAP_WAITING:
        {
            /*Handshske ,SUCCESS:IAP START  FAIL:IAP WAIT*/
            if(ucHandshake() == 1)
            {
                eIapStatus = IAP_HANDSHAKE_SUCCESS;
                xPackHandle.ucPackCRCState = 0x00;
                /*Clear PackRightNum for new IAP_GOING*/
                xPackHandle.ucPackRightNum = 0;
                //Handshake  success
                vIapProcess(eIapStatus);
                eDeviceState = DEVICE_MODE_IAP_GOING;
                /*NO data last MAX time for DEVICE_MODE_IAP_GOING*/
                usRunCount = 0;
                usTimeOut = 300;
            }
            else
            {
                eIapStatus = IAP_ERROR_HANDSHAKE;
                xPackHandle.ucPackCRCState = 0x00;
                //Handshake  error
                vIapProcess(eIapStatus);
            }				
        }
        break;
        
        /*IAP running*/
        case DEVICE_MODE_IAP_GOING:
        {
            /*Receive pack CRC check*/
            if(usDLdatacheck(USART_DL_BUF,ulLength) != 0)
            {
                int i=0;
                /*PackHandle Struct Config*/
                xPackHandle.ucPackCRCState = 1;
                xPackHandle.ucCurrentVersion[0] = USART_DL_BUF[i++];
                xPackHandle.ucCurrentVersion[1] = USART_DL_BUF[i++];
                xPackHandle.ucCurrentVersion[2] = USART_DL_BUF[i++];
                xPackHandle.ucCurrentVersion[3] = USART_DL_BUF[i++];
                xPackHandle.ucPackTotalNum = USART_DL_BUF[i++];
                xPackHandle.ucPackCurrentNum = USART_DL_BUF[i++];
                xPackHandle.ucPackRestNum = USART_DL_BUF[i++];
                xPackHandle.ulPackSize = (uint16_t)USART_DL_BUF[i] << 8 | USART_DL_BUF[i+1];
                i += 2;
                /*Check whether it is the last pack*/
                if(xPackHandle.ucPackRestNum == 0)
                {
                    uint32_t ulData1;
                    uint32_t ulData2;
                    ulData1 = *(__IO uint32_t *)FLASH_STATE_ADDR;
                    ulData2 = *(__IO uint32_t *)(FLASH_STATE_ADDR+4);				
                    
                    /*PageWriteBuff Config*/
                    ucPageWriteState[0] = 0x00;
                    ucPageWriteState[1] = (uint8_t)(ulData1 >> 8);
                    ucPageWriteState[2] = (uint8_t)(ulData1 >> 16);
                    ucPageWriteState[3] = (uint8_t)(ulData1 >> 24);
                    ucPageWriteState[4] = (uint8_t)(ulData2 & 0xFF);
                    ucPageWriteState[5] = (uint8_t)(ulData2 >> 8);
                    ucPageWriteState[6] = (uint8_t)(ulData2 >> 16);
                    ucPageWriteState[7] = (uint8_t)(ulData2 >> 24);
                    
                }
                /*decrypt the data*/
                vDecode(USART_DL_BUF+i,xPackHandle.ulPackSize);
                /*Write code to FLASH*/
                if(ucCodeToFlash(FLASH_USER_START_ADDR+(xPackHandle.ucPackCurrentNum-1)*2048,USART_DL_BUF+i,xPackHandle.ulPackSize) == 1)
                {
                    /*PackRightNum increse*/
                    xPackHandle.ucPackRightNum++;
                    /*The last pack*/
                    if(xPackHandle.ucPackRestNum == 0)
                    {
                        /*Check if PackRightNum=PackTotalNum*/
                        if(xPackHandle.ucPackRightNum == xPackHandle.ucPackTotalNum)
                        {
                            eIapStatus = IAP_PERPACK_SUCCESS;
                            xPackHandle.ucPackCRCState = 0x01;
                            //IAP PerPack Success
                            vIapProcess(eIapStatus);
                            HAL_Delay(100);
                            
                            /*Check if the Start Address is 0X08XXXXXX*/
                            if(((*(vu32*)(FLASH_USER_START_ADDR+4))&0xFF000000)==0x08000000)
                            {	
                                /*Set IAP_SUCCESS status*/
                                    ucPageWriteState[0] = 0x11;
                                    ucPageWriteState[1] = 0x01;
                                    ucPageWriteState[4] = xPackHandle.ucCurrentVersion[3];
                                    ucPageWriteState[5] = xPackHandle.ucCurrentVersion[2];
                                    ucPageWriteState[6] = xPackHandle.ucCurrentVersion[1];
                                    ucPageWriteState[7] = xPackHandle.ucCurrentVersion[0];
                                    /*Write Message Page*/
                                    ucFLASH_USER_PAGE_PROGRAM(FLASH_STATE_ADDR,ucPageWriteState,8);
                
                                    eIapStatus = IAP_SUCCESS;
                                    xPackHandle.ucPackCRCState = 0x00;
                                    //IAP Success
                                    vIapProcess(eIapStatus);
                                    
                                    ucJumpToLoad = 1;
                                    iap_load_app(FLASH_USER_START_ADDR);	
                            }
                            else
                            {
                                /*Write Message Page*/
                                ucFLASH_USER_PAGE_PROGRAM(FLASH_STATE_ADDR,ucPageWriteState,8);
                                eIapStatus = IAP_ERROR_ADDRESS;
                                xPackHandle.ucPackCRCState = 0x00;
                                //Code Start Address error
                                vIapProcess(eIapStatus);
                                eDeviceState = DEVICE_MODE_IAP_WAITING;
                                /*NO data last MAX time for DEVICE_MODE_IAP_WAITING*/
                                usRunCount = 0;
                                usTimeOut = 200;
                            }
                        }
                        else
                        {
                            /*Write Message Page*/
                            ucFLASH_USER_PAGE_PROGRAM(FLASH_STATE_ADDR,ucPageWriteState,8);
                            eIapStatus = IAP_PERPACK_SUCCESS;
                            xPackHandle.ucPackCRCState = 0x01;
                            //IAP PerPack Success
                            vIapProcess(eIapStatus);
                            eDeviceState = DEVICE_MODE_IAP_WAITING;
                            /*NO data last MAX time for DEVICE_MODE_IAP_WAITING*/
                            usRunCount = 0;
                            usTimeOut = 200;
                        }				
                    }
                    else
                    {
                        eIapStatus = IAP_PERPACK_SUCCESS;
                        xPackHandle.ucPackCRCState = 0x01;
                        //IAP PerPack Success
                        vIapProcess(eIapStatus);
                        //feed the dog
                        usRunCount = 0;
                        usTimeOut = 200;
                    }
                }
                else
                {
                    eIapStatus = IAP_ERROR_FLASH_WRITE;
                    xPackHandle.ucPackCRCState = 0x01;
                    //Flash write error
                    vIapProcess(eIapStatus);
                    
                    /*The last pack*/
                    if(xPackHandle.ucPackRestNum == 0)
                    {
                        /*Write Message Page*/
                        ucFLASH_USER_PAGE_PROGRAM(FLASH_STATE_ADDR,ucPageWriteState,8);
                        eDeviceState = DEVICE_MODE_IAP_WAITING;
                        /*NO data last MAX time for DEVICE_MODE_IAP_WAITING*/
                        usRunCount = 0;
                        usTimeOut = 200;
                    }
                }
            }
            else
            {
                eIapStatus = IAP_ERROR_DATA_CRC;
                xPackHandle.ucPackCRCState = 0x00;
                //Data received error
                vIapProcess(eIapStatus);
            }
        }
        break;
    
        default: break;
    }
    ulLength = 0;/*Clear ulLength*/
}
/*When no IAP progress opration correctly ,usRunCount increse, feed the whatchdog*/
    usRunCount++;

/*NO data last time exceed MAX time */
if(usRunCount > usTimeOut)	
{
    /*Check whether the device have load code success once*/
    ulData32 = *(__IO uint32_t *)FLASH_STATE_ADDR;
    if((uint8_t)(ulData32)  == 0x11)
    {
        /*Check if the Start Address is 0X08XXXXXX*/
        if(((*(vu32*)(FLASH_USER_START_ADDR+4))&0xFF000000)==0x08000000)
        {	
            eIapStatus = LAST_CODE_RUN;
            xPackHandle.ucPackCRCState = 0x00;
            /*LAST IAP Success code run*/
            vIapProcess(eIapStatus);
            usRunCount = 0;
            usTimeOut = 200;
            ucJumpToLoad = 1;
            iap_load_app(FLASH_USER_START_ADDR);
        }
        else
        {
            eIapStatus = IAP_ERROR_ADDRESS;
            xPackHandle.ucPackCRCState = 0x00;
            //Code Start Address error
            vIapProcess(eIapStatus);
            eDeviceState = DEVICE_MODE_IAP_WAITING;
            /*NO data last MAX time for DEVICE_MODE_IAP_WAITING*/
            usRunCount = 0;
            usTimeOut = 200;
        }
    }
    else
    {
        eIapStatus = IAP_ERROR_WAIT;
        xPackHandle.ucPackCRCState = 0x00;
        //Never load code success,wait for new code load
        vIapProcess(eIapStatus);
        eDeviceState = DEVICE_MODE_IAP_WAITING;
        /*NO data last MAX time for DEVICE_MODE_IAP_WAITING*/
        usRunCount = 0;
        usTimeOut = 200;
    }			
}	
}

//IAP State Set
void vIapStateSet(void)
{
uint8_t ucDataWrite[8];	
ulData32 = *(__IO uint32_t *)FLASH_STATE_ADDR;	
if(((uint8_t)(ulData32 >> 24)) != 0x22)
{	
    ucDataWrite[0] = 0x11;
    ucDataWrite[1] = 0x00;
    ucDataWrite[2] = 0x00;
    ucDataWrite[3] = 0x22;
    ucDataWrite[4] = 0x01;
    ucDataWrite[5] = 0x02;
    ucDataWrite[6] = 0x03;
    ucDataWrite[7] = 0x04;
    /*Write Message Page*/
    ucFLASH_USER_PAGE_PROGRAM(FLASH_STATE_ADDR,ucDataWrite,8);	
}
}





