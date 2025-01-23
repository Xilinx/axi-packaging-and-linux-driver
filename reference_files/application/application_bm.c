/*
Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.
SPDX-License-Identifier: MIT
*/
#include "application_bm.h"
#include <xil_types.h>
#include <xil_printf.h>
#include <xil_io.h>
#include "axi_1wire_host.h"
#include "xparameters.h"

#define ba XPAR_AXI_1WIRE_HOST_0_BASEADDR

void thermistor_config(s8 t_high, s8 t_low, int resolution) {
    u8 config;
    if (t_high <= t_low){
        xil_printf( "The highest temperature trigger value must be higher than the lowest temperature trigger value.\n\r");
    }
    else if (resolution > 12 || resolution < 9){
        xil_printf( "The valid resolution bits value are: 9, 10, 11 or 12.\n\r");
    }
    else {
        switch (resolution) {
			case 9:
				config = 0x1F;
				break;
			case 10:
				config = 0x3F;
				break;
			case 11:
				config = 0x5F;
				break;
			case 12:
				config = 0x7F;
				break;
			default:
				xil_printf( "Error when configuring resolution, will be using resolution of 9 bits.\n\r");
				config = 0x1F;
				break;
        }
        // Initialization

        if (AXI_1WIRE_HOST_ResetBus(ba) != 1){
        	// Skip ROM command
			AXI_1WIRE_HOST_WriteByte(ba, 0xCC);
			// Write Scratchpad
			AXI_1WIRE_HOST_WriteByte(ba, 0x4E);
			// Send 3 bytes (Temp_high, Temp_low, config)
			AXI_1WIRE_HOST_WriteByte(ba, t_high);
			AXI_1WIRE_HOST_WriteByte(ba, t_low);
			AXI_1WIRE_HOST_WriteByte(ba, config);
			// Initialization
			if (AXI_1WIRE_HOST_ResetBus(ba) != 1){
				// Skip ROM command
				AXI_1WIRE_HOST_WriteByte(ba, 0xCC);
				// Copy scratchpad to EEPROM
				AXI_1WIRE_HOST_WriteByte(ba, 0x48);
				// Read Bit until 1 received from device
				while(AXI_1WIRE_HOST_TouchBit(ba, 0x01) == 0){}
			}
			else {
				xil_printf( "Error no device detected.\n\r");
			}

        }
        else {
        	xil_printf( "Error no device detected.\n\r");
        }

    }
}

void thermistor_temp_reading(u8* byte0, u8* byte1, u8* byte2, u8* byte3, u8* byte4, u8* byte5, u8* byte6, u8* byte7, u8* byte8) {
    // Initialization
    if (AXI_1WIRE_HOST_ResetBus(ba) != 1){
    	// Skip ROM command
		AXI_1WIRE_HOST_WriteByte(ba, 0xCC);
		// Convert temperature command
		AXI_1WIRE_HOST_WriteByte(ba, 0x44);
		// Read Bit until 1 receive from device
		while(AXI_1WIRE_HOST_TouchBit(ba, 1) == 0){}
		// Initialization
		if (AXI_1WIRE_HOST_ResetBus(ba) != 1){
			// Skip ROM command
			AXI_1WIRE_HOST_WriteByte(ba, 0xCC);
			// Read Scratchpad command
			AXI_1WIRE_HOST_WriteByte(ba, 0xBE);
			// Read 2 Bytes of temperature
			*byte0 = AXI_1WIRE_HOST_ReadByte(ba);
			*byte1 = AXI_1WIRE_HOST_ReadByte(ba);
			// Read 3 Bytes of config
			*byte2 = AXI_1WIRE_HOST_ReadByte(ba);
			*byte3 = AXI_1WIRE_HOST_ReadByte(ba);
			*byte4 = AXI_1WIRE_HOST_ReadByte(ba);
			// Read 3 reserved Bytes
			*byte5 = AXI_1WIRE_HOST_ReadByte(ba);
			*byte6 = AXI_1WIRE_HOST_ReadByte(ba);
			*byte7 = AXI_1WIRE_HOST_ReadByte(ba);
			// Read CRC
			*byte8 = AXI_1WIRE_HOST_ReadByte(ba);
		}
		else {
			xil_printf( "Error no device detected.\n\r");
		}
    }
	else {
		xil_printf( "Error no device detected.\n\r");
	}
}

