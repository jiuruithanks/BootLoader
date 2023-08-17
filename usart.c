#include "usart.h"


/* ----------------------- Defines ------------------------------------------*/
#define USARTx                           LPUART1
#define USARTx_CLK_ENABLE                __HAL_RCC_LPUART1_CLK_ENABLE
#define USARTx_IRQn                      LPUART1_IRQn

#define USARTx_TX_PIN                    GPIO_PIN_1                
#define USARTx_TX_GPIO_PORT              GPIOC                       
#define USARTx_TX_GPIO_CLK_ENABLE        __HAL_RCC_GPIOC_CLK_ENABLE

#define USARTx_RX_PIN                    GPIO_PIN_0               
#define USARTx_RX_GPIO_PORT              GPIOC                    
#define USARTx_RX_GPIO_CLK_ENABLE        __HAL_RCC_GPIOC_CLK_ENABLE


#define USARTx_DE_PIN                    GPIO_PIN_1               
#define USARTx_DE_GPIO_PORT              GPIOB                    
#define USARTx_DE_GPIO_CLK_ENABLE        __HAL_RCC_GPIOB_CLK_ENABLE

UCHAR ucCriticalNesting = 0x00;
uint8_t USART_RX_BUF[USART_REC_LEN] __attribute__ ((at(0X20001000)));
uint8_t USART_DL_BUF[USART_REC_LEN];
uint8_t USART_TX_BUF[20];
uint32_t USART_RX_CNT=0;		//接收的字节数	
uint32_t USART_TX_CNT=0;		//发送的字节数	
uint32_t USART_TX_SIZE;
uint32_t ulOldCount=0;

/*RX TX Control*/
void vMBPortSerialEnable( BOOL xRxEnable, BOOL xTxEnable )
{
    ENTER_CRITICAL_SECTION(  );
    if( xRxEnable )
    {    
        USARTx->CR1 |= (USART_CR1_UE | USART_CR1_RE | USART_CR1_RXNEIE);
    }
    else
    {
        USARTx->CR1 &= ~(USART_CR1_RE | USART_CR1_RXNEIE);        
    }
    if( xTxEnable )
    {
        USARTx->CR1 &= ~(USART_CR1_TCIE | USART_CR1_TXEIE);        
        USARTx->CR1 |= ( USART_CR1_UE | USART_CR1_TE | USART_CR1_TCIE);
    }
    else
    {
        USARTx->CR1 &= ~(USART_CR1_TE | USART_CR1_TXEIE | USART_CR1_TCIE);
    }
    EXIT_CRITICAL_SECTION(  );
}

