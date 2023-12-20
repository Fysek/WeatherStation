# WeatherStation
<p align="center">
  <img src="https://user-images.githubusercontent.com/29653491/100155476-a229da00-2ea7-11eb-971e-46b5950a9ea2.PNG">
</p>


<!-- TABLE OF CONTENTS -->
<details open="open">
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#about-the-project">About The Project</a>
    </li>
    <li>
      <a href="#getting-started">Getting Started</a>
      <ul>
        <li><a href="#mqtt">MQTT Installation</a></li>
        <li><a href="#influxdb">Installation and configuration of InfluxDB</a></li>
        <li><a href="#forwarder">Uploading measurements to the database</a></li>
        <li><a href="#grafana">Grafana installation and configuration</a></li>
      </ul>
    </li>
    <li><a href="#usage">Usage</a></li>
    <li><a href="#license">License</a></li>
    <li><a href="#contact">Contact</a></li>
    <li><a href="#acknowledgements">Acknowledgements</a></li>
  </ol>
</details>

<!-- ABOUT THE PROJECT -->
## About the project
The weather station is one of the most popular IoT projects with which electronics fans begin their adventure with Raspberry Pi, sensors, data transfer and their visualization. In this article, we'll look at monitoring temperature, humidity, barometric pressure, and air quality with a universal sensor that connects directly to your Raspberry Pi without additional components. The big advantage of this application is that it does not require a lot of experience in electronics and programming, and it introduces many useful issues. In this project, we will additionally implement a database for storing readings along with the Grafana GUI to view the current readings.

In the app I use the IP address: 192.168.12.16 which is the address of my Raspberry Pi. To work properly on the reader's RPi, it must be changed in the source files and in Grafana configuration.


### Built With