void continuous_temperature_reading(s8 t_high, s8 t_low, int resolution) {
	u8 crc_table[256] = {0, 94, 188, 226, 97, 63, 221, 131, 194, 156, 126, 32, 163, 253, 31, 65,
				157, 195, 33, 127, 252, 162, 64, 30, 95, 1, 227, 189, 62, 96, 130, 220,
				35, 125, 159, 193, 66, 28, 254, 160, 225, 191, 93, 3, 128, 222, 60, 98,
				190, 224, 2, 92, 223, 129, 99, 61, 124, 34, 192, 158, 29, 67, 161, 255,
				70, 24, 250, 164, 39, 121, 155, 197, 132, 218, 56, 102, 229, 187, 89, 7,
				219, 133, 103, 57, 186, 228, 6, 88, 25, 71, 165, 251, 120, 38, 196, 154,
				101, 59, 217, 135, 4, 90, 184, 230, 167, 249, 27, 69, 198, 152, 122, 36,
				248, 166, 68, 26, 153, 199, 37, 123, 58, 100, 134, 216, 91, 5, 231, 185,
				140, 210, 48, 110, 237, 179, 81, 15, 78, 16, 242, 172, 47, 113, 147, 205,
				17, 79, 173, 243, 112, 46, 204, 146, 211, 141, 111, 49, 178, 236, 14, 80,
				175, 241, 19, 77, 206, 144, 114, 44, 109, 51, 209, 143, 12, 82, 176, 238,
				50, 108, 142, 208, 83, 13, 239, 177, 240, 174, 76, 18, 145, 207, 45, 115,
				202, 148, 118, 40, 171, 245, 23, 73, 8, 86, 180, 234, 105, 55, 213, 139,
				87, 9, 235, 181, 54, 104, 138, 212, 149, 203, 41, 119, 244, 170, 72, 22,
				233, 183, 85, 11, 136, 214, 52, 106, 43, 117, 151, 201, 74, 20, 246, 168,
				116, 42, 200, 150, 21, 75, 169, 247, 182, 232, 10, 84, 215, 137, 107, 53};

	u8 byte0_read, byte1_read, byte2_read, byte3_read, byte4_read, byte5_read, byte6_read, byte7_read, byte8_read;
	u16 bytes_read;
	int dec_p, int_p;
	u8 crc;

	thermistor_config(t_high, t_low, resolution);
	xil_printf("Configuration done\n\r");

	while (1){
		thermistor_temp_reading(&byte0_read, &byte1_read, &byte2_read, &byte3_read, &byte4_read, &byte5_read, &byte6_read, &byte7_read, &byte8_read);
		// Verify CRC
		crc = crc_table[byte0_read];
		crc = crc_table[byte1_read ^ crc];
		crc = crc_table[byte2_read ^ crc];
		crc = crc_table[byte3_read ^ crc];
		crc = crc_table[byte4_read ^ crc];
		crc = crc_table[byte5_read ^ crc];
		crc = crc_table[byte6_read ^ crc];
		crc = crc_table[byte7_read ^ crc];

		if (crc == byte8_read){
			// Concatenate both bytes and apply 2 complement
			if ((byte1_read & 0x80) != 0){
				bytes_read = (((byte1_read << 8) + byte0_read) ^ 0xFFFF) + 1;
			}
			else {
				bytes_read = ((byte1_read << 8) +  byte0_read);
			}

			// Get the integer part
			if ((byte1_read & 0x80) != 0){
				int_p = -1 * (bytes_read >> 4);
			}
			else {
				int_p = bytes_read >> 4;
			}

			// Get the decimal part
			switch (resolution) {
				case 9:
					dec_p = (((bytes_read & 0x8) >> 3) * 5000);
					break;
				case 10:
					dec_p = (((bytes_read & 0x8) >> 3) * 5000) + (((bytes_read & 0x4) >> 2) * 2500);
					break;
				case 11:
					dec_p = (((bytes_read & 0x8) >> 3) * 5000) + (((bytes_read & 0x4) >> 2) * 2500)
						+ (((bytes_read & 0x2) >> 1) * 1250);
					break;
				case 12:
					dec_p = (((bytes_read & 0x8) >> 3) * 5000) + (((bytes_read & 0x4) >> 2) * 2500)
						+ (((bytes_read & 0x2) >> 1) * 1250) + ((bytes_read & 0x1) * 625);
					break;
				default:
					xil_printf( "Error when configuring resolution, will be assume resolution of 9 bits.\n\r");
					dec_p = (((bytes_read & 0x8) >> 3) * 5000);
					break;
			}

			xil_printf("Temperature is: %d.%04d\r", int_p, dec_p);
		}
		else {
			xil_printf("CRC does not match\n\r");
		}
	}
}
