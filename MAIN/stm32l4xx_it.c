/**
  ******************************************************************************
  * @file    FLASH/FLASH_EraseProgram/Src/stm32l4xx_it.c
  * @author  MCD Application Team
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32l4xx_it.h"
#include "usart.h"

/** @addtogroup STM32L4xx_HAL_Examples
  * @{
  */

/** @addtogroup FLASH_EraseProgram
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
  HAL_IncTick();
}

/******************************************************************************/
/*                 STM32L4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32l4xx.s).                                         */
/******************************************************************************/

void PVD_PVM_IRQHandler(void)
{
   HAL_PWREx_PVD_PVM_IRQHandler();
}


/**
  * @brief This function handles RTC wake-up interrupt through EXTI line 20.
  */
//void RTC_WKUP_IRQHandler(void)
//{
//    vRtctimerISR(  );
//}


/**
  * @brief This function handles LPUART1 global interrupt.
  */
void LPUART1_IRQHandler(void)
{
    uint32_t ulIsrFlags   = READ_REG(LPUART1->ISR);
    uint32_t ulCr1Its     = READ_REG(LPUART1->CR1);

    if (((ulIsrFlags & (USART_ISR_RXNE | USART_ISR_ORE)) != 0U)
        && ((ulCr1Its & (USART_CR1_RXNEIE | USART_CR1_RE)) != 0U))
    {
       vLPUART1_Rx_IRQHandler();
    }
	
  /* UART in mode Transmitter ------------------------------------------------*/
    if (((ulIsrFlags & (USART_ISR_TXE | USART_ISR_TC)) != 0U)
      && ((ulCr1Its & (USART_CR1_TE | USART_CR1_TCIE | USART_CR1_TXEIE)) != 0U))

    {
        vLPUART1_Tx_IRQHandler();
    }
}


/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
