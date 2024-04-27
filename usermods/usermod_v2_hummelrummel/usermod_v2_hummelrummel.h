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

// DEBUG
typedef enum
{
  NONE = 0,
  SSD1306,       // U8X8_SSD1306_128X32_UNIVISION_HW_I2C
  SH1106,        // U8X8_SH1106_128X64_WINSTAR_HW_I2C
  SSD1306_64,    // U8X8_SSD1306_128X64_NONAME_HW_I2C
  SSD1305,       // U8X8_SSD1305_128X32_ADAFRUIT_HW_I2C
  SSD1305_64,    // U8X8_SSD1305_128X64_ADAFRUIT_HW_I2C
  SSD1306_SPI,   // U8X8_SSD1306_128X32_NONAME_HW_SPI
  SSD1306_SPI64, // U8X8_SSD1306_128X64_NONAME_HW_SPI
  SSD1309_SPI64  // U8X8_SSD1309_128X64_NONAME0_4W_HW_SPI
} DisplayType;
#ifndef FLD_TYPE
#define FLD_TYPE SSD1306
#endif
// DEBUG

class HummelRummelUsermod : public Usermod
{
private:
  // global usermod
  // peristent config
  bool enabled = false;
  // volatile states
  bool initDone = false;
  unsigned long lastTime = 0;

  // button handling
  // peristent config
  bool buttonRawValue;
  // volatile states
  bool buttonLastState[WLED_MAX_BUTTONS];

  // DEBUG
  DisplayType type = FLD_TYPE; // display type
  // DEBUG

public:
  void setup()
  {
    initDone = true;
  }

  void connected()
  {
  }

  void loop()
  {
    // if usermod is disabled or called during strip updating just exit
    // NOTE: on very long strips strip.isUpdating() may always return true so update accordingly
    if (!enabled || strip.isUpdating())
      return;

    // do your magic here
    if (millis() - lastTime > 1000)
    {
      lastTime = millis();
      Serial.println("AOM");
    }
  }

  void addToJsonInfo(JsonObject &root)
  {
    // if "u" object does not exist yet wee need to create it
    JsonObject user = root["u"];
    if (user.isNull())
      user = root.createNestedObject("u");
  }

  void addToJsonState(JsonObject &root)
  {
  }

  void readFromJsonState(JsonObject &root)
  {
  }

  void addToConfig(JsonObject &root)
  {
    JsonObject top = root.createNestedObject("HummelRummelUsermod");
    top["enabled"] = enabled;
    top["button-raw-value"] = buttonRawValue;

    // FOR DEBUG
    top["type"] = type;
    // FOR DEBUG
  }

  bool readFromConfig(JsonObject &root)
  {
    JsonObject top = root[HUMMELRUMMEL_USERMOD];

    bool configComplete = !top.isNull();

    configComplete &= getJsonValue(top["enabled"], enabled, true);
    configComplete &= getJsonValue(top["button-raw-value"], buttonRawValue, false);

    // FOR DEBUG
    configComplete &= getJsonValue(top["type"], type, SSD1305);
    // FOR DEBUG

    return configComplete;
  }

  void appendConfigData()
  {
    oappend(SET_F("dd=addDropdown('HummelRummelUsermod','type');"));
    oappend(SET_F("addOption(dd,'None',0);"));
    oappend(SET_F("addOption(dd,'SSD1306',1);"));
    oappend(SET_F("addOption(dd,'SH1106',2);"));
    oappend(SET_F("addOption(dd,'SSD1306 128x64',3);"));
    oappend(SET_F("addOption(dd,'SSD1305',4);"));
    oappend(SET_F("addOption(dd,'SSD1305 128x64',5);"));
    oappend(SET_F("addOption(dd,'SSD1306 SPI',6);"));
    oappend(SET_F("addOption(dd,'SSD1306 SPI 128x64',7);"));
    oappend(SET_F("addOption(dd,'SSD1309 SPI 128x64',8);"));
    oappend(SET_F("addInfo('HummelRummelUsermod:type',1,'<br><i class=\"warn\">Change may require reboot</i>','');"));
  }

