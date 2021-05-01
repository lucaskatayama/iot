
#include <Preferences.h>
#include "WiFi.h"

Preferences preferences;
String ssid;
String password;

void setup()
{
    Serial.begin(115200);
    Serial.println();
    preferences.begin("credentials", false);

    ssid = preferences.getString("ssid", "");
    password = preferences.getString("password", "");

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

void loop()
{
}
