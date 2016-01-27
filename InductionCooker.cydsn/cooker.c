#include "cooker.h"
static uint32 Cooker_Send(uint8 *data, uint32 count);
static uint32 Cooker_Receive(uint8 *data, uint32 count);

//static void Cooker_SendInitialPacket();
static void Cooker_ReceiveStatusPacket();
static void ih_control_set(uint8 bit, uint8 enabled);


uint8 cooker_status = COOKER_READY;

//initial packet
uint8 InitialBuffer[INIT_BUF_LEN] = {IH_CMD_INIT, 0x11,0x22,0x33,0x44,0x44,0x55,0x66,0x77};

//control cmd buffer
uint8 CtrlBuffer[CTRL_BUF_LEN] = {IH_CMD_READ_WRITE, 0xd2, 0x01, 0x00};

//read back status buffer
uint8 StatusBuffer[STATUS_BUF_LEN] = {};


void debug_led(uint8_t enabled)
{
    enabled == 0 ? Debug_led_Write(1) : Debug_led_Write(0); 
}

static void Cooker_ReceiveStatusPacket()
{
	uint8 s = 0x10;
    
	Cooker_Send(&s, 1);
	CyDelay(5);
	Cooker_Receive(StatusBuffer, STATUS_BUF_LEN);
}

static uint32 Cooker_Send(uint8 *data, uint32 count)
{

	uint32 status = TRANSFER_ERROR;

	I2C_I2CMasterWriteBuf(IH_ADDR, data, count, I2C_I2C_MODE_COMPLETE_XFER);
	/* Waits until master completes write transfer */
	while (0u == (I2C_I2CMasterStatus() & I2C_I2C_MSTAT_WR_CMPLT))
	{
	}

	/* Displays transfer status */
	if (0u == (I2C_I2C_MSTAT_ERR_XFER & I2C_I2CMasterStatus()))
	{
		/* Check if all bytes was written */
		if (I2C_I2CMasterGetWriteBufSize() == count)
		{
			status = TRANSFER_CMPLT;
		}
	}
	else
	{
	}
	(void) I2C_I2CMasterClearStatus();

	return (status);

}

static uint32 Cooker_Receive(uint8 *data, uint32 count)
{

	uint32 status = TRANSFER_ERROR;

	(void) I2C_I2CMasterReadBuf(IH_ADDR, data, count, \
								I2C_I2C_MODE_COMPLETE_XFER);

	/* Waits until master complete read transfer */
	while (0u == (I2C_I2CMasterStatus() & I2C_I2C_MSTAT_RD_CMPLT))
	{
	}

	/* Displays transfer status */
	if (0u == (I2C_I2C_MSTAT_ERR_XFER & I2C_I2CMasterStatus()))
	{
		/* Check packet structure */
		if ((I2C_I2CMasterGetReadBufSize() == count))
		{

			status = TRANSFER_CMPLT;
		}
	}
	else
	{
		status = TRANSFER_ERROR;
	}

	(void) I2C_I2CMasterClearStatus();

	return (status);
}

static void bit_set(uint8 *data, uint8 bit)
{
    *data |= 1 << bit;
}

static void bit_clear(uint8 *data, uint8 bit)
{
    *data &= ~(1 << bit);
}

static void ih_control_set(uint8 bit, uint8 enabled)
{
    uint8 low = CtrlBuffer[IH_ControlSet_POS] & 0xf;
    uint8 tmp;
    if (enabled) {
        bit_set(&low, bit);
    } else {
        bit_clear(&low, bit);
    }
    tmp = ((~low << 4) & 0xf0) | low;
    
    CtrlBuffer[IH_ControlSet_POS] =  tmp;
}

void set_beep(uint8 enabled)
{        
    ih_control_set(BIT_IH_ControlSet_BEEP, enabled);    
}

void set_fan(uint8 enabled)
{        
    ih_control_set(BIT_IH_ControlSet_FAN, enabled);   
}

void set_power(uint16 power)
{ 
    CtrlBuffer[2] = 0x10;
    CtrlBuffer[IH_PowerSetH_POS] = power / 10 / 25;
}

void cooker_send()
{
    Cooker_Send(CtrlBuffer, CTRL_BUF_LEN);
	CyDelay(3);
	Cooker_ReceiveStatusPacket();
	CyDelay(5);
}

#if 0
void cooker_process()
{ 
	switch(CURRENT_CMD) {

	case POWER_OFF:
		CtrlBuffer[IH_PowerSetL_POS] = 0x01;
        CtrlBuffer[IH_PowerSetL_POS] = 0x10;

		CtrlBuffer[IH_PowerSetH_POS] = 0x00;

		break;

	case POWER_ON:
		CtrlBuffer[IH_PowerSetL_POS] = 0x01;
        		CtrlBuffer[IH_PowerSetL_POS] = 0x10;

		CtrlBuffer[IH_PowerSetH_POS] = 0x00;

		break;

	case SET_POWER:
		CtrlBuffer[IH_PowerSetL_POS] = 0x10;
		CtrlBuffer[IH_PowerSetH_POS] = CMD_PARAM;

		break;

	default:
		break;
	}

	Cooker_Send(CtrlBuffer, CTRL_BUF_LEN);
	CyDelay(3);
	Cooker_ReceiveStatusPacket();
	CyDelay(5);
}
#endif
