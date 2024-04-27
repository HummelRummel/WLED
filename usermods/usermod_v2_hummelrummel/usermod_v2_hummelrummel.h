#pragma once

#include "wled.h"

/*
 *
 * This Mod enables the HummelRummel interactive toy functionality. It extends WLED with some flow effects, adds support for gyro sensor and modifies the button behaviour.
 * Currently implemented:
 *   - promotes it's ip on mqttConnect, so it can easily be detected
 *   - TBD replacement of normal button handler to remove, reset and other hiffen features
 *   - TBD Changed switch button handling to directly send the on/off signals of the button
 *   - TBD Add configurable callbacks for button change (this should be more reliable than mqtt)
 *   - TBD Check if it makes sense to change the callbacks to a opern tcp connection (bi-directional)
 *   - TBD bring the flow effects to the new branch
 *   - TBD integrate ADXL sensor into HummelRummelUsermod (as import)
 * 
 */

#define HUMMELRUMMEL_USERMOD "HummelRummelUsermod"

class HummelRummelUsermod : public Usermod {

  private:
    // Private class members. You can declare variables and functions only accessible to your usermod here
    bool initDone = false;
    bool enabled = false;
    unsigned long lastTime = 0;

  public:

    void setup() {
      initDone = true;
    }

    void connected() {
     }

    void loop() {
      // if usermod is disabled or called during strip updating just exit
      // NOTE: on very long strips strip.isUpdating() may always return true so update accordingly
      if (!enabled || strip.isUpdating()) return;

      // do your magic here
      if (millis() - lastTime > 1000) {
        lastTime = millis();
        Serial.println("AOM");
      }
    }

    void addToJsonInfo(JsonObject& root)
    {
      // if "u" object does not exist yet wee need to create it
      JsonObject user = root["u"];
      if (user.isNull()) user = root.createNestedObject("u");
    }

    void addToJsonState(JsonObject& root)
    {
    }

    void readFromJsonState(JsonObject& root)
    {
    }

    void addToConfig(JsonObject& root)
    {
      JsonObject top = root.createNestedObject("HummelRummelUsermod");
      top["enabled"] = enabled;
    }

    bool readFromConfig(JsonObject& root)
    {
      JsonObject top = root[HUMMELRUMMEL_USERMOD];

      bool configComplete = !top.isNull();

      configComplete &= getJsonValue(top["enabled"], enabled);

      return configComplete;
    }

    void appendConfigData()
    {
    }

    void handleOverlayDraw()
    {
    }

    bool handleButton(uint8_t b) {
      yield();
      // ignore certain button types as they may have other consequences
      if (!enabled
       || buttonType[b] == BTN_TYPE_NONE
       || buttonType[b] == BTN_TYPE_RESERVED
       || buttonType[b] == BTN_TYPE_PIR_SENSOR
       || buttonType[b] == BTN_TYPE_ANALOG
       || buttonType[b] == BTN_TYPE_ANALOG_INVERTED) {
        return false;
      }

      bool handled = false;
      // do your button handling here
      return handled;
    }
  
#ifndef WLED_DISABLE_MQTT
    void onMqttConnect(bool sessionPresent) {
      // if somehow no device topic is set
      if (!mqttDeviceTopic[0]) {
        return;
      }

      //Check if MQTT Connected, otherwise it will crash the 8266
      if (WLED_MQTT_CONNECTED) {
        char ip[16] = "";
        IPAddress localIP = Network.localIP();
        sprintf(ip, "%d.%d.%d.%d", localIP[0], localIP[1], localIP[2], localIP[3]);

        char topic[64];
        strcpy(topic, mqttDeviceTopic);
        strcat_P(topic, PSTR("/ip"));
        mqtt->publish(topic, 0, false, ip);
      }
    }
#endif

    /**
     * onStateChanged() is used to detect WLED state change
     * @mode parameter is CALL_MODE_... parameter used for notifications
     */
    void onStateChange(uint8_t mode) {
      // do something if WLED state changed (color, brightness, effect, preset, etc)
    }

    uint16_t getId()
    {
      return USERMOD_ID_HUMMELRUMMEL;
    }
};
