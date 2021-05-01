#include "BLEDevice.h"

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

void setup()
{
  Serial.begin(115200);
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
