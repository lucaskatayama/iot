#include <Arduino.h>
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(4, 5);
const byte address[] = "xupaxu";

void setup()
{
    Serial.begin(115200);
    // initialize the transceiver on the SPI bus
    if (!radio.begin())
    {
        Serial.println(F("radio hardware is not responding!!"));
        while (1)
        {
        } // hold in infinite loop
    }
    radio.openWritingPipe(address); //Setting the address where we will send the data
    radio.setPALevel(RF24_PA_MAX);  //You can set it as minimum or maximum depending on the distance between the transmitter and receiver.
    radio.stopListening();
    Serial.println("radio started");
}

void loop()
{
    if (radio.available())
    {
        Serial.println("on");
        char on[] = "on";
        radio.write(&on, sizeof(on));
        delay(1000);
        Serial.println("off");
        char off[] = "off";
        radio.write(&off, sizeof(off));
        delay(1000);
    }
}