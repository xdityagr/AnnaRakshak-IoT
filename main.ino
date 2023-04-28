//-----------------------------------------------------
// Code for Anna Rakshak
// Written by Aditya Gaur @ VBPS 2023
// check README for configuration
//-----------------------------------------------------


// Including neccessary libraries
#include <ETH.h>
#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiClient.h>
#include <WiFiGeneric.h>
#include <WiFiMulti.h>
#include <WiFiScan.h>
#include <WiFiServer.h>
#include <WiFiSTA.h>
#include <WiFiType.h>
#include <WiFiUdp.h>
#include <SPI.h>
#include <TFT_eSPI.h> 
#include "DHT.h"
#include "ThingSpeak.h"

// Init Libs
TFT_eSPI tft = TFT_eSPI();     
WiFiClient  client;

// Defining colors
#define TFT_GREY 0x5AEB
#define TFT_DARK_GREEN 0x166800
#define TFT_LIME_GREEN 0x42E317
#define TFT_CYAN 0x07FF      
#define TFT_NAVY 0x000F      
#define TFT_MAGENTA 0xF81F      

#define LOOP_PERIOD 1500 // Display updates every 35 ms

#define DHT11PIN 16 // Defining DHT Sensor Pin
DHT dht(DHT11PIN, DHT11); // Init

// Defining Conn Params
const char* ssid = "Jai shri shyam_5G";   // Write your SSID
const char* password = "guru1102";   // Write your WIFI password
unsigned long Channel_ID = 2008436;  //replace with your Channel ID
const char * API_key = "YP0W4WOOR4VYBYLR";

// Defining Variables
float ltx = 0;    // Saved x coord of bottom of needle

uint16_t osx = 120, osy = 120; // Saved x & y coords
uint32_t updateTime = 0;       // time for next update

int old_analog =  -999; // Value last displayed
int old_digital = -999; // Value last displayed

int value[6] = {0, 0, 0, 0, 0, 0};
int old_value[6] = { -1, -1, -1, -1, -1, -1};
int d = 0;

// Sensor Value Variables
float CO = 0.0;
int CO2 = 0;
int CO2_mapped = 0;
float N2 = 0.0;
float temp = 0.0;
float humid = 0.0;

// Function to load up all the UI.
void displayMainScreen() {
  // Top Bar
  tft.fillRect(0, 0, 480, 35, TFT_RED); 
  tft.setTextColor(TFT_WHITE); 
  tft.drawCentreString("ANNA RAKSHAK : IoT Enabled Food Grain Health Informer.", 240, 7, 2);

  // Quality Area --> Circle, Condition, Safety Standards
  tft.fillCircle(40, 80, 28, TFT_GREEN);
  tft.setTextColor(TFT_DARK_GREEN);  
  tft.drawString("GOOD", 24, 75,  2);
  tft.setTextColor(TFT_GREY);  
  tft.drawString("Your crops are", 85, 67,  2);
  tft.drawString("in the best condition!", 85, 79,  2);

  tft.drawFastVLine(238,54, 60, TFT_BLACK);

  tft.setTextColor(TFT_GREY);  
  tft.drawString("Safety Standards :", 262, 58,  2);
  tft.fillCircle(270, 86, 5, TFT_YELLOW);
  tft.fillCircle(270, 106, 5, TFT_ORANGE);
  tft.fillCircle(380, 86, 5, TFT_MAGENTA);
  tft.fillCircle(380, 106, 5, TFT_GREEN);

  tft.setTextColor(TFT_GREY);  
  tft.drawString("CO2 : 200PPM", 280, 83,  1);
  tft.drawString("CO : 60PPM", 280, 103,  1);
  tft.drawString("N2 : 400PPM", 390, 83,  1);
  tft.drawString("HI : 60%", 390, 103,  1);
   
  // Separator + Condition Levels 
  tft.drawLine(0,130,480,130,TFT_BLACK);
  tft.setTextColor(TFT_BLACK, TFT_WHITE);  
  tft.drawString("Condition Levels", 20, 124,  2);

  }

// Function for plotting graph
void plotLinear(char *label, int x, int y)
{
  int w = 36; // graph width

  // graph_rect
  tft.drawRect(x, y, w, 155, TFT_GREY); 
  tft.fillRect(x + 2, y + 19, w - 3, 155 - 38, TFT_WHITE);

  // graph_label
  tft.setTextColor(TFT_BLACK, TFT_WHITE);
  tft.drawCentreString(label, x + w / 2, y + 2, 2); 

  // plotting small & large lines intervals in the graph.
  for (int i = 0; i < 110; i += 10)
  {
    tft.drawFastHLine(x + 20, y + 27 + i, 6, TFT_BLACK);
  }

  for (int i = 0; i < 110; i += 50)
  {
    tft.drawFastHLine(x + 20, y + 27 + i, 9, TFT_BLACK);
  }

  // plotting pointer w/ triangles * plotting value_rect + value text
  tft.fillTriangle(x + 3, y + 127, x + 3 + 16, y + 127, x + 3, y + 127 - 5, TFT_RED);
  tft.fillTriangle(x + 3, y + 127, x + 3 + 16, y + 127, x + 3, y + 127 + 5, TFT_RED);
  tft.fillRect(x,y+138,w,20,TFT_RED);
  tft.setTextColor(TFT_WHITE, TFT_RED);
  tft.drawCentreString("---", x + w / 2, y + 155 - 18, 2);
}


