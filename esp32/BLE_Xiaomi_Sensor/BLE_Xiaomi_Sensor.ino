#include "BLEDevice.h"
#include <Wifi.h>
#include "HTTPClient.h"


static BLEAddress addr((std::string) "a4:c1:38:d2:a1:da");
static BLEUUID serviceUUID("ebe0ccb0-7a0a-4b0c-8a1a-6ff2997da3a6");
static BLEUUID charUUID("ebe0ccc1-7a0a-4b0c-8a1a-6ff2997da3a6");

static boolean doConnect = true;
static boolean connected = false;
static boolean done = false;

static BLEClient *pClient;
static BLERemoteCharacteristic *pRemoteCharacteristic;
static BLERemoteDescriptor *descriptor;

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

    String serverPath = serverName + "&temp=" + String(temp) + "&hum=" + String(hum);

    // Your Domain name with URL path or IP address with path
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
    BLERemoteCharacteristic *pBLERemoteCharacteristic,
    uint8_t *pData,
    size_t length,
    bool isNotify)
{
  float temp = (pData[0] | (pData[1] << 8)) * 0.01;
  int hum = *(pData + 2);
  doSomething(temp, hum);
  done = true;
  return;
}

void connectToServer(BLEAddress pAddress)
{
  Serial.print("Connecting to ");
  Serial.println(pAddress.toString().c_str());

  pClient = BLEDevice::createClient();
  Serial.println("Client created.");

  // Connect to the remove BLE Server.
  pClient->connect(pAddress);
  Serial.println("Connected to the server.");

  // Obtain a reference to the service we are after in the remote BLE server.

  BLERemoteService *pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr)
  {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(serviceUUID.toString().c_str());
    return;
  }

  // Obtain a reference to the characteristic in the service of the remote BLE server.
  pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
  if (pRemoteCharacteristic == nullptr)
  {
    Serial.print("Failed to find our characteristic UUID: ");
    Serial.println(charUUID.toString().c_str());
    return;
  }
  Serial.println("Waiting data.");
  pRemoteCharacteristic->registerForNotify(notifyCallback);
}

void disconnect()
{
  pRemoteCharacteristic->registerForNotify(NULL);
  pClient->disconnect();
}

void setupWIFI()
{
  Serial.println();
  preferences.begin("credentials", false);

  ssid = preferences.getString("ssid", "");
  password = preferences.getString("password", "");

  if (ssid == "" || password == "")
  {
    Serial.print("Failed to find our service UUID: ");
    Serial.println(serviceUUID.toString().c_str());
    return;
  }

  // Obtain a reference to the characteristic in the service of the remote BLE server.
  pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
  if (pRemoteCharacteristic == nullptr)
  {
    Serial.print("Failed to find our characteristic UUID: ");
    Serial.println(charUUID.toString().c_str());
    return;
  }
  Serial.println("Waiting data.");
  pRemoteCharacteristic->registerForNotify(notifyCallback);
}

void disconnect()
{
  pRemoteCharacteristic->registerForNotify(NULL);
  pClient->disconnect();
}

void setup()
{
  Serial.begin(115200);
  setupWIFI();
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("");
}

void loop()
{

  if (doConnect == true)
  {
    connectToServer(addr);
    doConnect = false;
    connected = true;
  }

  if (done)
  {
    Serial.println("Done.");
    disconnect();
    doConnect = true;
    connected = false;
    done = false;
    delay(30000);
  }
}
