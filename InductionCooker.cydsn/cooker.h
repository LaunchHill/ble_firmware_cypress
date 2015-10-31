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
#include <project.h>

#define Cooker_Power_Max ()
#define Cooker_Power_Min ()

#define TRANSFER_ERROR (-1)
#define TRANSFER_CMPLT (0u)

/* packet format
 * S 1Byte 1Byte n Byte(0<n<16) P
 * S: i2c start signal
 * Addr byte
 * Cmd byte
 * Data bytes
*/

#define IH_ADDR 0x28

#define IH_CMD_INIT 0x00
#define IH_CMD_READ_WRITE 0x10



#define CMD_POWERON    (0)
#define CMD_POWEROFF   (2)
#define CMD_POWERCHANGE (1)

//control packet format
//S 0x50 0x10 ControlSet PowerSetL PowerSetH P

//read status packet format
//S 0x50 0x00 S LoadTest MaxPPG MaxPowerL MaxPowerH LoadLeave LoadCUR CurrentAdjust MaxPowerM P


//IH
#define IH_ControlSet_POS   (1)
#define IH_PowerSetL_POS    (2)
#define IH_PowerSetH_POS    (3)

#define IH_IHStatus_POS     (0)
#define IH_VoltageValue_POS (1)
#define IH_CurrentValue_POS (2)
#define IH_IGBTValue_POS    (3)
#define IH_TOPValue_POS     (4)

#define INIT_BUF_LEN 9
#define CTRL_BUF_LEN 4
#define STATUS_BUF_LEN 5

#define POWER_MIN 20
#define POWER_MAX 80

uint8 InitialBuffer[INIT_BUF_LEN];
uint8 CtrlBuffer[CTRL_BUF_LEN];
uint8 StatusBuffer[STATUS_BUF_LEN];

// VoltageValue CurrentValue  TOPValue
 uint8 sensorData[3];

uint8 CURRENT_CMD;
uint8 CMD_PARAM;


enum {
    Cooker_OK = 0b0000,
    Cooker_Reserved = 0b0001,
    Cooker_FAULT_ELECTRICAL = 0b0010, 
    Cooker_FAULT_IGBT_OPEN = 0b0011,
    Cooker_FAULT_IGBT_SHORT_OR_OVERHEAT = 0b0100,
    Cooker_FAULT_Surface_OPEN = 0b0101,
    Cooker_FAULT_Surface_SHORT_OR_OVERHEAT = 0b0110,
    Cooker_FAULT_Surface_SENSOR_LOSE = 0b0111,
    
}FaultCode;

enum {
    POWER_OFF,
    POWER_ON,
    SET_POWER,
    SET_FAN,
}CMD;



uint32 Cooker_Send(uint8 *data, uint32 count);
uint32 Cooker_Receive(uint8 *data, uint32 count);

void Cooker_SendInitialPacket();
void Cooker_ReceiveStatusPacket();
void cmd_loop();