// plotting pointer & changing/updating values
void plotPointer(void)
{
  int dy = 187; // pointer y
  byte pw = 15; // pointer width

  tft.setTextColor(TFT_WHITE, TFT_RED); 

  // Move the 5 pointers one pixel towards new value
  for (int i = 0; i < 5; i++)
  {
    char buf[8]; dtostrf(value[i], 4, 0, buf); // loading current value
    tft.drawRightString(buf, 55 + (i * 80) + 36 - 5, 187 - 27 + 155 - 18, 2); // drawing value

    int dx = 3 + 55 + (80 * i);
    
    // Limit value to emulate needle end stops & mapping values greater than 100 (CO2).
    if (value[i] < 0) value[i] = 0; 
    if (value[i] > 100) {
      if  (i ==0) {
        value[i] = map(value[i],0,1000,1,100);
      }
      else {
        value[i] = 100;
        }
      
      };

    // Updating values & pointer pos
    while (!(value[i] == old_value[i])) {
      dy = 187 + 100 - old_value[i];
      if (old_value[i] > value[i])
      {
        tft.drawLine(dx, dy - 5, dx + pw, dy, TFT_WHITE);
        old_value[i]--;
        tft.drawLine(dx, dy + 6, dx + pw, dy + 1, TFT_RED);
      }
      else
      {
        tft.drawLine(dx, dy + 5, dx + pw, dy, TFT_WHITE);
        old_value[i]++;
        tft.drawLine(dx, dy - 6, dx + pw, dy - 1, TFT_RED);
      }
    }
  }
}



// setup method
void setup(void) {
  analogReadResolution(12);
  Serial.begin(57600); // For debug
  
  dht.begin(); // DHT Sensor start

  tft.init(); // init TFT display
  tft.setRotation(1);
  tft.fillScreen(TFT_WHITE);

  Serial.print("Anna Rakshak COM [BAUD 57600] [Made with <3 by Aditya]\n ");
  displayMainScreen(); // callin fnc to load the main screen

  // init Wifi Conn
  WiFi.mode(WIFI_STA);   
  ThingSpeak.begin(client);

  // Drawing Linear Graphs

  byte d = 80; // gap between graphs

  // Plotting 5 graphs with x_pos varrying with d&start_pos
  int start_pos = 55;
  plotLinear("CO2", start_pos, 160);
  plotLinear("CO", start_pos +  d, 160);
  plotLinear("N2O", start_pos+ (2 * d), 160);
  plotLinear("Humidity", start_pos+ (3 * d), 160);
  plotLinear("Temperature", start_pos +(4 * d), 160);
    
  updateTime = millis(); // Next update time

  // Wifi Conn
  if(WiFi.status() != WL_CONNECTED){
      Serial.print("Connecting to WiFi...");
      while(WiFi.status() != WL_CONNECTED){
        WiFi.begin(ssid, password); 
        delay(5000);     
      } 
      Serial.println("\nConnected.");
    }

  

  
}

void readSensorVals() {
    int val = analogRead(35);   // value from middle pin for MQ9 ranges from (10xx-...)
    
    CO = val/40;
    CO2 = analogRead(32)/2;
    CO2 = CO2+47;
    N2 = CO2/50; 
    humid = dht.readHumidity();
    temp = dht.readTemperature();

   //  Serial.print("CO2 Value : ");
  //  Serial.print(CO2);
  //  Serial.print("CO Value : ");
  //  Serial.print(CO);
  //  Serial.print("N2 Value : ");
  //  Serial.print(N2);
  //  Serial.print("Temp Value : ");
  //  Serial.print(temp);
  //  Serial.print("\n");
  //  Serial.print("Humidity Value : ");
  //  Serial.print(humid);

  }

void loop () {
  if (updateTime <= millis()) {
    //updating 
    updateTime = millis() + LOOP_PERIOD;

    d += 4; if (d >= 360) d = 0;

    // getting sensor vals
    readSensorVals();

    // plotting pointer
    value[0] = CO2;
    value[1] = CO;
    value[2] = N2;
    value[3] = humid;
    value[4] = temp;
  
    plotPointer();

    // uploading data to thingspeak    
    int Data1 = ThingSpeak.writeField(Channel_ID, 7, humid,API_key);
    int Data2 = ThingSpeak.writeField(Channel_ID, 1, CO2,API_key);
    int Data3 = ThingSpeak.writeField(Channel_ID, 2, CO,API_key);
    int Data4 = ThingSpeak.writeField(Channel_ID, 6, temp,API_key);
    int Data5 = ThingSpeak.writeField(Channel_ID, 3, N2,API_key);
    

}
}
