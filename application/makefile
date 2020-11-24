# makefile for BME680

all: main.o bme680_controller.o bme680.o mqtt.o
	gcc -o WeatherStation main.o bme680_controller.o bme680.o mqtt.o -lpaho-mqtt3c

main.o: main.c  
	gcc -c main.c

bme680_controller.o: bme680_controller.c  
	gcc -c bme680_controller.c

bme680.o: bme680.c bme680.h bme680_defs.h
	gcc -c bme680.c
	
mqtt.o: mqtt.c
	gcc -c mqtt.c 

clean:
	rm bme680_controller.o bme680