* [MQTT](https://mqtt.org)
* [Grafana](https://grafana.com/)
* [InfluxDB](https://www.influxdata.com/)
* [C]()
* [Python]()


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

<!-- MQTT -->
### MQTT Installation
The MQTT protocol provides a lightweight method of messaging using the publish-subscribe model. This makes it suitable for transmitting messages from devices such as low-power sensors or mobile devices such as a smartphone or a microcontroller.

To build the project with the MQTT client, we need to download the library from git and install it. For this we execute six commands in the terminal:

Install prerequisites:
```sh
sudo apt-get install build-essential gcc make cmake cmake-gui cmake-curses-gui
sudo apt-get install fakeroot devscripts dh-make lsb-release
sudo apt-get install libssl-dev
```

Then get the source:
```sh
git clone https://github.com/eclipse/paho.mqtt.c.git
cd paho.mqtt.c
git checkout v1.3.1
cmake -Bbuild -H. -DPAHO_WITH_SSL=ON -DPAHO_ENABLE_TESTING=OFF
sudo cmake --build build/ --target install
sudo ldconfig
```
After the libraries installation is completed, we can take care of the MQTT broker. The broker acts as a server that receives data on a given topic from various devices. Clients can subscribe to specific topics and post messages on various topics.

Installation of the Eclipse Mosquitto distribution is done by executing the command:
```sh
sudo apt install mosquitto mosquitto-clients
```
Now we have the entire environment ready and we can start the first launch of the application. First, we open a new console and run the MQTT broker with the following instructions:
```sh
mosquitto -v
```
When the broker is listening in the background, we can run the weather station application. On another terminal, in the application directory, we execute the command:
```sh
make
```
Then run the WeatherStation application:
```sh
./WeatherStation 5 1000 logs.txt
```
The arguments in this command mean:
- The first number is the interval between subsequent measurements in seconds,
- The second number indicates the number of measurements,
- The third argument is the name of the log file.

<!--INFLUXDB-->
### Installation and configuration of InfluxDB
In the application, I use InfluxDB to store data as it is optimized for time series data. Of course, it would also be possible to work with other databases like MariaDB or mongoDB, but InfluxDB works directly with Grafana. The first thing to do is install InfluxDB on your Raspberry Pi. Installation is fairly easy, you just need to execute the following two commands in the terminal:
```sh
sudo apt install influxdb
sudo apt install influxdb-client
```
After successful installation, we run InfluxDB and control the current status with the following commands:
```sh
sudo service influxdb start
sudo service influxdb status
```
After installing InfluxDB, we need to make one configuration change to enable the HTTP endpoint. This is necessary because we want to write the data from the MQTT subscriber to the existing database. Go to file
```sh
sudo cd /etc/influxdb/influxdb.conf
```
In the configuration file deselect the first setting by removing the "#" sign in the three lines shown below. 
```sh
enabled = true
bind-address = "8086"
auth-enabled = false
```
The InfluxDB must be restarted for each configuration change to take effect. We will restart InfluxDB with the following command.
```sh
sudo service influxdb restart
```
The configuration is complete and we can proceed to creating a database in which all measurements are stored and a user who saves the MQTT data to the database. First, we'll run InfluxDB with the following command in the Raspberry Pi terminal:
```sh
influx
```
Now we will create a new database with any name. I chose weather_stations as the name. We will build a database with the command:
```sh
CREATE DATABASE weather_stations
```
The database has been created and now we create a new user and give him access to the previously created database. I choose mqtt as my username and password.
```sh 
CREATE USER mqtt WITH PASSWORD 'mqtt'
GRANT ALL ON weather_stations TO mqtt
```

<!-- FORWARDER -->
### Uploading measurements to the database
Now that influxdb is up and running, we need to write a little script that passes the data from the broker to InfluxDB. Python was used for this task, which, using the paho mqtt library, saves the measurements coming to the broker in the database. The file was named forwarder.py and can be found in the WeatherStation root directory.

The program starts by connecting to the broker with a given IP address and the weather_stations database in InfluxDB. The client created in this way subscribes to the given topics and listens for messages in these topics. If it receives measurements by a broker, it calls a function (callback), which formats the data and sends it to the database. The program runs in an endless loop and should be turned off when finished.

We run the program with the command:
```sh
pip3 install paho-mqtt
pip3 install influxdb

python3 forwarder.py
```

<!-- GRAFANA -->
### Grafana installation and configuration
To install Grafana, we must first check the latest version in the browser: https://github.com/grafana/grafana/releases. At the moment the latest version is 6.6.0. We put the version number in the command to download and install:
```sh 
wget https://dl.grafana.com/oss/release/grafana_6.6.0_armhf.deb
sudo dpkg -i grafana_6.6.0_armhf.deb
sudo apt-get update
sudo apt-get install grafana
```
After installation, we can start the grafana server with the command in the terminal:
```sh
sudo service grafana-server start
```
Access to Grafana is obtained on port 3000. Just enter the combination of the Raspberry Pi's IP address and port in the browser. In this case it is 192.168.12.16:3000.
At the first launch, a login window pops up: The default username and password is admin. During the first login, we can change the password.

Now it's time to choose your data source. We choose InfluxDB. The most important fields are the name of the source, URL, the name of the database we are using, and the user name and password. We save the settings and proceed to creating the panel.

In the repository there is the GrafanaWeatherStationJson file, which contains information about the panel.

### TODO

- [ ] Add requirements.txt for python
- [ ] Create a linux service to run the app in the background
- [ ] Add .gitignore
- [ ] Clean the code, make it more readable
- [ ] Add debug mode


<!-- LICENSE -->
## License

Distributed under the MIT License. See `LICENSE` for more information.

<!-- CONTACT -->
## Contact

Mateusz Dyrdol - [@Mateusz Dyrdol](https://www.linkedin.com/in/mateusz-dyrdol/) - mateusz.dyrdol@gmail.com

Project Link: [https://github.com/Fysek/ArchSound/](https://github.com/Fysek/ArchSound/)

<!-- ACKNOWLEDGEMENTS -->
## Acknowledgements
* [Adafruit MCP3008 github](https://github.com/adafruit/Adafruit_CircuitPython_MCP3xxx)
* [Analog Inputs using MCP3008](https://learn.adafruit.com/reading-a-analog-in-and-controlling-audio-volume-with-the-raspberry-pi)
* [Multiple MCP3008](https://www.raspberrypi.org/forums/viewtopic.php?t=210330)
* [Pymixer - playing multiple sounds at once](https://classes.engineering.wustl.edu/ese205/core/index.php?title=Playing_multiple_sounds_at_once)
* [Raspberry Pi Multiple SPIs](https://blog.stabel.family/raspberry-pi-4-multiple-spis-and-the-device-tree/)
* [Raspberry Pi Pinout](https://pinout.xyz/pinout/spi)
* [Ultraflat Potentiometer Membrane](https://www.vishay.com/sensors/list/product-32537/)
