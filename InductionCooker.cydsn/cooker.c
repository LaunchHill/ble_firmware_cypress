/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

/* [] END OF FILE */
#include "cooker.h"


//initial packet
uint8 InitialBuffer[INIT_BUF_LEN] = {IH_CMD_INIT, 0x11,0x22,0x33,0x44,0x44,0x55,0x66,0x77};

//control cmd buffer
uint8 CtrlBuffer[CTRL_BUF_LEN] = {IH_CMD_READ_WRITE, 0x96, 0x01, 0x00};

//read back status buffer
uint8 StatusBuffer[STATUS_BUF_LEN] = {};



void Cooker_SendInitialPacket()
{
//    I2C_I2CMasterSendStart(IH_BOOSTRAP_WRITE, );
    Cooker_Send(InitialBuffer, 10);    
}


void Cooker_ReceiveStatusPacket()
{
    uint8 s = 0x10;
//    I2C_I2CMasterWriteByte(0x10);
//    I2C_I2CMasterWriteByte(0x5110);
     Cooker_Send(&s, 1);
     CyDelay(5);
     Cooker_Receive(StatusBuffer, STATUS_BUF_LEN);
    
}

//#if 0
//low level transfer api
uint32 Cooker_Send(uint8 *data, uint32 count) 
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

uint32 Cooker_Receive(uint8 *data, uint32 count) 
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
//#endif



//cooker protect (1): overheat
//               (2): no pan
//               (3): other error
void Cooker_Protect();

void cmd_loop()
{
//    CURRENT_CMD = SET_POWER;
    
    switch(CURRENT_CMD) {
        case POWER_OFF:
            CtrlBuffer[IH_ControlSet_POS] = 0x96;            
            CtrlBuffer[IH_ControlSet_POS] = 0xd2;
            CtrlBuffer[IH_PowerSetL_POS] = 0x01;
            CtrlBuffer[IH_PowerSetH_POS] = 0x00;
            
            break;
        
        case POWER_ON:
            CtrlBuffer[IH_ControlSet_POS] = 0x96;
            CtrlBuffer[IH_PowerSetL_POS] = 0x01;
            CtrlBuffer[IH_PowerSetH_POS] = 0x00;
            break;
        
        case SET_POWER:
            CtrlBuffer[IH_ControlSet_POS] = 0xb4;
            CtrlBuffer[IH_PowerSetL_POS] = 0x10;
            
            CtrlBuffer[IH_PowerSetH_POS] = CMD_PARAM;
/*            
            if (CMD_PARAM > POWER_MAX)
                CMD_PARAM = POWER_MAX;
            
            if (CMD_PARAM < POWER_MIN);
                CMD_PARAM = POWER_MIN;
*/            
//            CtrlBuffer[IH_PowerSetH_POS] = 0x20;   
            break;
        
        case SET_FAN:
            break;
        
        default:
            break;           
    }
    
    Cooker_Send(CtrlBuffer, CTRL_BUF_LEN);
    CyDelay(3);
    Cooker_ReceiveStatusPacket();
    CyDelay(100);
    //Fault Protect
/*    
    if ()
    {
    }
*/    
}