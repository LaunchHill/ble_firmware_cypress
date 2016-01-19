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
#include "spi_simulation.h"
#include "project.h"
uint16 power_prev = 0;

void quirk_reset(int duration)
{

	if (QUIRK.QUIRK_ON != duration) {
		QUIRK.QUIRK_ON = duration;
		QUIRK.QUIRK_OFF = 10 - duration;

		QUIRK.CURRENT_ON = QUIRK.QUIRK_ON;
		QUIRK.CURRENT_OFF = QUIRK.QUIRK_OFF;
		Timer_Start();
	}
}

void quirk_reload()
{
	QUIRK.CURRENT_ON = QUIRK.QUIRK_ON;
	QUIRK.CURRENT_OFF = QUIRK.QUIRK_OFF;
}

void quirk_off()
{
	QUIRK.QUIRK_OFF = 0;
	QUIRK.QUIRK_ON = 0;
	Timer_Stop();
}

void low_power_quirk()
{
	/* 1000 walt on off */
	CMD_PARAM = 40;
	//on
	if (QUIRK.CURRENT_ON-- > 0) {
		CURRENT_CMD = SET_POWER;

		//off
	} else if (QUIRK.CURRENT_OFF-- > 0){
		CMD_PARAM = 0;
		CURRENT_CMD = SET_POWER;
	} else {
		quirk_reload();
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

void cmd_execute_simulation(struct spi_packet *pack)
{
	int i;
	uint16 power = 0;

	uint8 quirk;

	unsigned char *data = pack->data[0];

	for (i = 0; i< pack->header.packet_len - 4; i+=2, data+=2) {
		switch(data[0]) {
			/* POWER_L and POWER_H must be all appeared */
		case CMD_POWER_H:
			if (data[1] != 0)
				power = data[1] << 8 & 0xff00;
			else
				CURRENT_CMD = POWER_OFF;

			break;

		case CMD_POWER_L:
			if (power != 0) {
				power |= (data[1] & 0xff);
				/* power resolution is 0.1W */
				if (power < 10000 ) {
					quirk = (power / 10) / 100;
					quirk = quirk < 1 ? 1 : quirk;
					quirk_reset(quirk);
				} else {
					quirk_off();
					CURRENT_CMD = SET_POWER;
					CMD_PARAM = power / 250;
				}

			} else {
				CURRENT_CMD = POWER_OFF;
				quirk_off();
			}

			break;

#if 0
		case CMD_POWER_L:
			if (power != 0) {
				power |= (data[1] & 0xff);
				CURRENT_CMD = SET_POWER;
				CMD_PARAM = power / 250;
			} else {
				CURRENT_CMD = POWER_OFF;
			}
			break;
#endif
		default:
			break;
		}
	}
}

void pack_from_raw_packet(struct spi_packet *packet, unsigned char *packet_raw, int size)
{
	packet->header.vendor_h = packet_raw[0];
	packet->header.vendor_l = packet_raw[1];
	packet->header.packet_len = packet_raw[2];
	packet->header.packet_sum = packet_raw[3];

	memcpy(packet->data, packet_raw + 4, size - 4);
}

void spi_send()
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

	/* workround for transmission double check */
    /* memcpy(PrevStatusBuffer, StatusBuffer, STATUS_BUF_LEN); */

}
