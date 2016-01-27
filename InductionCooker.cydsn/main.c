#include <project.h>

#include "cooker.h"

#include <stdio.h>
#include <stdlib.h>
#include "spi_simulation.h"

#define WDT_COUNT0_MATCH    (0x29AAu)

#define WDT_COUNT1_MATCH    (0x0003u)

void StackEventHandler( uint32 eventCode, void *eventParam );

CYBLE_GATT_HANDLE_VALUE_PAIR_T      cmdHandle;
int connected = 0;
uint8 *spi_packet_raw;

struct spi_packet packet;

CY_ISR(quirk_handle)
{
	Timer_ReadStatusRegister();
	QUIRK.drink++;

}

void timeoutCallback()
{
    CySysWdtClearInterrupt(CY_SYS_WDT_COUNTER1_INT);
    
}

   
void Initialize()
{
	CyGlobalIntEnable;
    
	CyBle_Start( StackEventHandler );
	I2C_scl_SetDriveMode(I2C_scl_DM_RES_UP);
	I2C_sda_SetDriveMode(I2C_sda_DM_RES_UP);
	I2C_Start();

	CySysWdtWriteMatch(CY_SYS_WDT_COUNTER1, WDT_COUNT1_MATCH);
	CySysWdtWriteMode(CY_SYS_WDT_COUNTER1, CY_SYS_WDT_MODE_INT_RESET);
	CySysWdtWriteClearOnMatch(CY_SYS_WDT_COUNTER1, 1u);
	CySysWdtWriteCascade(CY_SYS_WDT_CASCADE_01);
//    CySysWdtSetInterruptCallback(CY_SYS_WDT_COUNTER1, timeoutCallback);
	CySysWdtEnable(CY_SYS_WDT_COUNTER1_MASK | CY_SYS_WDT_COUNTER0_MASK);
    CySysWdtSetInterruptCallback(CY_SYS_WDT_COUNTER1_MASK, timeoutCallback);
	isr_Timer_StartEx(quirk_handle);    
    
}

int main()
{
	Initialize();

    cooker_send();
	for(;;)
	{
		CyBle_ProcessEvents();
        CySysWatchdogFeed(CY_SYS_WDT_COUNTER1);

	}
}

void StackEventHandler( uint32 eventCode, void *eventParam )
{
	CYBLE_GATTS_WRITE_REQ_PARAM_T *wrReqParam;
	switch( eventCode )
	{
		/* Generic events */

	case CYBLE_EVT_HOST_INVALID:
		break;

	case CYBLE_EVT_STACK_ON:
		CyBle_GappStartAdvertisement( CYBLE_ADVERTISING_FAST );

		break;

	case CYBLE_EVT_TIMEOUT:
		break;

	case CYBLE_EVT_GAP_DEVICE_CONNECTED:
		connected = 1;

		break;

	case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:
        set_power(0);
        quirk_stop();
        
		CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
		connected = 0;
		break;

	case CYBLE_EVT_GATTS_WRITE_REQ:
    		wrReqParam =  (CYBLE_GATTS_WRITE_REQ_PARAM_T *)eventParam;
    		if (wrReqParam->handleValPair.attrHandle == CYBLE_HOT_PANNEL_SIMULATION_SERVICE_SPI_MOSI_CHAR_HANDLE) {
    			spi_packet_raw = wrReqParam->handleValPair.value.val;

    			if (wrReqParam->handleValPair.value.len >= 4)
    				pack_from_raw_packet(&spi_packet, wrReqParam->handleValPair.value.val, wrReqParam->handleValPair.value.len);
                    
    			if (packet_validation(&spi_packet) == 0)
    				update_state(&spi_packet);                    
            }        
		CyBle_GattsWriteRsp(cyBle_connHandle);
		break;

	default:
		break;
	}
}
