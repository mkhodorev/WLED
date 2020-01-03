/*
 * This file allows you to add own functionality to WLED more easily
 * See: https://github.com/Aircoookie/WLED/wiki/Add-own-functionality
 * EEPROM bytes 2750+ are reserved for your custom use case. (if you extend #define EEPSIZE in wled01_eeprom.h)
 * bytes 2400+ are currently ununsed, but might be used for future wled features
 */

//Use userVar0 and userVar1 (API calls &U0=,&U1=, uint16_t)

//gets called once at boot. Do all initialization that doesn't depend on network here
void userSetup()
{
  switchStatus = 0;
  #if SWITCH_PIN >= 0
    pinMode(SWITCH_PIN, OUTPUT);
    #if SWITCH_MDE
      digitalWrite(SWITCH_PIN, 0);
    #else
      digitalWrite(SWITCH_PIN, 1);
    #endif
  #endif
}

//gets called every time WiFi is (re-)connected. Initialize own network interfaces here
void userConnected()
{

}

//loop. You can use "if (WLED_CONNECTED)" to check for successful connection
void userLoop()
{
  
}

void processCustomSwitch(char* payload)
{ 
  #if SWITCH_PIN >= 0
    if (strstr(payload, "ON") || strstr(payload, "on") || strstr(payload, "true"))
    {
      onSwitch();
    } else if (strstr(payload, "OFF") || strstr(payload, "off") || strstr(payload, "false"))
    {
      offSwitch();
    } else if (strstr(payload, "t") || strstr(payload, "T"))
    {
      toggleSwitch();
    }
  #endif
}

void onSwitch()
{
  #if SWITCH_PIN >= 0
    switchStatus = 1;
    digitalWrite(SWITCH_PIN, SWITCH_MDE);
    publishMqttSwitchStatus();
  #endif
}

void offSwitch()
{
  #if SWITCH_PIN >= 0
    switchStatus = 0;
    digitalWrite(SWITCH_PIN, !SWITCH_MDE);
    publishMqttSwitchStatus();
  #endif
}

void toggleSwitch()
{
  #if SWITCH_PIN >= 0
    if (switchStatus)
      offSwitch();
     else
      onSwitch();
    publishMqttSwitchStatus();
  #endif
}

void publishMqttSwitchStatus()
{
  if (mqtt == nullptr || !mqtt->connected()) return;
  DEBUG_PRINTLN("Publish MQTT Switch Status");

  char subuf[38];
  strcpy(subuf, mqttDeviceTopic);
  strcat(subuf, "/switch/status");
  if (switchStatus)
    mqtt->publish(subuf, 0, true, "ON");
  else
    mqtt->publish(subuf, 0, true, "OFF");
}
