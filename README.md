# WeatherStation
<!-- ABOUT THE PROJECT -->
## About the project
The weather station is one of the most popular IoT projects with which electronics fans begin their adventure with Raspberry Pi, sensors, data transfer and their visualization. In this article, we'll look at monitoring temperature, humidity, barometric pressure, and air quality with a universal sensor that connects directly to your Raspberry Pi without additional components. The big advantage of this application is that it does not require a lot of experience in electronics and programming, and it introduces many useful issues. In this project, we will additionally implement a database for storing readings along with the Grafana GUI to view the current readings.

In the app I use the IP address: 192.168.12.16 which is the address of my Raspberry Pi. To work properly on the reader's RPi, it must be changed in the source files and in Grafana configuration.


### Built With

* [MQTT](https://mqtt.org)
* [Grafana](https://grafana.com/)
* [InfluxDB](https://www.influxdata.com/)


<!-- GETTING STARTED -->
## Getting Started
Connecting the BME680 to the Raspberry Pi

The first step to run the application is to connect the sensor to our Raspberry Pi. Depending on the manufacturer of the board, we can communicate via the SPI or I2C protocol. This application uses a board from pimoroni.com that supports only the I2C protocol.

The sensor is connected to the Raspberry Pi using four wires:

| Raspberry Pi  | BME680 |
| ------------- | ------------- |
| 3V3	  | 2-6V  |
| SDA1 I2C  | SCA  |
| SCL1 I2C  | SCL  |
| Ground	  | GND  |

### MQTT Installation
The MQTT protocol provides a lightweight method of messaging using the publish-subscribe model. This makes it suitable for transmitting messages from devices such as low-power sensors or mobile devices such as a smartphone or a microcontroller.

To build the project with the MQTT client, we need to download the library from git and install it. For this we execute six commands in the terminal:

```sh
git clone https://github.com/eclipse/paho.mqtt.c.git
cd paho.mqtt.c
git checkout v1.3.1
cmake -Bbuild -H. -DPAHO_WITH_SSL=ON -DPAHO_ENABLE_TESTING=OFF
sudo cmake --build build/ --target install
sudo ldconfig
```
### Installation and configuration of InfluxDB

### Uploading measurements to the database

### Grafana installation and configuration



