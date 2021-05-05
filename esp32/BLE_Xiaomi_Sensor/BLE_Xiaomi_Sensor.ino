#include <Arduino.h>
#include "BLEDevice.h"
#include <Preferences.h>
#include "WiFi.h"
#include <HTTPClient.h>

#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 5        /* Time ESP32 will go to sleep (in seconds) */

Preferences preferences;
String ssid;
String password;

static String serverName = "http://api.thingspeak.com/update?api_key=<APIKEY>";

static BLEAddress addr((std::string) "a4:c1:38:d2:a1:da");
static BLEUUID serviceUUID("ebe0ccb0-7a0a-4b0c-8a1a-6ff2997da3a6");
static BLEUUID charUUID("ebe0ccc1-7a0a-4b0c-8a1a-6ff2997da3a6");

static boolean doConnect = true;
static boolean done = false;

static BLERemoteCharacteristic *pRemoteCharacteristic;
static BLEClient *pClient;

static void doSomething(float temp, int hum)
{
  Serial.print("Temperature: ");
  Serial.print(temp);
  Serial.println("C");

  Serial.print("Humidity: ");
  Serial.print(hum);
  Serial.println("%");

  if (WiFi.status() == WL_CONNECTED)
  {
    HTTPClient http;

    String serverPath = serverName + "&field1=" + String(temp) + "&field2=" + String(hum);

    // Your Domain name with URL path or IP address with path
    Serial.println(serverPath);
    http.begin(serverPath.c_str());

    // Send HTTP GET request
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0)
    {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println(payload);
    }
    else
    {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    // Free resources
    http.end();
  }
  else
  {
    Serial.println("WiFi Disconnected");
  }
}

static void notifyCallback(
    BLERemoteCharacteristic *pRemoteCharacteristic,
    uint8_t *pData,
    size_t length,
    bool isNotify)
{
  float temp = (pData[0] | (pData[1] << 8)) * 0.01;
  int hum = *(pData + 2);
  Serial.println("data packet received");
  Serial.println(temp);
  Serial.println(hum);
  doSomething(temp, hum);
  done = true;
}

class MyClientCallback : public BLEClientCallbacks
{
  void onConnect(BLEClient *pClient)
  {
    Serial.println("Connecting to the server.");
    // Obtain a reference to the service we are after in the remote BLE server.
  }

  void onDisconnect(BLEClient *pclient)
  {

    Serial.println("onDisconnect");
  }
};

void connectToServer(BLEAddress pAddress)
{

  pClient = BLEDevice::createClient();
  Serial.println("Client created.");
  Serial.print("Connecting to ");
  Serial.println(pAddress.toString().c_str());
  pClient->connect(pAddress);

  BLERemoteService *pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr)
  {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(serviceUUID.toString().c_str());
    return;
  }
  Serial.println("service found");
  // Obtain a reference to the characteristic in the service of the remote BLE server.
  pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
  if (pRemoteCharacteristic == nullptr)
  {
    Serial.print("Failed to find our characteristic UUID: ");
    Serial.println(charUUID.toString().c_str());
    return;
  }
  Serial.println("characteristic found");
  Serial.println("Waiting data.");
  pRemoteCharacteristic->registerForNotify(notifyCallback);

  // Connect to the remove BLE Server.
}

void setupWIFI()
{
  Serial.println();
  preferences.begin("credentials", false);

  ssid = preferences.getString("ssid", "");
  password = preferences.getString("password", "");
  Serial.println(ssid);
  Serial.println(password);
  if (ssid == "" || password == "")
  {
    Serial.println("No values saved for ssid or password");
  }
  else
  {
    // Connect to Wi-Fi
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    Serial.print("Connecting to WiFi ..");
    while (WiFi.status() != WL_CONNECTED)
    {
      Serial.print('.');
      delay(1000);
    }
    Serial.println(WiFi.localIP());
  }
}

void setup()
{
  Serial.begin(115200);
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  setupWIFI();
  Serial.println("Starting Arduino BLE Client application...");
  setupWIFI();

  BLEDevice::init("");
}

void loop()
{

  if (doConnect == true)
  {
    connectToServer(addr);
    doConnect = false;
  }
  if (done)
  {
    pClient->disconnect();
    doConnect = true;
    done = false;
    delay(10000);
    
  }
}
