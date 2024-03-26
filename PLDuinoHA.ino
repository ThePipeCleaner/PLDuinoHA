#include <ArduinoHA.h>
#include <ArduinoHADefines.h>
#include <HADevice.h>
#include <HAMqtt.h>


#include <ESP8266WiFi.h>
#include <PLDuino.h>
#include "utils.h"

#define LED_PIN         D0
#define BROKER_ADDR     IPAddress(192,168,0,17)
#define WIFI_SSID       "MyNetwork"
#define WIFI_PASSWORD   "MyPassword"

WiFiClient client;
HADevice device;
HAMqtt mqtt(client, device);

// "led" is unique ID of the switch. You should define your own ID.
HASwitch led("led");

void onSwitchCommand(bool state, HASwitch* sender)
{
    digitalWrite(LED_PIN, (state ? HIGH : LOW));
    sender->setState(state); // report state back to the Home Assistant
}



// "myCover" is unique ID of the cover. You should define your own ID.
HACover cover("myCover", HACover::PositionFeature);

// PositionFeature is optional, however required to be in a "stopped" state other than open or closed.
// If ommitted, a CommandStop will result in HA treating the cover as either Open or Closed immediately after the command.
// See https://www.home-assistant.io/integrations/cover.mqtt/
// Added in https://github.com/dawidchyrzynski/arduino-home-assistant/pull/111

void onCoverCommand(HACover::CoverCommand cmd, HACover* sender) {
    if (cmd == HACover::CommandOpen) {
        Serial.println("Command: Open");
        sender->setState(HACover::StateOpening); // report state back to the HA
    } else if (cmd == HACover::CommandClose) {
        Serial.println("Command: Close");
        sender->setState(HACover::StateClosing); // report state back to the HA
    } else if (cmd == HACover::CommandStop) {
        Serial.println("Command: Stop");
        sender->setState(HACover::StateStopped); // report state back to the HA
    }

    // Available states:
    // HACover::StateClosed
    // HACover::StateClosing
    // HACover::StateOpen
    // HACover::StateOpening
    // HACover::StateStopped

    // You can also report position using setPosition() method
}

void setup() {
    Serial.begin(9600);
    Serial.println("Starting...");




    // Unique ID must be set!
    byte mac[WL_MAC_ADDR_LENGTH];
    WiFi.macAddress(mac);
    device.setUniqueId(mac, sizeof(mac));

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    // connect to wifi
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(500); // waiting for the connection
    }
    Serial.println();
    Serial.println("Connected to the network");

    // set device's details (optional)
    device.setName("PLDuino");
    device.setSoftwareVersion("1.0.0");

    // handle switch state
    led.onCommand(onSwitchCommand);
    led.setName("My LED"); // optional
    
    
    
    
    cover.onCommand(onCoverCommand);
    cover.setName("My cover"); // optional

    // Optionally you can set retain flag for the HA commands
    // cover.setRetain(true);

    // Optionally you can enable optimistic mode for the HACover.
    // In this mode you won't need to report state back to the HA when commands are executed.
    // cover.setOptimistic(true);

    mqtt.begin(BROKER_ADDR);
}

void loop() {

    mqtt.loop();

    // You can also change the state at runtime as shown below.
    // This kind of logic can be used if you want to control your cover using a button connected to the device.
    // cover.setState(HACover::StateOpening); // use any state you want

    // You can also change the state at runtime as shown below.
    // This kind of logic can be used if you want to control your switch using a button connected to the device.
    // led.setState(true); // use any state you want

}
