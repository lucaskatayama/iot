//Include Libraries
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//create an RF24 object
RF24 radio(9, 8); // CE, CSN

//address through which two modules communicate.
const byte address[6] = "xupaxu";

void setup()
{
    Serial.begin(9600);
    Serial.println("starting");
    radio.begin();

    //set the address
    radio.openReadingPipe(0, address);

    //Set module as receiver
    radio.startListening();
    Serial.println("radio started");
}

void loop()
{
    //Read the data if available in buffer
    uint8_t pipe;
    char payload[32] = {};
    if (radio.available(&pipe))
    {                                           // is there a payload? get the pipe number that recieved it
        uint8_t bytes = radio.getPayloadSize(); // get the size of the payload
        radio.read(&payload, bytes);            // fetch payload from FIFO
        Serial.print(F("Received "));
        Serial.print(bytes); // print the size of the payload
        Serial.print(F(" bytes on pipe "));
        Serial.print(pipe); // print the pipe number
        Serial.print(F(": "));
        Serial.println(payload); // print the payload's value
        delay(1000);
    }

    else
    {
        Serial.println("not available");
        delay(1000);
    }
}