# Xiaomi Temperature Sensor

![Sensor](https://i.imgur.com/B5J2PLV.png?1)

|                     |                                        |
|---------------------|----------------------------------------|
| Model               | `LYWSD03MMC` |
| Service UUID        | `ebe0ccb0-7a0a-4b0c-8a1a-6ff2997da3a6` |
| Characteristic UUID | `ebe0ccc1-7a0a-4b0c-8a1a-6ff2997da3a6` |

## Temprature calculation

```c++
float temp = (pData[0] | (pData[1] << 8)) * 0.01; 
```

## References

* [Github](https://github.com/polclota/esp32lywsd03mmc/blob/master/src/main.ino)
* [Working version for BLE](https://github.com/wakwak-koba/arduino-esp32)
