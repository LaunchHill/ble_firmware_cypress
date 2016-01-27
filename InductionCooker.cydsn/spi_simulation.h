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
#include "project.h"
#define KEY_BASE         0x22
#define CMD_POWER_H      0x22
#define CMD_POWER_L      0x23
#define CMD_PERIPHERAL_H 0x24
#define CMD_PERIPHERAL_L 0x25
#define CMD_LOADTEST     0x26
#define CMD_GETDEFAULTS  0x27
#define CMD_LOG          0x28
#define CMD_ID           0x29

#define CLB_CURBASE_H    0x32
#define CLB_CURBASE_L    0x33
#define CLB_CURFACT_H    0x34
#define CLB_CURFACT_L    0x35
#define CLB_VOLBASE_H    0x36
#define CLB_VOLBASE_L    0x37
#define CLB_VOLFACT_H    0x38
#define CLB_VOLFACT_L    0x39

#define LMT_RSPLENGTH    0x42
#define LMT_CURRENT_H    0x44
#define LMT_CURRENT_L    0x45
#define LMT_POWER_H      0x46
#define LMT_POWER_L      0x47
#define LMT_PULSE_H      0x48
#define LMT_PULSE_L      0x49
#define LMT_TEMPSINK_H   0x4a
#define LMT_TEMPSINK_L   0x4b
#define LMT_VOLTAGE_H    0x4c //?
#define LMT_VOLTAGE_L    0x4d

#define RSP_ERROR_H      0x62
#define RSP_ERROR_L      0x63
#define RSP_VOLTAGE_H    0x64
#define RSP_VOLTAGE_L    0x65
#define RSP_CURRENT_H    0x66
#define RSP_CURRENT_L    0x67
#define RSP_TEMPSINK_H   0x68
#define RSP_TEMPSINK_L   0x69
#define RSP_PULSE_H      0x6a
#define RSP_PULSE_L      0x6b
#define RSP_TEMPCOIL_H   0x6c
#define RSP_TEMPCOIL_L   0x6d
#define RSP_LOADTEST     0x6e
#define RSP_TOPOLOGY     0x6f
#define RSP_VERSION_H    0x72
#define RSP_VERSION_L    0x73
#define RSP_CLOCK_H      0x74
#define RSP_CLOCK_L      0x75
#define KEY_TOP          0x75

#define CMD_PERIPHERAL_FAN   0x01
#define CMD_PERIPHERAL_LIGHT  0x02
#define CMD_PERIPHERAL_BEEP  0x04

struct packet_header {
	uint8 vendor_h;
	uint8 vendor_l;
	uint8 packet_len;
	char packet_sum;
};

struct spi_packet {
	struct packet_header header;

	/* max 20 key value paires */
    uint8 data[20][2];    
};

//global
struct spi_packet spi_packet;

void cmd_execute_simulation(struct spi_packet *packet);
int packet_validation(struct spi_packet *packet);
void pack_from_raw_packet(struct spi_packet *packet,uint8 *packet_raw, int size);

void update_state(struct spi_packet *packet);


/* seconds set power on 1000Walt */
struct quirk {
    uint8 timeout;
    uint8 drink;
};

struct quirk QUIRK;

void quirk_start(uint16 power);
void quirk_stop();
void spi_send_response();

