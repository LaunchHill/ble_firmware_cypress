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
	low_power_quirk();
}


CY_ISR_PROTO(SendFrameHandler);

void Initialize()
{
	CyGlobalIntEnable;   /* Enable global interrupts */
	/* Place your initialization/startup code here (e.g. MyInst_Start()) */

	CyBle_Start( StackEventHandler );
	I2C_scl_SetDriveMode(I2C_scl_DM_RES_UP);
	I2C_sda_SetDriveMode(I2C_sda_DM_RES_UP);
	I2C_Start();


	CySysWdtWriteMatch(CY_SYS_WDT_COUNTER1, WDT_COUNT1_MATCH);
	CySysWdtWriteMode(CY_SYS_WDT_COUNTER1, CY_SYS_WDT_MODE_RESET);
	CySysWdtWriteClearOnMatch(CY_SYS_WDT_COUNTER1, 1u);
	CySysWdtWriteCascade(CY_SYS_WDT_CASCADE_01);
	CySysWdtEnable(CY_SYS_WDT_COUNTER1_MASK | CY_SYS_WDT_COUNTER0_MASK);

	isr_Timer_StartEx(quirk_handle);
	//Timer_Start();
}

int main()
{
	Initialize();
	for(;;)
	{
		cmd_loop();
		if (connected) {
			/* if (strncmp(PrevStatusBuffer, StatusBuffer, STATUS_BUF_LEN) == 0) { */
				spi_send();
			/* } */

		}
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

	case CYBLE_EVT_HARDWARE_ERROR:
		break;

	case CYBLE_EVT_HCI_STATUS:
		break;

	case CYBLE_EVT_STACK_BUSY_STATUS:
		break;

	case CYBLE_EVT_PENDING_FLASH_WRITE:
		break;


		/* GAP events */

	case CYBLE_EVT_GAP_AUTH_REQ:
		break;

	case CYBLE_EVT_GAP_PASSKEY_ENTRY_REQUEST:
		break;

	case CYBLE_EVT_GAP_PASSKEY_DISPLAY_REQUEST:
		break;

	case CYBLE_EVT_GAP_AUTH_COMPLETE:
		break;

	case CYBLE_EVT_GAP_AUTH_FAILED:
		break;

	case CYBLE_EVT_GAP_DEVICE_CONNECTED:
		connected = 1;

		break;

	case CYBLE_EVT_GAP_DEVICE_DISCONNECTED:
        CURRENT_CMD = POWER_OFF;
		quirk_off();
		CyBle_GappStartAdvertisement(CYBLE_ADVERTISING_FAST);
		connected = 0;
		break;

	case CYBLE_EVT_GAP_ENCRYPT_CHANGE:
		break;

	case CYBLE_EVT_GAP_CONNECTION_UPDATE_COMPLETE:
		break;

	case CYBLE_EVT_GAP_KEYINFO_EXCHNGE_CMPLT:
		break;


		/* GAP Peripheral events */

	case CYBLE_EVT_GAPP_ADVERTISEMENT_START_STOP:
		break;


		/* GAP Central events */

	case CYBLE_EVT_GAPC_SCAN_PROGRESS_RESULT:
		break;

	case CYBLE_EVT_GAPC_SCAN_START_STOP:
		break;


		/* GATT events */

	case CYBLE_EVT_GATT_CONNECT_IND:
		break;

	case CYBLE_EVT_GATT_DISCONNECT_IND:
		break;


		/* GATT Client events (CYBLE_EVENT_T) */

	case CYBLE_EVT_GATTC_ERROR_RSP:
		break;

	case CYBLE_EVT_GATTC_XCHNG_MTU_RSP:
		break;

	case CYBLE_EVT_GATTC_READ_BY_GROUP_TYPE_RSP:
		break;

	case CYBLE_EVT_GATTC_READ_BY_TYPE_RSP:
		break;

	case CYBLE_EVT_GATTC_FIND_INFO_RSP:
		break;

	case CYBLE_EVT_GATTC_FIND_BY_TYPE_VALUE_RSP:
		break;

	case CYBLE_EVT_GATTC_READ_RSP:
		break;

	case CYBLE_EVT_GATTC_READ_BLOB_RSP:
		break;

	case CYBLE_EVT_GATTC_READ_MULTI_RSP:
		break;

	case CYBLE_EVT_GATTC_WRITE_RSP:
		break;

	case CYBLE_EVT_GATTC_EXEC_WRITE_RSP:
		break;

	case CYBLE_EVT_GATTC_HANDLE_VALUE_NTF:
		break;

	case CYBLE_EVT_GATTC_HANDLE_VALUE_IND:
		break;


		/* GATT Client events (CYBLE_EVT_T) */

	case CYBLE_EVT_GATTC_INDICATION:
		break;

	case CYBLE_EVT_GATTC_SRVC_DISCOVERY_FAILED:
		break;

	case CYBLE_EVT_GATTC_INCL_DISCOVERY_FAILED:
		break;

	case CYBLE_EVT_GATTC_CHAR_DISCOVERY_FAILED:
		break;

	case CYBLE_EVT_GATTC_DESCR_DISCOVERY_FAILED:
		break;

	case CYBLE_EVT_GATTC_SRVC_DUPLICATION:
		break;

	case CYBLE_EVT_GATTC_CHAR_DUPLICATION:
		break;

	case CYBLE_EVT_GATTC_DESCR_DUPLICATION:
		break;

	case CYBLE_EVT_GATTC_SRVC_DISCOVERY_COMPLETE:
		break;

	case CYBLE_EVT_GATTC_INCL_DISCOVERY_COMPLETE:
		break;

	case CYBLE_EVT_GATTC_CHAR_DISCOVERY_COMPLETE:
		break;

	case CYBLE_EVT_GATTC_DISCOVERY_COMPLETE:
		break;


		/* GATT Server events (CYBLE_EVENT_T) */

	case CYBLE_EVT_GATTS_XCNHG_MTU_REQ:
		break;

	case CYBLE_EVT_GATTS_WRITE_REQ:
		wrReqParam =  (CYBLE_GATTS_WRITE_REQ_PARAM_T *)eventParam;
		if (wrReqParam->handleValPair.attrHandle == CYBLE_HOT_PANNEL_SIMULATION_SERVICE_SPI_MOSI_CHAR_HANDLE) {
			spi_packet_raw = wrReqParam->handleValPair.value.val;

			if (wrReqParam->handleValPair.value.len >= 4)
				pack_from_raw_packet(&spi_packet, wrReqParam->handleValPair.value.val, wrReqParam->handleValPair.value.len);

			if (packet_validation(&spi_packet) == 0)
				cmd_execute_simulation(&spi_packet);
		}

		CyBle_GattsWriteRsp(cyBle_connHandle);

		break;

	case CYBLE_EVT_GATTS_WRITE_CMD_REQ:
		break;

	case CYBLE_EVT_GATTS_PREP_WRITE_REQ:

		break;

	case CYBLE_EVT_GATTS_EXEC_WRITE_REQ:

		break;

	case CYBLE_EVT_GATTS_HANDLE_VALUE_CNF:
		break;

	case CYBLE_EVT_GATTS_DATA_SIGNED_CMD_REQ:
		break;


		/* GATT Server events (CYBLE_EVT_T) */

	case CYBLE_EVT_GATTS_INDICATION_ENABLED:
		break;

	case CYBLE_EVT_GATTS_INDICATION_DISABLED:
		break;


		/* L2CAP events */

	case CYBLE_EVT_L2CAP_CONN_PARAM_UPDATE_REQ:
		break;

	case CYBLE_EVT_L2CAP_CONN_PARAM_UPDATE_RSP:
		break;

	case CYBLE_EVT_L2CAP_COMMAND_REJ:
		break;

	case CYBLE_EVT_L2CAP_CBFC_CONN_IND:
		break;

	case CYBLE_EVT_L2CAP_CBFC_CONN_CNF:
		break;

	case CYBLE_EVT_L2CAP_CBFC_DISCONN_IND:
		break;

	case CYBLE_EVT_L2CAP_CBFC_DISCONN_CNF:
		break;

	case CYBLE_EVT_L2CAP_CBFC_DATA_READ:
		break;

	case CYBLE_EVT_L2CAP_CBFC_RX_CREDIT_IND:
		break;

	case CYBLE_EVT_L2CAP_CBFC_TX_CREDIT_IND:
		break;

	case CYBLE_EVT_L2CAP_CBFC_DATA_WRITE_IND:
		break;


		/* default catch-all case */

	default:
		break;
	}
}
