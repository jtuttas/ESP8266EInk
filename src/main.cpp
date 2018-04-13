
// Supporting Arduino Forum Topics:
// Waveshare e-paper displays with SPI: http://forum.arduino.cc/index.php?topic=487007.0
// Good Dispay ePaper for Arduino : https://forum.arduino.cc/index.php?topic=436411.0

// mapping suggestion from Waveshare SPI e-Paper to Wemos D1 mini
// BUSY -> D2, RST -> D4, DC -> D3, CS -> D8, CLK -> D5, DIN -> D7, GND -> GND, 3.3V -> 3.3V

// mapping suggestion from Waveshare SPI e-Paper to generic ESP8266
// BUSY -> GPIO4, RST -> GPIO2, DC -> GPIO0, CS -> GPIO15, CLK -> GPIO14, DIN -> GPIO13, GND -> GND, 3.3V -> 3.3V

// mapping suggestion for ESP32, e.g. LOLIN32, see .../variants/.../pins_arduino.h for your board
// NOTE: there are variants with different pins for SPI ! CHECK SPI PINS OF YOUR BOARD
// BUSY -> 4, RST -> 16, DC -> 17, CS -> SS(5), CLK -> SCK(18), DIN -> MOSI(23), GND -> GND, 3.3V -> 3.3V

// new mapping suggestion for STM32F1, e.g. STM32F103C8T6 "BluePill"
// BUSY -> A1, RST -> A2, DC -> A3, CS-> A4, CLK -> A5, DIN -> A7

// mapping suggestion for AVR, UNO, NANO etc.
// BUSY -> 7, RST -> 9, DC -> 8, CS-> 10, CLK -> 13, DIN -> 11
#include <Arduino.h>
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include <GxEPD.h>
#include <GxGDEP015OC1/GxGDEP015OC1.cpp>
#include <GxIO/GxIO_SPI/GxIO_SPI.cpp>
#include <GxIO/GxIO.cpp>
#include <Fonts/FreeSansBold24pt7b.h>
#include <BitmapGraphics.h>
#include <ArduinoJson.h>

float tempC = 0;

GxIO_Class io(SPI, SS, 0, 2); // arbitrary selection of D3(=0), D4(=2), selected for default of GxEPD_Class
// GxGDEP015OC1(GxIO& io, uint8_t rst = 2, uint8_t busy = 4);
GxEPD_Class display(io);
const char *ssid = "FRITZ!Box Fon WLAN 7390";
const char *password = "geheim";
const char *mqtt_server = "service.joerg-tuttas.de";
WiFiClient espClient;
PubSubClient client(espClient);

void showPartialUpdate(float temperature)
{
    String temperatureString = String(temperature, 1);
    const char *name = "FreeSansBold24pt7b";
    const GFXfont *f = &FreeSansBold24pt7b;

    uint16_t box_x = 60;
    uint16_t box_y = 60;
    uint16_t box_w = 90;
    uint16_t box_h = 100;
    uint16_t cursor_y = box_y + 16;

    display.setRotation(45);
    display.setFont(f);
    display.setTextColor(GxEPD_BLACK);

    display.fillRect(box_x, box_y, box_w, box_h, GxEPD_WHITE);
    display.setCursor(box_x, cursor_y + 38);
    display.print(temperatureString);
    display.updateWindow(box_x, box_y, box_w, box_h, true);
}

void callback(char *topic, byte *payload, unsigned int length)
{
    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++)
    {
        Serial.print((char)payload[i]);
    }
    Serial.println();
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(payload);
    float temp = root["temp"];
    Serial.print("Temp gelesen:");
    Serial.println(temp);
    showPartialUpdate(temp);
   
}

void reconnect()
{
    // Loop until we're reconnected
    while (!client.connected())
    {
        Serial.print("Attempting MQTT connection...");
        // Create a random client ID
        String clientId = "ESP8266Client-";
        clientId += String(random(0xffff), HEX);
        // Attempt to connect
        if (client.connect(clientId.c_str()))
        {
            Serial.println("connected");
            // ... and resubscribe
            client.subscribe("esp/temp");
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
}



void setup()
{

    Serial.begin(115200);
    delay(500);
    Serial.println("setup");
    display.init();
    Serial.println("display Init");
    display.drawExampleBitmap(gImage_splash, 0, 0, 200, 200, GxEPD_BLACK);
    display.update();
    Serial.write("\r\nConnect to WLAN");
    WiFi.begin(ssid, password);
    Serial.println();
    Serial.println("Waiting for connection and IP Address from DHCP");
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(2000);
        Serial.print(".");
    }
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);

    display.drawExampleBitmap(gImage_gui, 0, 0, 200, 200, GxEPD_BLACK);
    display.update();

    display.drawExampleBitmap(gImage_gui, sizeof(gImage_gui), GxEPD::bm_default | GxEPD::bm_partial_update);
}

void loop()
{
    if (!client.connected())
    {
        reconnect();
    }
    client.loop();
}
