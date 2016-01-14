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
