#include <Arduino.h>
#include <Adafruit_BMP085.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define DHTPIN 6
#define DHTTYPE DHT11
#define LIGHT_PIN A0
#define SOIL_PIN A1
#define INTERVAL_PIN 10
#define RELAY_PIN 12

#define DHTTYPE DHT11 // DHT 11

DHT_Unified dht(DHTPIN, DHTTYPE);

Adafruit_BMP085 bmp;
int dryValue = 0;
int wetValue = 1023;
int friendlyDryValue = 0;
int friendlyWetValue = 100;

float pres;
float temp;
float hum;
float light;
float soil;

unsigned long intervals[5] = {1000, 5000, 30000, 60000, 100};
int selected = 0;
unsigned long previous;

void setupSensors()
{
  // DHT11
  dht.begin();

  // BMP180
  if (!bmp.begin())
  {
    while (1)
    {
    }
  }
}

void setup()
{
  Serial.begin(9600);
  pinMode(INTERVAL_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, HIGH);
  setupSensors();
  while (!Serial)
  {
  }
  Serial.println("humidity,pressure,temperature,light,soil");
  previous = millis();
}

void readSensors()
{
  pres = bmp.readPressure() / 1000;
  temp = bmp.readTemperature();

  // dht11
  sensors_event_t event;

  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  hum = 0;
  if (!isnan(event.relative_humidity))
  {
    hum = event.relative_humidity;
  }

  light = map(analogRead(LIGHT_PIN), 0, 1023, 0, 100);
  soil = map(analogRead(SOIL_PIN), 0, 1023, 100, 0);
}

void changeInterval()
{
  if (digitalRead(INTERVAL_PIN) == HIGH)
  {
    selected = (selected + 1) % 5;
    delay(1000);
  }
}

void loop()
{
  changeInterval();
  unsigned long current = millis();
  unsigned long interval = intervals[selected];
  if ((current - previous) >= interval)
  {
    readSensors();
    Serial.print(hum);
    Serial.print(",");
    Serial.print(pres);
    Serial.print(",");
    Serial.print(temp);
    Serial.print(",");
    Serial.print(light);
    Serial.print(",");
    Serial.println(soil);
    previous = current;
  }
}
