
#pragma mark - Depend ArduinoJson 6.x and CoinMarketCapApi2 libraries
/*
cd ~/Arduino/libraries
git clone https://github.com/lewisxhe/CoinMarketCapApi2
git clone -b 6.x https://github.com/bblanchon/ArduinoJson.git
*/

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "CoinMarketCapApi.h"

#include "alert.h"
#include "info.h"

#include "btclogo.h"
#include "btc.h"

//---------------------------------------//

// Edit this for your network
#define wifi_ssid "OhXIT"
#define wifi_password "lucask123"


// The new api requires a developer key, so you must apply for a key to use
// https://pro.coinmarketcap.com/account
#define APIKEY "92f1f7db-b8c0-4a71-8791-98e92f48891c"

//---------------------------------------//

WiFiClientSecure client;
CoinMarketCapApi api(client, APIKEY);

// CoinMarketCap's limit is "no more than 10 per minute"
// Make sure to factor in if you are requesting more than one coin.
unsigned long api_mtbs = 30000; //mean time between api requests
unsigned long api_due_time = 0;
// SPI TFT settings edit UserSetup !!!
#include <TFT_eSPI.h>
#include <SPI.h>

//
#ifndef TFT_DISPOFF
#define TFT_DISPOFF 0x28
#endif

#ifndef TFT_SLPIN
#define TFT_SLPIN 0x10
#endif

#define TFT_MOSI 19
#define TFT_SCLK 18
#define TFT_CS 5
#define TFT_DC 16
#define TFT_RST 23
#define TOUCH_CS 5

#include "Button2.h"
#define TFT_BL 4 // Display backlight control pin
#define ADC_EN 14
#define ADC_PIN 34
#define BUTTON_1 35
#define BUTTON_2 0

TFT_eSPI tft = TFT_eSPI(135, 240);

#define TFT_AQUA 0x04FF
#define TFT_GRAY 0x8410

Button2 btn1(BUTTON_1);
Button2 btn2(BUTTON_2);
static String fiat = "USD";
static String ticker[] = {"BTC", "LTC", "ETH", "XRP", "CHZ", "DOGE", "UNI", "SEED", "BAT", "SUSHI", "CAKE"};
int selected = -1;

void clear()
{
    tft.fillRect(0, 37, 240, 73, TFT_BLACK); //price
    //tft.fillRect(0, 80, 229, 20, TFT_BLACK); //rank
    tft.drawLine(0, 115, 240, 115, TFT_DARKGREY);
    //tft.fillRect(100, 110, 140, 25, TFT_BLACK); // %
    tft.fillRect(0, 120, 240, 15, TFT_BLACK);
        
    tft.drawLine(120, 115, 120, 135, TFT_DARKGREY);
}

unsigned long debounce = 0;
unsigned long now = 0;

void drawCentreString(String buf, int x, int y, uint8_t font)
{
    int16_t x1, y1;
    uint16_t w, h, d;
    w = tft.textWidth(buf.c_str(), font);
    h = tft.fontHeight(font);
    Serial.println("========================");
    w = x - w/2;
    h = y - h/2;
    tft.drawString(buf, w, h, font);
    
    Serial.println("========================");
}

void change()
{
    selected = (selected + 1) % 8;
    String t = ticker[selected];
    clear();
    tft.fillRect(5, 5, 100, 20, TFT_BLACK); //ticker symbol
    tft.setTextColor(TFT_WHITE);

    tft.drawString(t + " " + fiat, 5, 5, 2);
    if (debounce == 0 || millis() - debounce <= 1000)
    {
        debounce = millis();
    }
}
void button_init()
{
    btn1.setPressedHandler([](Button2 &b) {
        change();
    });

    btn2.setPressedHandler([](Button2 &b) {
        Serial.println("updating " + ticker[selected]);
        print();
    });
}

void button_loop()
{
    btn1.loop();
    btn2.loop();
    if (debounce > 0 && millis() - debounce > 1000)
    {
        print();
        debounce = 0;
    }
}