/*LPUART1 Init*/
BOOL xMBPortSerialInit( ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity )
{
	BOOL            bInitialized = TRUE;
	ULONG           ulTmpReg;
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    
    /* LPUART1 clock enable */
    USARTx_CLK_ENABLE();
  
    USARTx_TX_GPIO_CLK_ENABLE();
    
    USARTx_DE_GPIO_CLK_ENABLE();

    /**LPUART1 GPIO Configuration    
    PC0     ------> LPUART1_RX
    PC1     ------> LPUART1_TX 
    PB1     ------> LPUART1_DE
    */
    GPIO_InitStruct.Pin = USARTx_TX_PIN|USARTx_RX_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF8_LPUART1;
    HAL_GPIO_Init(USARTx_TX_GPIO_PORT, &GPIO_InitStruct);
	
//    GPIO_InitStruct.Pin = USARTx_DE_PIN;
//    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//    GPIO_InitStruct.Pull = GPIO_NOPULL;
//    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
//    HAL_GPIO_Init(USARTx_DE_GPIO_PORT, &GPIO_InitStruct);
//	  HAL_GPIO_WritePin(USARTx_DE_GPIO_PORT, USARTx_DE_PIN, GPIO_PIN_RESET);
	
    GPIO_InitStruct.Pin = USARTx_DE_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
    GPIO_InitStruct.Alternate = GPIO_AF8_LPUART1;
    HAL_GPIO_Init(USARTx_DE_GPIO_PORT, &GPIO_InitStruct);
    
    
    /* LPUART1 interrupt Init */
    HAL_NVIC_SetPriority(LPUART1_IRQn, 2, 1);
    HAL_NVIC_EnableIRQ(LPUART1_IRQn);
	
    /* Set the UART Communication parameters */    
    CLEAR_BIT(USARTx->CR1,USART_CR1_UE);

    CLEAR_BIT(USARTx->CR1,USART_CR1_M | USART_CR1_PCE | USART_CR1_PS | USART_CR1_TE | USART_CR1_RE);
	
	switch ( eParity )
    {
        case MB_PAR_NONE:
        {
            switch ( ucDataBits )
            {
                case 8:
                    SET_BIT(USARTx->CR1, UART_WORDLENGTH_8B | UART_PARITY_NONE);
                    break;
                case 7:
                    SET_BIT(USARTx->CR1, UART_WORDLENGTH_7B | UART_PARITY_NONE);
                    break;
                default:
                    bInitialized = FALSE;
                    break;
            }
            break;
        }
        case MB_PAR_ODD:
        {
            switch ( ucDataBits )
            {
                case 8:
                    SET_BIT(USARTx->CR1, UART_WORDLENGTH_9B | UART_PARITY_ODD);
                    break;
                case 7:
                    SET_BIT(USARTx->CR1, UART_WORDLENGTH_8B | UART_PARITY_ODD);
                    break;
                default:
                    bInitialized = FALSE;
                    break;
            }
            break;
        }
        case MB_PAR_EVEN:
        {
            switch ( ucDataBits )
            {
                case 8:
                    SET_BIT(USARTx->CR1, UART_WORDLENGTH_9B | UART_PARITY_EVEN);
                    break;
                case 7:
                    SET_BIT(USARTx->CR1, UART_WORDLENGTH_8B | UART_PARITY_EVEN);
                    break;
                default:
                    bInitialized = FALSE;
                    break;
            }
            break;
        }
    }
	
    CLEAR_BIT(USARTx->CR2,USART_CR2_STOP);
    SET_BIT(USARTx->CR2, UART_STOPBITS_1);	

    /* In asynchronous mode, the following bits must be kept cleared:
    - HDSEL bits in the USART_CR3 register.*/
    CLEAR_BIT(USARTx->CR3,USART_CR3_RTSE | USART_CR3_CTSE | USART_CR3_HDSEL);
    SET_BIT(USARTx->CR3, UART_HWCONTROL_NONE);
    
    /* Enable the Driver Enable mode by setting the DEM bit in the CR3 register */
    SET_BIT(USARTx->CR3, USART_CR3_DEM);
    
    /* Set the Driver Enable polarity */
    MODIFY_REG(USARTx->CR3, USART_CR3_DEP, UART_DE_POLARITY_HIGH);
 
    /* Set the Driver Enable assertion and deassertion times */
    ulTmpReg = (0x10 << UART_CR1_DEAT_ADDRESS_LSB_POS);
    ulTmpReg |= (0x01 << UART_CR1_DEDT_ADDRESS_LSB_POS);
    MODIFY_REG(USARTx->CR1, (USART_CR1_DEDT | USART_CR1_DEAT), ulTmpReg);    
    
    if((ulBaudRate <= 4800))
	{
		__HAL_RCC_LPUART1_CONFIG(RCC_LPUART1CLKSOURCE_LSE);
		WRITE_REG(USARTx->BRR, (uint32_t)(UART_DIV_LPUART(LSE_VALUE, ulBaudRate)));
	}
    else
	{
		__HAL_RCC_LPUART1_CONFIG(RCC_LPUART1CLKSOURCE_HSI);
		WRITE_REG(USARTx->BRR, (uint32_t)(UART_DIV_LPUART(HSI_VALUE, ulBaudRate)));
	}

	/*ENABLE RX , DISABLE TX*/
	vMBPortSerialEnable(TRUE ,FALSE);
    return bInitialized;
}

BOOL
xMBPortSerialPutByte( CHAR ucByte )
{
    USARTx->TDR = (ucByte & (USHORT)0x01FF);
    return TRUE;
}

BOOL
xMBPortSerialGetByte( CHAR * pucByte )
{
    *pucByte = (UCHAR)(USARTx->RDR & (UCHAR)0xFF);
    USARTx->ICR = USART_ICR_PECF | USART_ICR_FECF | USART_ICR_NECF | USART_ICR_ORECF;
    return TRUE;
}

uint32_t ulRx_State_Check(void) 
{
	uint32_t ulLength=0;
	if(USART_RX_CNT)
	{
		if(ulOldCount == USART_RX_CNT)
		{
			ulLength = USART_RX_CNT;
			ulOldCount=0;
			USART_RX_CNT=0;
			for(int i=0;i<ulLength;i++)
			{
				USART_DL_BUF[i] = USART_RX_BUF[i];
			}
			return ulLength;
		}
		else
		{
			ulOldCount = USART_RX_CNT;
		}
	}
	return 0;
}
	

void vLPUART1_Rx_IRQHandler(void)
{
	USART_RX_BUF[USART_RX_CNT++] = (uint8_t)(USARTx->RDR & (uint8_t)0xFF);
	if(eDeviceState == DEVICE_MODE_IAP_WAITING)
	{
		if(USART_RX_CNT >= 10)
		{
			/* disable the receive interrupt */
			vMBPortSerialEnable(FALSE,FALSE);    
		}
	}
	else
	{
		if(USART_RX_CNT >= USART_REC_LEN)
		{
			/* disable the receive interrupt */
			vMBPortSerialEnable(FALSE,FALSE);    
		}
	
	}

}

void vLPUART1_Tx_IRQHandler(void)
{
	USARTx->TDR = (USART_TX_BUF[USART_TX_CNT++] & (uint16_t)0x01FF);

	if(USART_TX_CNT >= USART_TX_SIZE)
	{
		/* disable the transmit interrupt */
		vMBPortSerialEnable(TRUE,FALSE);
	}


}

void EnterCriticalSection( void )
{
    if( ucCriticalNesting == 0 )
    {
        __disable_irq(  );
    }
    ucCriticalNesting++;
}

void ExitCriticalSection( void )
{
    ucCriticalNesting--;
    if( ucCriticalNesting == 0 )
    {
		__enable_irq(  );
    }
}




