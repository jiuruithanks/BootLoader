#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "stm32l4xx_hal.h"
#include "dataprocess.h"
#include <stdio.h>

#define ENTER_CRITICAL_SECTION( )   EnterCriticalSection( )
#define EXIT_CRITICAL_SECTION( )    ExitCriticalSection( )

#ifndef TRUE
#define TRUE            1
#endif

#ifndef FALSE
#define FALSE           0
#endif

#define USART_REC_LEN  			3*1024 		 //MAX RX SIZE
		
extern uint8_t  USART_RX_BUF[USART_REC_LEN]; //USART RX BUFF
extern uint8_t  USART_DL_BUF[USART_REC_LEN]; //DownLoad data handle BUFF
extern uint8_t  USART_TX_BUF[20]; 			 //USART TX BUFF
extern uint32_t USART_RX_CNT;				 //USART RX COUNT 
extern uint32_t USART_TX_CNT;				 //USART TX COUNT
extern uint32_t USART_TX_SIZE;				 //USART TX SIZE

typedef char    BOOL;
typedef unsigned char UCHAR;
typedef char    CHAR;
typedef unsigned short USHORT;
typedef short   SHORT;
typedef unsigned long ULONG;
typedef long    LONG;

typedef enum
{
    MB_PAR_NONE,                /*!< No parity. */
    MB_PAR_ODD,                 /*!< Odd parity. */
    MB_PAR_EVEN                 /*!< Even parity. */
} eMBParity;

void vMBPortSerialEnable( BOOL xRxEnable, BOOL xTxEnable );						//RX TX Control
BOOL xMBPortSerialInit( ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity );//LPUART1 Init

BOOL xMBPortSerialPutByte( CHAR ucByte );
BOOL xMBPortSerialGetByte( CHAR * pucByte );

uint32_t ulRx_State_Check(void);
void vLPUART1_Rx_IRQHandler(void);
void vLPUART1_Tx_IRQHandler(void);

void            EnterCriticalSection( void );
void            ExitCriticalSection( void );

#endif


