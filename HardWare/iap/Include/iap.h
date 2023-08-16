#ifndef __IAP_H__
#define __IAP_H__
#include "sys.h"  
#include "usart.h"
#include "clocks.h"
#include "dataprocess.h"
#include "stmflash.h"
#include "stm32l4xx_hal.h"   
typedef  void (*iapfun)(void);				//Define parameters of a function type  							
void iap_load_app(uint32_t appxaddr);		//Jump to APP
void vDeviceSLeep( void );					//Sleep mode
void vDeviceRunControl(void);				//Device run control program
void vIapStateSet(void);	                //IAP State Set
#endif







































