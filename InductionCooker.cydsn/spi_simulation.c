#include "cooker.h"
#include "spi_simulation.h"
#include "project.h"

uint16 power;
uint8 peripheral;
    
void quirk_update(uint8 timeout)
{
   QUIRK.drink = 0;
   QUIRK.timeout = timeout;
   Timer_Stop();
   Timer_Start();
}

void quirk_stop()
{
    Timer_Stop();    
}

void quirk_start(uint16 power) {

    uint8 timeout;
    timeout = (power / 10) / 100;
    
    timeout = timeout < 1 ? 1 : timeout;
            
    if (timeout != QUIRK.timeout)
        quirk_update(timeout);
    
    if (QUIRK.drink < QUIRK.timeout) {
        set_power(10000);
        
    } else if (10 - QUIRK.drink > 0){
        set_power(0);
    } else {
        QUIRK.drink = 0;
    }
}

int packet_validation(struct spi_packet *pack)
{
	int i;
	char checksum = 0;
	unsigned char *data = pack->data[0];

	if (pack->header.packet_len < 4)
		return -1;

	checksum += pack->header.packet_len;
	checksum += pack->header.packet_sum;
	checksum += pack->header.vendor_h;
	checksum += pack->header.vendor_l;

	for (i =0; i < pack->header.packet_len - 4; i++) {
		checksum += data[i];
	}

	if (checksum != 0)
		return -1;

	/* vendor not match */
	if (pack->header.vendor_h != 'A' || pack->header.vendor_l != 'K' )
		return -1;

	return 0;
}

void spi_send_response()
{
	struct spi_packet response;
	CYBLE_GATT_HANDLE_VALUE_PAIR_T powerHandle;
	uint32 i,j;
	uint16 temp = 0;

	/* orgin protocol */
	response.header.vendor_h = 0x41;
	response.header.vendor_l = 0x4B;

	response.header.packet_len = 20;

	/* fill key value pair */

	temp = StatusBuffer[1] * 100;
	response.data[0][0] = RSP_VOLTAGE_L;
	response.data[0][1] = temp & 0xff;
	response.data[1][0] = RSP_VOLTAGE_H;
	response.data[1][1] = temp >> 8 & 0xff;

	temp = StatusBuffer[2] * 100;
	response.data[2][0] = RSP_CURRENT_L;
	response.data[2][1] = temp & 0xff;
	response.data[3][0] = RSP_CURRENT_H;
	response.data[3][1] = temp >> 8 & 0xff;

	temp = StatusBuffer[4] * 100;
	response.data[4][0] = RSP_TEMPCOIL_L;
	response.data[4][1] = temp & 0x00ff;
	response.data[5][0] = RSP_TEMPCOIL_H;
	response.data[5][1] = temp >> 8 & 0x00ff;

	temp = StatusBuffer[5] * 100;
	response.data[6][0] = RSP_TEMPSINK_L;
	response.data[6][1] = temp & 0xff;
	response.data[7][0] = RSP_TEMPSINK_H;
	response.data[7][1] = temp >> 4 & 0xff;

	/* checksum */
	for (i = 0; i < STATUS_BUF_LEN; i++) {
		for (j = 0; j < 2; j++) {
			response.header.packet_sum += response.data[i][j];
		}
	}
    
	powerHandle.attrHandle = CYBLE_HOT_PANNEL_SIMULATION_SERVICE_SPI_MISO_CHAR_HANDLE;
	powerHandle.value.val = (unsigned char *)&response;
	powerHandle.value.len = response.header.packet_len;
	powerHandle.value.actualLen = response.header.packet_len;

	CyBle_GattsNotification(cyBle_connHandle, &powerHandle);

	memset(&response, 0, sizeof(struct spi_packet));
    
}

void update_state(struct spi_packet *pack)
{
    uint8 key;
    uint8 value;
    uint8 *pair = pack->data[0];
     
    power = 0;  
    peripheral = 0;
    int i;

    for (i = 0; i < pack->header.packet_len - 4; i+=2, pair+=2) {
        key = pair[0];
        value = pair[1];
        
        switch(key) {    
        case CMD_POWER_H:
            power |= value << 8;           
            break;
            
        case CMD_POWER_L:
            power |= value;
            break;
            
        case CMD_PERIPHERAL_L:
            peripheral |= value;
            
            break;        
        default:
            break;
        }            
    }
    
    if (power > 0 && power < 10000) {
        quirk_start(power);
    } else {
        quirk_stop();
        set_power(power);
    }
    
    if (peripheral & CMD_PERIPHERAL_FAN)
        set_fan(1);
    else
        set_fan(0);
    
    if (peripheral & CMD_PERIPHERAL_BEEP)
        set_beep(1);
    else
        set_beep(0);
        
    cooker_send();
    
    spi_send_response();
}

void pack_from_raw_packet(struct spi_packet *packet, unsigned char *packet_raw, int size)
{
	packet->header.vendor_h = packet_raw[0];
	packet->header.vendor_l = packet_raw[1];
	packet->header.packet_len = packet_raw[2];
	packet->header.packet_sum = packet_raw[3];

	memcpy(packet->data, packet_raw + 4, size - 4);
}

