/*
Copyright (C) 2024, Advanced Micro Devices, Inc. All rights reserved.
SPDX-License-Identifier: MIT
*/

#include <stdio.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/ioctl.h>
#define DEVICE_FILE_NAME "/dev/xlnx_w1"

#define XLNX_IOCTL_RESET_BUS 	_IOR('k', 0, int)
#define XLNX_IOCTL_READ_BIT 	_IOR('k', 1, int)
#define XLNX_IOCTL_WRITE_BIT 	_IOW('k', 2, int)
#define XLNX_IOCTL_READ_BYTE 	_IOR('k', 3, int)
#define XLNX_IOCTL_WRITE_BYTE	_IOW('k', 4, int)

int main(int argc, char **argv)
{
    uint8_t crc_table[256] = {0, 94, 188, 226, 97, 63, 221, 131, 194, 156, 126, 32, 163, 253, 31, 65,
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
    uint8_t crc, byte0, byte1, byte2, byte3, byte4, byte5, byte6, byte7, byte8;
    uint16_t bytes;
    int dec_p, int_p;

    int fd, resp_io, tx, rx;
    
    fd = open(DEVICE_FILE_NAME, O_RDWR);
    
    if (fd < 0)
    {
    	printf("Cannot open device\n");
        goto err;
    }
    else
    {
    	printf("Device opened\n");
    }

    while (1)
    {
        // Initialization
        if (ioctl(fd, XLNX_IOCTL_RESET_BUS, &rx) < 0){
            printf("Error 1\n");
            goto err;
        }
        if (rx == 1){
            printf("Error 2\n");
            goto err;
        }
        tx = 0xCC;
        if(ioctl(fd, XLNX_IOCTL_WRITE_BYTE, &tx) < 0){
            printf("Error 3\n");
            goto err;
        }
        tx = 0x44;
        if (ioctl(fd, XLNX_IOCTL_WRITE_BYTE, &tx) < 0){
            printf("Error 4\n");
            goto err;
        }
        rx = 0;
        while(rx == 0){
            if (ioctl(fd, XLNX_IOCTL_READ_BIT, &rx) < 0){
            printf("Error 5\n");
                goto err;
            }
        }
        if (ioctl(fd, XLNX_IOCTL_RESET_BUS, &rx) < 0){
            printf("Error 6\n");
            goto err;
        }
        if (rx == 1){
            printf("Error 7\n");
            goto err;
        }
        tx = 0xCC;
        if (ioctl(fd, XLNX_IOCTL_WRITE_BYTE, &tx) < 0){
            printf("Error 8\n");
            goto err;
        }
        tx = 0xBE;
        if (ioctl(fd, XLNX_IOCTL_WRITE_BYTE, &tx) < 0){
            printf("Error 9\n");
            goto err;
        }

        if (ioctl(fd, XLNX_IOCTL_READ_BYTE, &byte0) < 0){
            printf("Error 10\n");
            goto err;
        }
        if (ioctl(fd, XLNX_IOCTL_READ_BYTE, &byte1) < 0){
            printf("Error 11\n");
            goto err;
        }
        if (ioctl(fd, XLNX_IOCTL_READ_BYTE, &byte2) < 0){
            printf("Error 12\n");
            goto err;
        }
        if (ioctl(fd, XLNX_IOCTL_READ_BYTE, &byte3) < 0){
            printf("Error 13\n");
            goto err;
        }
        if (ioctl(fd, XLNX_IOCTL_READ_BYTE, &byte4) < 0){
            printf("Error 14\n");
            goto err;
        }
        if (ioctl(fd, XLNX_IOCTL_READ_BYTE, &byte5) < 0){
            printf("Error 15\n");
            goto err;
        }
        if (ioctl(fd, XLNX_IOCTL_READ_BYTE, &byte6) < 0){
            printf("Error 16\n");
            goto err;
        }
        if (ioctl(fd, XLNX_IOCTL_READ_BYTE, &byte7) < 0){
            printf("Error 17\n");
            goto err;
        }
        if (ioctl(fd, XLNX_IOCTL_READ_BYTE, &byte8) < 0){
            printf("Error 18\n");
            goto err;
        }

        crc = crc_table[byte0];
		crc = crc_table[byte1 ^ crc];
		crc = crc_table[byte2 ^ crc];
		crc = crc_table[byte3 ^ crc];
		crc = crc_table[byte4 ^ crc];
		crc = crc_table[byte5 ^ crc];
		crc = crc_table[byte6 ^ crc];
		crc = crc_table[byte7 ^ crc];
        if (crc == byte8){
			// Concatenate both bytes and apply 2 complement
			if ((byte1 & 0x80) != 0){
				bytes = (((byte1 << 8) + byte0) ^ 0xFFFF) + 1;
			}
			else {
				bytes = ((byte1 << 8) +  byte0);
			}

			// Get the integer part
			if ((byte1 & 0x80) != 0){
				int_p = -1 * (bytes >> 4);
			}
			else {
				int_p = bytes >> 4;
			}

			// Get the decimal part
			dec_p = (((bytes & 0x8) >> 3) * 5000) + (((bytes & 0x4) >> 2) * 2500)
						+ (((bytes & 0x2) >> 1) * 1250) + ((bytes & 0x1) * 625);

			printf("\rTemperature is: %d.%04d\n", int_p, dec_p);
		}
		else {
			printf("\rCRC does not match\n");
		}
    }

err:
    close(fd);
    
    return 0;
}
