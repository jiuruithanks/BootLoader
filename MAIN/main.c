/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "clocks.h"
#include "usart.h"
#include "iap.h"
#include "dataprocess.h"
#include "aes.h"

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
	/*Input clock is 32KHz LSI, prescaler is 32,so the IWDG ckock is 1KHz, Reload value is 500
      so the IWDG Reset Time is about 500ms*/
    HAL_Init(  );
    HAL_Delay( 100 );
    /*Wait for the VDD reach 2.8V*/
    while( __HAL_PWR_GET_FLAG (PWR_FLAG_PVDO) != 0);

    /*Set the system clock to 4MHz to run the init program*/    
    vSystemClockConfig4MHz(  );
	/*Config the Periph Clock*/
    vPeriphClockConfig(  );   

#if (DEVICE_EXTERNAL_SMPS_USAGE != 0)   
    HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE2);   
#else
    HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);
#endif
	
	 /* Ensure that MSI is wake-up system clock */ 
    __HAL_RCC_WAKEUPSTOP_CLK_CONFIG(RCC_STOP_WAKEUPCLOCK_MSI);
	
	/*USART Init*/
	xMBPortSerialInit(57600,8,MB_PAR_NONE);
		
	vSystemClockConfig16MHz(  );
	
	/*Set version if no code*/
	vIapStateSet(  );
	
	/**/		
    while(1)
    {	
		vDeviceRunControl(  );
		HAL_Delay(10);	
    }
}


