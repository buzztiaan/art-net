/*
 * 
 * simple ESP8266 based Art-Net receiver , singlechannel version for on Sonoff
 * 
 * using; ArtnetWifi as from https://github.com/rstephan/ArtnetWifi
 *        WiFiManager as from https://github.com/tzapu/WiFiManager
 *        ArduinoJson as from https://github.com/bblanchon/ArduinoJson
 *        
 *        
 * 2016 buZz | NURDspace.nl
 * 
 */

/*
 * 
 * TODO:
 * 
 * - implement button
 * - maybe make configuration zappable?
 * 
 */

#include <FS.h>

#include <ESP8266WiFi.h>

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>

#include <ArduinoJson.h>

#include <WiFiUdp.h>
#include <ArtnetWifi.h>

// preconfig defaults;
char configChannel[4] = "100"; 
char configUniverse[2] = "0";

//flag for saving data
bool shouldSaveConfig = false;

const int relayPin = 12;
const int ledPin = 13;
const int buttonPin = 0;

// const int maxUniverses = numberOfChannels / 512 + ((numberOfChannels % 512) ? 1 : 0);
// bool universesReceived[maxUniverses];
bool sendFrame = 1;
//int previousDataLength = 0;


//
ArtnetWifi artnet;

//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void setup() {
  Serial.begin(115200);

  artnet.begin();

  pinMode(relayPin, OUTPUT);
  pinMode(ledPin, OUTPUT);

  digitalWrite(relayPin, LOW);
  digitalWrite(ledPin, HIGH);

  // SPIFFS stuff for reading json configuration
  Serial.println("mounting FS...");
  if (SPIFFS.begin()) {
    if (SPIFFS.exists("/config.json")) {
      Serial.println("reading config file");
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        if (json.success()) {
          // get custom configs!
          strcpy(configChannel, json["channel"]);
          strcpy(configUniverse, json["universe"]);
          
        } else {
          Serial.println("failed to load json config");
        }
      }
    } else {
      Serial.println("json config file not found");
    }
  } else {
    Serial.println("failed to mount FS");
  }

  WiFiManagerParameter custom_channel("channel", "channel", configChannel, 4);
  WiFiManagerParameter custom_universe("universe", "universe", configUniverse, 2);

  WiFiManager wifiManager;

  wifiManager.setSaveConfigCallback(saveConfigCallback);

  wifiManager.addParameter(&custom_channel);
  wifiManager.addParameter(&custom_universe);

  if (!wifiManager.autoConnect("WifiManager-ESP", "password")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  Serial.println("connected...yeey :)");

  strcpy(configChannel, custom_channel.getValue());
  strcpy(configUniverse, custom_universe.getValue());

  WiFi.mode(WIFI_STA);
  WiFi.softAPdisconnect(true);

  // save the json again
  if (shouldSaveConfig) {
    Serial.println("saving config");
    DynamicJsonBuffer jsonBuffer;
    JsonObject& json = jsonBuffer.createObject();
    json["channel"] = configChannel;
    json["universe"] = configUniverse;

    File configFile = SPIFFS.open("/config.json", "w");
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    json.printTo(Serial);
    json.printTo(configFile);
    configFile.close();
    //end save
  }

  artnet.setArtDmxCallback(onDmxFrame);
  
}


void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data)
{
  sendFrame = 1;

  bool relay = false;

  if (universe == atoi(configUniverse)) {
    relay = data[atoi(configChannel)] > 127? HIGH : LOW;
  } else {
    sendFrame = false;
  }
  
  if (sendFrame)
  {
    digitalWrite(relayPin, relay);
    digitalWrite(ledPin, !relay);
  }
}

void loop() {
  artnet.read();

}
