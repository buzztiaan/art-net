/*
This example will receive multiple universes via Artnet and control a strip of ws2811 leds via 
Adafruit's NeoPixel library: https://github.com/adafruit/Adafruit_NeoPixel
This example may be copied under the terms of the MIT license, see the LICENSE file for details
*/

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <ArtnetWifi.h>
#include "Freepin_WS2801.h"

//Wifi settings
const char* ssid = "setup";
const char* password = "yourstuff";

// Neopixel settings
const int numLeds = 50; // change for your setup
const int numberOfChannels = numLeds * 3; // Total number of channels you want to receive (1 led = 3 channels)

const byte dataPin = 12;  // yellow
const byte clockPin = 14; // green
Freepin_WS2801 strip = Freepin_WS2801(numLeds, dataPin, clockPin);

// Artnet settings
ArtnetWifi artnet;
const int startUniverse = 0; // CHANGE FOR YOUR SETUP most software this is 1, some software send out artnet first universe as 0.

// Check if we got all universes
const int maxUniverses = numberOfChannels / 512 + ((numberOfChannels % 512) ? 1 : 0);
bool universesReceived[maxUniverses];
bool sendFrame = 1;
int previousDataLength = 0;

// connect to wifi – returns true if successful or false if not
boolean ConnectWifi(void)
{
  boolean state = true;
  int i = 0;

  WiFi.begin(ssid, password);
  WiFi.mode(WIFI_STA);
  WiFi.softAPdisconnect(true);

  Serial.println("");
  Serial.println("Connecting to WiFi");
  
  // Wait for connection
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (i > 20){
      state = false;
      break;
    }
    i++;
  }
  if (state){
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("");
    Serial.println("Connection failed.");
    
  makeitacolor(Color(255,0,0));
  strip.show();
  
  }
  
  return state;
}


// Create a 24 bit color value from R,G,B
uint32_t Color(byte r, byte g, byte b)
{
  uint32_t c;
  c = r;
  c <<= 8;
  c |= g;
  c <<= 8;
  c |= b;
  return c;
}

void makeitacolor(uint32_t color) {
  for (int i=0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, color);
  }
  strip.show();
}

void initTest()
{
  makeitacolor(Color(255,0,0));
  strip.show();
  delay(500);
  makeitacolor(Color(0,255,0));
  strip.show();
  delay(500);
  makeitacolor(Color(0,0,255));
  strip.show();
  delay(500);
  makeitacolor(Color(0,0,0));
  strip.show();
}

void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data)
{
  sendFrame = 1;
  // set brightness of the whole strip 
  //if (universe == 15)
  //{
//    leds.setBrightness(data[0]);
    //leds.show();
  //}

  // Store which universe has got in
  if ((universe - startUniverse) < maxUniverses)
    universesReceived[universe - startUniverse] = 1;

  for (int i = 0 ; i < maxUniverses ; i++)
  {
    if (universesReceived[i] == 0)
    {
      //Serial.println("Broke");
      sendFrame = 0;
      break;
    }
  }

  // read universe and put into the right part of the display buffer
  for (int i = 0; i < length / 3; i++)
  {
    int led = i + (universe - startUniverse) * (previousDataLength / 3);
    if (led < numLeds)
      strip.setPixelColor(led, Color(data[i * 3 + 1], data[i * 3], data[i * 3 + 2]));
  }
  previousDataLength = length;     
  
  if (sendFrame)
  {
    strip.show();
    // Reset universeReceived to 0
    memset(universesReceived, 0, maxUniverses);
  }
}

void setup()
{
  Serial.begin(115200);
  artnet.begin();
  strip.begin();
  makeitacolor(Color(255,255,0));
  strip.show();
  initTest();
  ConnectWifi();
  
  // this will be called for each packet received
  artnet.setArtDmxCallback(onDmxFrame);
}

void loop()
{
  // we call the read function inside the loop
  artnet.read();
}