  void handleOverlayDraw()
  {
  }

#ifndef WLED_DISABLE_MQTT
  void onMqttConnect(bool sessionPresent)
  {
    // if somehow no device topic is set
    if (!mqttDeviceTopic[0])
    {
      return;
    }

    // Check if MQTT Connected, otherwise it will crash the 8266
    if (WLED_MQTT_CONNECTED)
    {
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
  void onStateChange(uint8_t mode)
  {
    // do something if WLED state changed (color, brightness, effect, preset, etc)
  }

  uint16_t getId()
  {
    return USERMOD_ID_HUMMELRUMMEL;
  }

  bool handleButton(uint8_t b);
};

// Copied from button implementation but it's actually independent
#define WLED_DEBOUNCE_THRESHOLD 50    // only consider button input of at least 50ms as valid (debouncing)
#define WLED_LONG_PRESS 600           // long press if button is released after held for at least 600ms
#define WLED_DOUBLE_PRESS 350         // double press if another press within 350ms after a short press
#define WLED_LONG_REPEATED_ACTION 300 // how often a repeated action (e.g. dimming) is fired on long press on button IDs >0

bool HummelRummelUsermod::handleButton(uint8_t b)
{
  yield();
  // ignore certain button types as they may have other consequences
  if (!enabled || buttonType[b] == BTN_TYPE_NONE || buttonType[b] == BTN_TYPE_RESERVED || buttonType[b] == BTN_TYPE_PIR_SENSOR || buttonType[b] == BTN_TYPE_ANALOG || buttonType[b] == BTN_TYPE_ANALOG_INVERTED)
  {
    return false;
  }

  // Replaced the button implementaion for momentary buttons
  // When the usermod is enabled the following button interactions differ from the default:
  // - removed the reset feature when pressing longer than 5sec
  unsigned long now = millis();
  if (buttonRawValue)
  {
    // things has changed, just update the timer
    if (isButtonPressed(b) == buttonLastState[b])
    {
      buttonPressedTime[b] = 0;
      return true;
    }
    if (buttonPressedTime[b] == 0)
    {
      buttonPressedTime[b] = now;
    }

    long dur = now - buttonPressedTime[b];
    if (dur > WLED_DEBOUNCE_THRESHOLD)
    {
      buttonLastState[b] = !buttonLastState[b];

      // apply the macro if one is defined for the
      if (macroButton[b])
        applyPreset(macroButton[b], CALL_MODE_BUTTON_PRESET);

      // do the callback dance
      if (WLED_MQTT_CONNECTED)
      {
        char topic[64];
        strcpy(topic, mqttDeviceTopic);
        strcat_P(topic, PSTR("/btn"));
        if (buttonLastState[b])
        {
          mqtt->publish(topic, 0, false, "ON");
        }
        else
        {
          mqtt->publish(topic, 0, false, "OFF");
        }
      }

      return true;
    }
  }

  // momentary button logic
  if (isButtonPressed(b))
  { // pressed
    // if all macros are the same, fire action immediately on rising edge
    if (macroButton[b] && macroButton[b] == macroLongPress[b] && macroButton[b] == macroDoublePress[b])
    {
      if (!buttonPressedBefore[b])
        shortPressAction(b);
      buttonPressedBefore[b] = true;
      buttonPressedTime[b] = now; // continually update (for debouncing to work in release handler)
      return true;
    }

    if (!buttonPressedBefore[b])
      buttonPressedTime[b] = now;
    buttonPressedBefore[b] = true;

    if (now - buttonPressedTime[b] > WLED_LONG_PRESS)
    { // long press
      if (!buttonLongPressed[b])
        longPressAction(b);
      else if (b)
      { // repeatable action (~3 times per s) on button > 0
        longPressAction(b);
        buttonPressedTime[b] = now - WLED_LONG_REPEATED_ACTION; // 333ms
      }
      buttonLongPressed[b] = true;
    }
  }
  else if (!isButtonPressed(b) && buttonPressedBefore[b])
  { // released
    long dur = now - buttonPressedTime[b];

    // released after rising-edge short press action
    if (macroButton[b] && macroButton[b] == macroLongPress[b] && macroButton[b] == macroDoublePress[b])
    {
      if (dur > WLED_DEBOUNCE_THRESHOLD)
        buttonPressedBefore[b] = false; // debounce, blocks button for 50 ms once it has been released
      return true;
    }

    if (dur < WLED_DEBOUNCE_THRESHOLD)
    {
      buttonPressedBefore[b] = false;
      return true;
    } // too short "press", debounce
    bool doublePress = buttonWaitTime[b]; // did we have a short press before?
    buttonWaitTime[b] = 0;

    if (!buttonLongPressed[b])
    { // short press
      // NOTE: this interferes with double click handling in usermods so usermod needs to implement full button handling
      if (b != 1 && !macroDoublePress[b])
      { // don't wait for double press on buttons without a default action if no double press macro set
        shortPressAction(b);
      }
      else
      { // double press if less than 350 ms between current press and previous short press release (buttonWaitTime!=0)
        if (doublePress)
        {
          doublePressAction(b);
        }
        else
        {
          buttonWaitTime[b] = now;
        }
      }
    }
    buttonPressedBefore[b] = false;
    buttonLongPressed[b] = false;
  }

  // if 350ms elapsed since last short press release it is a short press
  if (buttonWaitTime[b] && now - buttonWaitTime[b] > WLED_DOUBLE_PRESS && !buttonPressedBefore[b])
  {
    buttonWaitTime[b] = 0;
    shortPressAction(b);
  }

  // do your button handling here
  return true;
}
