# BLE Xiaomi Sensor

This project uses BLE Bluetooth to connect to a Xiaomi Temperature Sensor to retrieve `humidity` and `temperature` data and reports it to [ThingSpeak](https://thingspeak.com/)
platform.

There is no need for schematics, because it connects through Bluetooth and uses WiFi connection to send data to ThingSpeak.

## Bluetooth

The Bluetooth protocol transfers a series of records in the following form:

``` text
Service -> Characteristics -> Values
```

Each of one of that blocks has a specific UUID identifying them. So to select one, first you need to get the UUID.

For that, I used a mobile app **nRF Connect for Mobile** which uses phone Bluetooth to connect to the device and list all `Services` , `Characteristics` and their values.

To get the Temperature and Humidy characteristic, you use the `Descriptor` which has metadata about the characteristic.

I listed the UUIDs.

## Xiaomi Temperature Sensor

![Sensor](https://i.imgur.com/B5J2PLV.png?1)

|                     |                                        |
|---------------------|----------------------------------------|
| Model               | `LYWSD03MMC` |
| Service UUID        | `ebe0ccb0-7a0a-4b0c-8a1a-6ff2997da3a6` |
| Characteristic UUID | `ebe0ccc1-7a0a-4b0c-8a1a-6ff2997da3a6` |

## Data from Notification Callback

The Notification callback provides an pointer to the beginning of the data and its length.
We can iterates over all bytes and read the information.

The information provided from Xiaomi sensor comes in the form:

``` text
length: 5
[temperature] [temperature] [humidity] [battery] [battery]
```

The humidity is an percentage `integer` .

The temperature is calculated below:

## Temperature calculation

```c++
float temp = (pData[0] | (pData[1] << 8)) * 0.01; 
```

## More information

### Preferences and WiFi

I also use the `Preferences` library to save the WiFi SSID and password inside the ESP32.

### ThingSpeak

You need to create an account and generate an API key. Replace the `<APIKey>` in the source code.

## References

* [Basic Arduino C ESP32 code for Mijia LYWSD03MMC](https://github.com/polclota/esp32lywsd03mmc/blob/master/src/main.ino)
* [Working version for BLE](https://github.com/wakwak-koba/arduino-esp32)
* [ThingSpeak](https://thingspeak.com/)
* [nRF Connect for Mobile](https://play.google.com/store/apps/details?id=no.nordicsemi.android.mcp)
