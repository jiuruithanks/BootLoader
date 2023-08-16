
/*
 *file: clocks.h
 *Copyright: Copyright (c) 2019 Smart Drive Sensing(wuxi)Co. Ltd. All rights reserved.
 *Created on 2019-11-05  
 *Author:Xia Dengming
 *Version 1.0 
 *Title: System clock config
 */ 
 
#ifndef __CLOCKS_H__
#define __CLOCKS_H__

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"
	
void vSystemClockConfig1MHz(void);

void vSystemClockConfig4MHz(void);

void vSystemClockConfig16MHz(void);

void vSystemClockConfig20MHz(void);

void vSystemClockConfig80MHz(void); 

void vPeriphClockConfig(void);


#endif /*__CLOCKS_H__*/

/********** (C) COPYRIGHT Smart Drive Sensing(wuxi)Co. Ltd *****END OF FILE****/