// begin setup
void setup()
{
    Serial.begin(115200);
    Serial.println(F("Boot Ticker"));

    tft.setRotation(3); //rotate 90 degree
    tft.fillScreen(TFT_BLACK);

    Serial.println(tft.width());
    Serial.println(tft.height());

    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);

    uint16_t time = millis();
    time = millis() - time;
    tft.init();
    tft.fillScreen(TFT_BLACK);
    tft.setSwapBytes(true);
    tft.pushImage(0, 0, btclogoWidth, btclogoHeight, btclogo);
    delay(3000);

    // Starup
    tft.fillScreen(TFT_BLACK);
    Serial.print("Connecting to ");
    tft.drawString("Connecting to ", 15, 10, 2);
    
    Serial.println(wifi_ssid);
    tft.drawString(wifi_ssid, 15, 25, 2);
    tft.pushImage(200, 2, infoWidth, infoHeight, info);
    delay(1000);

    WiFi.begin(wifi_ssid, wifi_password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    tft.setTextColor(TFT_GREEN);
    tft.drawString("WiFi connected", 15, 40, 2);
    tft.setTextColor(TFT_WHITE);
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    delay(1000);
    tft.fillRect(0, 0, 240, 135, TFT_BLACK);

    //tft.pushImage(5, 15, btcWidth, btcHeight, btc);

    button_init();
}

void printTickerData(String ticker)
{
    Serial.println("---------------------------------");
    Serial.println("Getting ticker data for " + ticker);

    //For the new API, you can use the currency ID or abbreviated name, such as
    //Bitcoin, you can view the letter after Circulating Supply at https://coinmarketcap.com/, it is BTC

    CMCTickerResponse response = api.GetTickerInfo(ticker, fiat);
    if (response.error == "")
    {
        tft.setTextColor(TFT_YELLOW);

        if (response.percent_change_1h < 0)
        {
            tft.setTextColor(TFT_RED);
        }

        if (response.percent_change_1h > 0)
        {
            tft.setTextColor(TFT_GREEN);
        }

        int length = 6;
        if (response.price > 100000) {
          length = 4;
        }
        tft.fillRect(0, 37, 240, 73, TFT_BLACK); 
        drawCentreString(String(response.price), 120, 74, 6);
        //tft.drawString(String(response.price).c_str(), 5, 37, length);

        tft.drawLine(0, 115, 240, 115, TFT_DARKGREY);
        
        tft.fillRect(0, 120, 240, 15, TFT_BLACK);
        
        tft.drawLine(120, 115, 120, 135, TFT_DARKGREY);
        // 1h change
        tft.drawString("% 1h  "+String(response.percent_change_1h), 5, 120, 2);

        // 24 hours change
        tft.setTextColor(TFT_YELLOW);
        if (response.percent_change_24h < 0)
        {
            tft.setTextColor(TFT_RED);
        }
        if (response.percent_change_24h > 0)
        {
            tft.setTextColor(TFT_GREEN);
        }
        tft.drawString("% 24h  "+ String(response.percent_change_24h), 125, 120, 2);
    }

    else
    {
        Serial.print("Error getting data: ");
        Serial.println(response.error);
        tft.fillRect(200, 2, 40, 32, TFT_BLACK); //wifi RSSI and alert
        tft.pushImage(203, 2, alertWidth, alertHeight, alert);
    }
    Serial.println("---------------------------------");
}

float RSSI = 0.0;
unsigned long lastWIFI = 0;
void wifi()
{
    unsigned long now = millis();
    if (now - lastWIFI < 10000) {
      return;
    }
    lastWIFI = now;
    // int signal bars
    Serial.print("WiFi Signal strength: ");
    Serial.print(WiFi.RSSI());
    tft.fillRect(200, 2, 40, 32, TFT_BLACK); //wifi RSSI and alert

    int bars;
    RSSI = WiFi.RSSI();
    uint16_t color = TFT_GRAY;
    if (RSSI >= -55)
    {
        bars = 5;
        Serial.println(" 5 bars");
        color = TFT_GREEN;
    }
    else if (RSSI < -55 & RSSI >= -65)
    {
        bars = 4;
        Serial.println(" 4 bars");
        color = TFT_GREEN;
    }
    else if (RSSI < -65 & RSSI >= -70)
    {
        bars = 3;
        Serial.println(" 3 bars");
        color = TFT_YELLOW;
    }
    else if (RSSI < -70 & RSSI >= -78)
    {
        bars = 2;
        Serial.println(" 2 bars");
        color = TFT_YELLOW;
    }
    else if (RSSI < -78 & RSSI >= -82)
    {
        bars = 1;
        Serial.println(" 1 bars");
        color = TFT_RED;
    }
    else
    {
        bars = 0;
        Serial.println(" 0 bars");
        color = TFT_RED;
    }

    // signal bars
    for (int b = 0; b <= bars; b++)
    {
        tft.fillRect(202 + (b * 6), 23 - (b * 4), 5, b * 4, color);
    }
    
}

void print() {
  printTickerData(ticker[selected]);
}
void loop()
{
    button_loop();
    wifi();
    if (millis() > api_due_time)
    {
        change();
        //print();
        api_due_time = millis() + api_mtbs;
    }
}
