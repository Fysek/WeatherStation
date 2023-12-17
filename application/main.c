#include "bme680_controller.h"
#include "bme680.h"
#include "bme680_defs.h"
#include "mqtt.h"

#define ADDRESS     	"127.0.0.1"
#define CLIENTID    	"Raspberry Pi 4B"
#define BME680_I2C_ADDR 0x76
#define DAY_SECONDS     86400


float calculate_average(float baseline_gas_data[], int size) {
    float sum = 0.0;
    int i;

    // Calculate sum of all elements in the array
    for (i = 0; i < size; ++i) {
        sum += baseline_gas_data[i];
    }

    // Calculate the average
    float average = sum / size;

    return average;
}

int main(int argc, char *argv[]){
	int delay, nMeas;
	char *outputFile = NULL;

	// Input argument parser
	if( argc == 2 ) {
		delay = strtol(argv[1], NULL, 10);
	}
	else if( argc == 3 ) {
		delay = strtol(argv[1], NULL, 10);
		nMeas = strtol(argv[2], NULL, 10);
	}
	else if( argc == 4 ) {
		delay = strtol(argv[1], NULL, 10);
		nMeas = strtol(argv[2], NULL, 10);
		outputFile = argv[3]; 
	}
	else {
		delay = 3;
		nMeas = 3;
	}
	
    MQTTClient client;
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    conn_opts.username = "<<7>>/<<RPi>>";
    MQTTClient_setCallbacks(client, NULL, NULL, on_message, NULL);
	
	int rc;
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        exit(-1);
    }
	
	// open I2C 
	i2cOpen();
	// set address of the BME680
	i2cSetAddress(BME680_I2C_ADDR_PRIMARY);

	struct bme680_dev Sensor;

	Sensor.dev_id = BME680_I2C_ADDR_PRIMARY;
	Sensor.intf = BME680_I2C_INTF;
	Sensor.read = user_i2c_read;
	Sensor.write = user_i2c_write;
	Sensor.delay_ms = user_delay_ms;

	int8_t rslt = BME680_OK;
	rslt = bme680_init(&Sensor);

    uint8_t set_required_settings;
    
    //Select the power mode 
	//Must be set before writing the sensor configuration 
	Sensor.power_mode = BME680_FORCED_MODE; 

	// Set the temperature, pressure and humidity settings
	Sensor.tph_sett.os_hum = BME680_OS_2X;
	Sensor.tph_sett.os_pres = BME680_OS_4X;
	Sensor.tph_sett.os_temp = BME680_OS_8X;
	Sensor.tph_sett.filter = BME680_FILTER_SIZE_3;

	// Set the remaining gas sensor settings and link the heating profile 
	Sensor.gas_sett.run_gas = BME680_ENABLE_GAS_MEAS;
	
	// Create a ramp heat waveform in 3 steps 
	Sensor.gas_sett.heatr_temp = 320; //degree Celsius 
	Sensor.gas_sett.heatr_dur = 150; //ms

	// Set the required sensor settings needed
	set_required_settings = BME680_OST_SEL | BME680_OSP_SEL | BME680_OSH_SEL | BME680_FILTER_SEL 
		| BME680_GAS_SENSOR_SEL;

	// Set the desired sensor configuration 
	rslt = bme680_set_sensor_settings(set_required_settings,&Sensor);

	// Set the power mode 
	rslt = bme680_set_sensor_mode(&Sensor);
	
	// Get the total measurement duration so as to sleep or wait till the  measurement is complete 
	uint16_t meas_period;
	bme680_get_profile_dur(&meas_period, &Sensor);
	
	// Delay till the measurement is ready 
	user_delay_ms(meas_period + delay*1000); 

	printf("\n***Start of measurements***\n");

	time_t t = time(NULL);

    struct bme680_field_data data;

	struct tm tm = *localtime(&t);
	int idx=0;
	char res[20]; 
	
	//IAQ
	int idx_reset = DAY_SECONDS / delay;
	float baseline_gas_data[50];
	int data_count = 0;
	float hum, gas, air_quality_score;
	float hum_offset, gas_offset;
	float hum_score, gas_score;
	float gas_baseline = 50000;
	float hum_baseline = 40.0;
    float hum_weighting = 0.25;
	

	while(1) {

		// Get sensor data
		rslt = bme680_get_sensor_data(&data, &Sensor);
		
		// Avoid using measurements from an unstable heating setup 
		if(data.status & BME680_HEAT_STAB_MSK)
		{
			t = time(NULL);
			tm = *localtime(&t);
			
			
			gcvt(data.temperature / 100.0f, 4, res);
			publish(client, "home/bme680/temperature",  res);
			gcvt(data.pressure / 100.0f, 6, res);
			publish(client, "home/bme680/pressure", res);
			gcvt(data.humidity / 1000.0f, 4, res);
			publish(client, "home/bme680/humidity", res);
			hum = data.humidity / 1000.0f;
			gcvt(data.gas_resistance/ 1000.0f, 5, res);
			publish(client, "home/bme680/gas_resistance", res);
			gas = data.gas_resistance; // must be in clear values, no kohms
	

			if (idx >= 50){
				gas_baseline = calculate_average(baseline_gas_data, 50);
			} else {
				baseline_gas_data[idx] = gas;
			}
	
			hum_offset = hum - hum_baseline;
			gas_offset = gas_baseline - gas;

            if (hum_offset > 0) {
                hum_score = (100 - hum_baseline - hum_offset) /
                            (100 - hum_baseline) * (hum_weighting * 100);
            } else {
                hum_score = (hum_baseline + hum_offset) /
                            hum_baseline * (hum_weighting * 100);
            }

            if (gas_offset > 0) {
                gas_score = (gas / gas_baseline) * (100 - (hum_weighting * 100));
            } else {
                gas_score = 100 - (hum_weighting * 100);
            }

            air_quality_score = hum_score + gas_score;	
			gcvt(air_quality_score, 3, res);
			publish(client, "home/bme680/air_quality", res);

			/*
			printf("%d-%02d-%02d %02d:%02d:%02d ", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
			printf("T: %.2f degC, P: %.2f hPa, H: %.2f %%rH", data.temperature / 100.0f,
					data.pressure / 100.0f, data.humidity / 1000.0f );
			printf(", G: %.2f kOhms", data.gas_resistance/ 1000.0f);
			printf(", IAQ: %.1f ", air_quality_score);
			printf("\r\n");
			*/

			write2file(outputFile, tm, data);
			if (idx == idx_reset) { 
				idx = 0; //reset index to get gas_baseline again after a day
			} else {
				idx++;
			}
		} else {
			//When setup unstable, add measurement
			nMeas++;
		}
		
		// Trigger next meausurement
		rslt = bme680_set_sensor_mode(&Sensor); 

		// Wait for next meausurement
		user_delay_ms(meas_period + delay*1000); 		

	}

	printf("***End of measurements***\n");

    //close I2C Bus
	i2cClose();
	
	MQTTClient_disconnect(client, 1000);
    MQTTClient_destroy(&client);
    return rc;
}
