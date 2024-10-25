# Nordig Thingy 53 Environment Sensor
This project contains the code to let a Nordic Thingy 53 measure temperature, humidity, air pressure and acceleration in three axes. The gathered data is sent via Bluetooth Low Energy. It has been build with ```nRF Connect SDK 2.7``` version. The data is read from the sensors ```BME688``` and ```ADXL362```. The sources consist of two modules. ```main``` contains the hardware initialization and reading the sensors. ```sensor_hub_service``` contains all necessary code to send data via Bluetooth.

## Current Use of Bluetooth Low Enery UUIDS
### Environment Service
UUID: 181A

#### Characteristics:
| UUID     | Characteristic | Unit | Data type |
| -------- | -------------- | ---- | --------- |
| 2a6e     | Temperature    | °C   | double    |
| 2a6d     | Pressure       | Pa   | double    |
| 2a6f     | Humidity       | %    | double    |

### Position Service
UUID: 1821

#### Characteristics:
| UUID         | Characteristic      | Unit------- | Data type |
| -------------| ------------------- | ----------- | --------- |
| 506a55c4-... | Acceleration x axis | factor of g | double    |
| 51838aff-... | Acceleration y axis | factor of g | double    |
| 753e3050-... | Acceleration z axis | factor of g | double    |