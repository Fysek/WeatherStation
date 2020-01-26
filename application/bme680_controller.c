#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "bme680.h"
// bme680 3 1000 logs.txt
int i2cHandler;
// open I2C Bus
void i2cOpen()
{
	i2cHandler = open("/dev/i2c-1", O_RDWR);
	if (i2cHandler < 0) {
		perror("I2C is already opened!");
		exit(1);
	}
}

// close I2C Bus
void i2cClose()
{
	close(i2cHandler);
}

// set the I2C slave address for all subsequent I2C device transfers
void i2cSetAddress(int address)
{
	if (ioctl(i2cHandler, I2C_SLAVE, address) < 0) {
		perror("I2C address does not exist!");
		exit(1);
	}
}

void user_delay_ms(uint32_t period)
{
    sleep(period/1000);
}

int8_t user_i2c_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len)
{
    int8_t rslt = 0; /* Return 0 for Success, non-zero for failure */

    uint8_t reg[1];
	reg[0]=reg_addr;

 	if (write(i2cHandler, reg, 1) != 1) {
		perror("user_i2c_read_reg");
		rslt = 1;
	}
	if (read(i2cHandler, reg_data, len) != len) {
		perror("user_i2c_read_data");
		rslt = 1;
	}

    return rslt;
}

int8_t user_i2c_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len)
{
    int8_t rslt = 0; /* Return 0 for Success, non-zero for failure */

	uint8_t reg[16];
    reg[0]=reg_addr;
	
    for (int i=1; i<len+1; i++)
       reg[i] = reg_data[i-1];

    if (write(i2cHandler, reg, len+1) != len+1) {
		perror("user_i2c_write");
		rslt = 1;
        exit(1);
	}

    return rslt;
}

void write2file(char *outputFile, struct tm tm, struct bme680_field_data data)
{
	// Write measurement to output file if specified.
	if(outputFile != NULL)
	{
		FILE *f = fopen(outputFile, "a");
		if (f == NULL)
		{
			printf("Error opening file!\n");
			//exit(1);
		}
		else
		{
			fprintf(f,"%d-%02d-%02d %02d:%02d:%02d ", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
			fprintf(f,"T: %.2f degC, P: %.2f hPa, H: %.2f %%rH", data.temperature / 100.0f,
					data.pressure / 100.0f, data.humidity / 1000.0f );
			fprintf(f,", G: %d Ohms", data.gas_resistance);
			fprintf(f,"\r\n");
			fclose(f);
		}
	
	}
}


