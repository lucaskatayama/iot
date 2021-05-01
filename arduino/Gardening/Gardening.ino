#include <Arduino.h>
#include <Adafruit_BMP085.h>
#include <DHT.h>

#define DHTPIN 2
#define DHTTYPE DHT11
#define LIGHT_PIN A0
#define SOIL_PIN A1
#define INTERVAL_PIN 8
#define RELAY_PIN 12

Adafruit_BMP085 bmp;
DHT dht(DHTPIN, DHTTYPE);
int dryValue = 0;
int wetValue = 1023;
int friendlyDryValue = 0;
int friendlyWetValue = 100;

float pres;
float temp;
float hum;
float temp2;
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
  hum = dht.readHumidity();
  light = analogRead(LIGHT_PIN);
  soil = map(analogRead(SOIL_PIN), 0, 1023, 0, 100);
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
