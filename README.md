# AnnaRakshak-IoT

Code for Anna Rakshak/ Grain Saviour (IoT Enabled Food Grain Health Information System)<br>
This code is only compatible with a **TFT_SPI** diplay and an **ESP32/ESP8266** & Hardware used for the IoT Device. 
<br>
## Setup/Working of the device

- It currently uses ThingSpeak platform by mathswork for uploading & displaying all the sensor data.

- It **cannot** create its own hotspot for now, but works by connecting it to a WiFi network. To change the connection parameters, change : 
```
        const char* ssid = "SSID";   // Write your SSID
        const char* password = "YOUR PASSWORD";   // Write your WIFI password
```

- If using Arudino IDE, These **Additional Board Manager URL's** are required : 
> http://arduino.esp8266.com/stable/package_esp8266com_index.json
http://dl.sipeed.com/MAIX/Maixduino/package_Maixduino_k210_index.json
http://dan.drown.org/stm32duino/package_STM32duino_index.json
https://dl.espressif.com/dl/package_esp32_index.json
http://arduino.esp8266.com/stable/package_esp8266com_index.json
https://dl.espressif.com/dl/package_esp32_index.json
http://arduino.esp8266.com/stable/package_esp8266com_index.json

- When using Arduino IDE, Change `User_setup.h` in the libraries folder for configuring TFT Display. You can find it in the resources.

- If using ThingSpeak, Change the following lines with your values: 
```
unsigned long Channel_ID = ;  //replace with your Channel ID
const char * API_key = "Your API Key"; //replace with your API Key
```
- Current problem: It takes 15-20s to upload the data to ThingSpeak. This has to be fixed.

## Credits :

**Aditya Gaur** _@ VBPS_ <br>
**Kanishk Anand** _@ VBPS_ <br>
**Mr. Arun** _@ VBPS/ATL_ <br>
