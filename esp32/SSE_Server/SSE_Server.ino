#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiClient.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include <Arduino_JSON.h>
#include <Adafruit_BMP085.h>
#include "DHT.h"

#define DHTPIN 4 // Digital pin connected to the DHT sensor

#define DHTTYPE DHT11 // DHT 11

#define LED 2

DHT dht(DHTPIN, DHTTYPE);

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Create an Event Source on /events
AsyncEventSource events("/events");

// Search for parameter in HTTP POST request
const char *PARAM_INPUT_1 = "ssid";
const char *PARAM_INPUT_2 = "pass";

//Variables to save values from HTML form
String ssid;
String pass;

// File paths to save input values permanently
const char *ssidPath = "/ssid.txt";
const char *passPath = "/pass.txt";

Adafruit_BMP085 bmp;

// Set your Gateway IP address
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 0, 0);

// Timer variables (check wifi)
unsigned long previousMillis = 0;
const long interval = 10000; // interval to wait for Wi-Fi connection (milliseconds)

//Variables to hold sensor readings
float temp;
float hum;
float pres;
float alt;
float hic;

// Json Variable to Hold Sensor Readings
JSONVar readings;

// Timer variables (get sensor readings)
unsigned long lastTime = 0;
unsigned long timerDelay = 5000;

//-----------------FUNCTIONS TO HANDLE SENSOR READINGS-----------------//

void led(int onoff)
{
  digitalWrite(LED, onoff);
}

// Return JSON String from sensor Readings
String getJSONReadings()
{
  readings["temperature"] = String(temp);
  readings["humidity"] = String(hum);
  readings["pressure"] = String(pres);
  readings["altitude"] = String(alt);
  readings["heat"] = String(hic);
  String jsonString = JSON.stringify(readings);
  return jsonString;
}

//-----------------FUNCTIONS TO HANDLE SPIFFS AND FILES-----------------//

// Initialize SPIFFS
void initSPIFFS()
{
  if (!SPIFFS.begin(true))
  {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  else
  {
    Serial.println("SPIFFS mounted successfully");
  }
}

// Read File from SPIFFS
String readFile(fs::FS &fs, const char *path)
{
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path);
  if (!file || file.isDirectory())
  {
    Serial.println("- failed to open file for reading");
    return String();
  }

  String fileContent;
  while (file.available())
  {
    fileContent = file.readStringUntil('\n');
    break;
  }
  return fileContent;
}

// Write file to SPIFFS
void writeFile(fs::FS &fs, const char *path, const char *message)
{
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file)
  {
    Serial.println("- failed to open file for writing");
    return;
  }
  if (file.print(message))
  {
    Serial.println("- file written");
  }
  else
  {
    Serial.println("- frite failed");
  }
}

// Initialize WiFi
bool initWiFi()
{
  if (ssid == "")
  {
    Serial.println("Undefined SSID or IP address.");
    return false;
  }

  WiFi.mode(WIFI_STA);

  WiFi.begin(ssid.c_str(), pass.c_str());
  Serial.println("Connecting to WiFi...");

  unsigned long currentMillis = millis();
  previousMillis = currentMillis;

  while (WiFi.status() != WL_CONNECTED)
  {
    currentMillis = millis();
    if (currentMillis - previousMillis >= interval)
    {
      Serial.println("Failed to connect.");
      return false;
    }
  }

  Serial.println(WiFi.localIP());
  return true;
}

void setup()
{
  // Serial port for debugging purposes
  Serial.begin(115200);

  pinMode(LED, OUTPUT);

  if (!bmp.begin())
  {
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
    while (1)
    {
    }
  }

  dht.begin();

  // Init SPIFFS
  initSPIFFS();

  // Load values saved in SPIFFS
  ssid = readFile(SPIFFS, ssidPath);
  pass = readFile(SPIFFS, passPath);

  if (initWiFi())
  {
    // If ESP32 inits successfully in station mode light up all pixels in a teal color
    Serial.println("Connected");
    led(HIGH);
    //Handle the Web Server in Station Mode
    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(SPIFFS, "/index.html", "text/html");
    });
    server.serveStatic("/", SPIFFS, "/");

    // Request for the latest sensor readings
    server.on("/readings", HTTP_GET, [](AsyncWebServerRequest *request) {
      getSensorReadings();
      String json = getJSONReadings();
      request->send(200, "application/json", json);
      json = String();
    });

    events.onConnect([](AsyncEventSourceClient *client) {
      if (client->lastId())
      {
        Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
      }
    });
    server.addHandler(&events);
    if (!MDNS.begin("esp32"))
    {
      Serial.println("Error setting up MDNS responder!");
      while (1)
      {
        delay(1000);
      }
    }
    server.begin();
    MDNS.addService("http", "tcp", 80);
  }
  else
  {
    // else initialize the ESP32 in Access Point mode
    // light up all pixels in a red color

    // Set Access Point
    Serial.println("Setting AP (Access Point)");
    // NULL sets an open Access Point
    WiFi.softAP("ESP-WIFI-MANAGER", NULL);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);

    // Web Server Root URL For WiFi Manager Web Page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(SPIFFS, "/wifimanager.html", "text/html");
    });

    server.serveStatic("/", SPIFFS, "/");

    // Get the parameters submited on the form
    server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
      int params = request->params();
      for (int i = 0; i < params; i++)
      {
        AsyncWebParameter *p = request->getParam(i);
        if (p->isPost())
        {
          // HTTP POST ssid value
          if (p->name() == PARAM_INPUT_1)
          {
            ssid = p->value().c_str();
            Serial.print("SSID set to: ");
            Serial.println(ssid);
            // Write file to save value
            writeFile(SPIFFS, ssidPath, ssid.c_str());
          }
          // HTTP POST pass value
          if (p->name() == PARAM_INPUT_2)
          {
            pass = p->value().c_str();
            Serial.print("Password set to: ");
            Serial.println(pass);
            // Write file to save value
            writeFile(SPIFFS, passPath, pass.c_str());
          }
        }
      }
      delay(3000);
      // After saving the parameters, restart the ESP32
      ESP.restart();
    });
    if (!MDNS.begin("esp32"))
    {
      Serial.println("Error setting up MDNS responder!");
      while (1)
      {
        delay(1000);
      }
    }
    server.begin();

    // Add service to MDNS-SD
    MDNS.addService("http", "tcp", 80);
  }
}

void getSensorReadings()
{
  Serial.print("Temperature = ");

  temp = bmp.readTemperature();
  Serial.print(temp);
  Serial.println(" *C");

  Serial.print("Pressure = ");
  pres = bmp.readPressure();
  Serial.print(pres);
  Serial.println(" Pa");

  // Calculate altitude assuming 'standard' barometric
  // pressure of 1013.25 millibar = 101325 Pascal
  Serial.print("Altitude = ");
  alt = bmp.readAltitude();
  Serial.print(alt);
  Serial.println(" meters");

  hum = dht.readHumidity();
  // Read temperature as Celsius (the default)
  hic = dht.computeHeatIndex(dht.readTemperature(), hum, false);

  Serial.println("sensor reading");
}

void loop()
{
  // If the ESP32 is set successfully in station mode...
  if (WiFi.status() == WL_CONNECTED)
  {

    //...Send Events to the client with sensor readins and update colors every 30 seconds
    if (millis() - lastTime > timerDelay)
    {
      getSensorReadings();

      String message = getJSONReadings();
      events.send(message.c_str(), "new_readings", millis());
      lastTime = millis();
    }
  }
}
