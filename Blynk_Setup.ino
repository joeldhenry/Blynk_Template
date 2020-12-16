
#include <FS.h> //this needs to be first, or it all crashes and burns...

#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager
//needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h> //https://github.com/bblanchon/ArduinoJson
#include <BlynkSimpleEsp8266.h>

//flag for saving data
bool shouldSaveConfig = false;
char blynk_token[34];
WiFiManager wifiManager;

bool setupBlynk()
{
    getBlynkToken();
    setupBlynkWifiManager();
    updateBlynkToken();
    Blynk.config(blynk_token);
    return Blynk.connect();
}

void getBlynkToken()
{
    //read configuration from FS json
    Serial.println("mounting FS...");

    if (SPIFFS.begin())
    {
        Serial.println("mounted file system");
        if (SPIFFS.exists("/config.json"))
        {
            //file exists, reading and loading
            Serial.println("reading config file");
            File configFile = SPIFFS.open("/config.json", "r");
            if (configFile)
            {
                Serial.println("opened config file");
                size_t size = configFile.size();
                // Allocate a buffer to store contents of the file.
                std::unique_ptr<char[]> buf(new char[size]);

                configFile.readBytes(buf.get(), size);
                DynamicJsonDocument doc(1024);
                auto error = deserializeJson(doc, buf.get());
                serializeJson(doc, Serial);
                if (error)
                {
                    Serial.println("failed to load json config");
                    return;
                }
                Serial.println("\nparsed json");
                strcpy(blynk_token, doc["blynk_token"]);
            }
        }
    }
    else
    {
        Serial.println("failed to mount FS");
    }
    //end read
}

void setupBlynkWifiManager()
{
    WiFiManagerParameter custom_blynk_token("blynk", "blynk token", blynk_token, 33);

    WiFiManager wifiManager;
    wifiManager.setSaveConfigCallback(saveConfigCallback);
    wifiManager.setAPStaticIPConfig(IPAddress(10, 0, 0, 1), IPAddress(10, 0, 0, 1), IPAddress(255, 255, 255, 0));
    wifiManager.addParameter(&custom_blynk_token);
    wifiManager.setTimeout(120);

    if (!wifiManager.autoConnect("Blynk"))
    {
        Serial.println("failed to connect and hit timeout");
        delay(3000);
        //reset and try again, or maybe put it to deep sleep
        ESP.reset();
        delay(5000);
    }

    //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");

    //read updated parameters
    strcpy(blynk_token, custom_blynk_token.getValue());
}

void updateBlynkToken()
{
    //save the custom parameters to FS
    if (shouldSaveConfig)
    {
        Serial.println("saving config");
        DynamicJsonDocument doc(1024);
        doc["blynk_token"] = blynk_token;

        File configFile = SPIFFS.open("/config.json", "w");
        if (!configFile)
        {
            Serial.println("failed to open config file for writing");
        }

        serializeJson(doc, Serial);
        serializeJson(doc, configFile);
        configFile.close();
        //end save
    }
}

//callback notifying us of the need to save config
void saveConfigCallback()
{
    Serial.println("Should save config");
    shouldSaveConfig = true;
}

void resetSettings()
{
    wifiManager.resetSettings();
    ESP.reset();
    delay(5000);
}

// when connected, get reset and min/max temps
BLYNK_CONNECTED()
{
    Blynk.syncAll();
}
