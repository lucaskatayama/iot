#include <Arduino.h>

#define RELAY1_PIN 2
#define RELAY2_PIN 3

#define LED_GREEN_PIN 5
#define LED_RED_PIN 6

#define LDR_PIN A0

#define WATER_LEVEL_PIN 10
#define PIR_PIN 11
#define SOIL_PIN 12
#define CHECK_INTERVAL 5000
#define LIGHT_THRESHOLD 50
#define LIGHT_INTERVAL 300000

// Create variables:
int val = 0;
bool motionState = false; // We start with no motion detected.
unsigned long lastPump = 0;
unsigned long lastLight = 0;

void toggleLight(boolean onoff)
{
    if (onoff)
    {
        digitalWrite(RELAY2_PIN, LOW);
    }
    else
    {
        digitalWrite(RELAY2_PIN, HIGH);
    }
}

void Pump()
{
    int water = digitalRead(WATER_LEVEL_PIN);

    if (water == LOW)
    {
        //turn off relay
        digitalWrite(RELAY1_PIN, HIGH);
        // turn red light
        digitalWrite(LED_RED_PIN, HIGH);
        digitalWrite(LED_GREEN_PIN, LOW);
        return;
    }
    else
    {
        digitalWrite(LED_RED_PIN, LOW);
    }
    unsigned long now = millis();
    if (now - lastPump > CHECK_INTERVAL)
    {
        int soil = digitalRead(SOIL_PIN);
        if (soil)
        {

            digitalWrite(RELAY1_PIN, LOW);
            digitalWrite(LED_GREEN_PIN, HIGH);
        }
        else
        {
            digitalWrite(RELAY1_PIN, HIGH);
            digitalWrite(LED_GREEN_PIN, LOW);
        }
        lastPump = now;
    }
}

void PIR()
{
    int ldr = map(analogRead(LDR_PIN), 0, 1023, 0, 100);
    if (millis() - lastLihgt >= LIGHT_INTERVAL && ldr >= LIGHT_THRESHOLD)
    {
        toggleLight(false);
        return;
    }
    // Read out the pirPin and store as val:
    val = digitalRead(PIR_PIN);
    // If motion is detected (pirPin = HIGH), do the following:
    if (val == HIGH)
    {
        // Change the motion state to true (motion detected):
        if (motionState == false)
        {
            motionState = true;
        }
    }
    // If no motion is detected (pirPin = LOW), do the following:
    else
    {
        // Change the motion state to false (no motion):
        if (motionState == true)
        {
            motionState = false;
        }
    }
    lastLight = millis();
    toggleLight(motionState);
}

void testPINS()
{
    digitalWrite(LED_GREEN_PIN, HIGH);
    digitalWrite(LED_RED_PIN, HIGH);
    digitalWrite(RELAY1_PIN, LOW);
    digitalWrite(RELAY2_PIN, LOW);
    delay(2000);
    digitalWrite(LED_GREEN_PIN, LOW);
    digitalWrite(LED_RED_PIN, LOW);
    digitalWrite(RELAY1_PIN, HIGH);
    digitalWrite(RELAY2_PIN, HIGH);
    delay(2000);
}

void setup()
{
    Serial.begin(9600);
    pinMode(WATER_LEVEL_PIN, INPUT);
    pinMode(SOIL_PIN, INPUT);
    pinMode(PIR_PIN, INPUT);
    pinMode(LDR_PIN, INPUT);

    pinMode(LED_GREEN_PIN, OUTPUT);
    pinMode(LED_RED_PIN, OUTPUT);
    pinMode(RELAY1_PIN, OUTPUT);
    pinMode(RELAY2_PIN, OUTPUT);

    testPINS();

    lastPump = millis();
    lastLight = millis();
    Serial.println("ldr,soil,water");
}

void loop()
{
    PIR();
    Pump();

    delay(500);
}