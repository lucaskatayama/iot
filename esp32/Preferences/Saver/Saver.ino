
#include <Preferences.h>

Preferences preferences;

const char *ssid = "******************";
const char *password = "********************";

void setup()
{
    Serial.begin(115200);
    Serial.println();

    preferences.begin("credentials", false);
    preferences.putString("ssid", ssid);
    preferences.putString("password", password);

    Serial.println("Network Credentials Saved using Preferences");

    preferences.end();
}

void loop()
{
}
