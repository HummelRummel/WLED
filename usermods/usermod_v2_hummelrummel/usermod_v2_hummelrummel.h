#pragma once

#include "wled.h"
#include "hr_flow_effects.h"
#include "hr_guitare_effect.h"

/*
 *
 * This Mod enables the HummelRummel interactive toy functionality. It extends WLED with some flow effects, adds support for gyro sensor and modifies the button behaviour.
 * Currently implemented:
 *   - promotes it's ip on mqttConnect, so it can easily be detected
 *   - replacement of normal button handler to remove, reset and other hidden features
 *   - Added parameter to switch between normal and on/off behaviour for push button
 *   - TBD Add configurable callbacks for button change (this should be more reliable than mqtt)
 *   - TBD Check if it makes sense to change the callbacks to a opern tcp connection (bi-directional)
 *   - bring the flow effects to the new branch
 *   - TBD integrate ADXL sensor into HummelRummelUsermod (as import)
 *
 */

class HummelRummelUsermod : public Usermod
{
private:
  // global usermod
  // peristent config
  bool enabled = false;
  bool initDone = false;
  static const char _name[];

  // button handling
  // peristent config
  bool buttonRawValue;
  // volatile states
  bool buttonLastState[WLED_MAX_BUTTONS];

  // guitare effect
  // presistent config
  bool enableGuitareMode;
  uint8_t guitareCorpusLedCnt;
  GuitareButton guitareButtons[WLED_MAX_BUTTONS - 1]; // the last button is not a guitare button but the state switch

public:
  // volatile states
  GuitareNote guitareNotes[MAX_GUITARE_NOTES];

  void setup()
  {
    strip.addEffect(255, &mode_fill_level_rainbow, _data_FX_MODE_FILL_LEVEL_RAINBOW);
    strip.addEffect(255, &mode_fill_level_color, _data_FX_MODE_FILL_LEVEL_COLOR);
    guitareNotesHack = this->guitareNotes;
    strip.addEffect(255, &mode_guitare, _data_FX_MODE_GUITARE);
    initDone = true;
  }

  void connected()
  {
  }

  void loop()
  {
    // if usermod is disabled or called during strip updating just exit
    // NOTE: on very long strips strip.isUpdating() may always return true so update accordingly
    if (!initDone || !enabled || strip.isUpdating())
      return;

    // currently nothing to do here
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
    if (!initDone || !enabled)
      return; // prevent crash on boot applyPreset()

    JsonObject usermod = root[FPSTR(_name)];
    if (usermod.isNull())
      usermod = root.createNestedObject(FPSTR(_name));

    usermod["button0"] = guitareButtons[0].virtualButtonState;
    usermod["button1"] = guitareButtons[1].virtualButtonState;
    usermod["button2"] = guitareButtons[2].virtualButtonState;
  }

  void readFromJsonState(JsonObject &root)
  {
    if (!initDone)
      return; // prevent crash on boot applyPreset()

    JsonObject usermod = root[FPSTR(_name)];
    if (!usermod.isNull())
    {
      if (!usermod["button0"].isNull())
      {
        HR_PRINTLN("Virtual Button 0");
        handleVirtualButton(usermod["button0"], &guitareButtons[0]);
      }
      if (!usermod["button1"].isNull())
      {
        HR_PRINTLN("Virtual Button 0");
        handleVirtualButton(usermod["button1"], &guitareButtons[1]);
      }
      if (!usermod["button2"].isNull())
      {
        HR_PRINTLN("Virtual Button 0");
        handleVirtualButton(usermod["button2"], &guitareButtons[2]);
      }
    }
  }

  void handleVirtualButton(uint8_t newState, GuitareButton *btn)
  {
    if (newState != btn->virtualButtonState)
    {
      if (newState)
      {
        HR_PRINTLN("Trigger On Note");
        triggerGuitareOnNote(millis(), btn);
      }
      else
      {
        HR_PRINTLN("Trigger Off Note");
        triggerGuitareOffNote(millis(), btn);
      }
      btn->virtualButtonState = newState;
    }
  }

  void addToConfig(JsonObject &root)
  {
    JsonObject top = root.createNestedObject(FPSTR(_name));
    top["enabled"] = enabled;
    top["button-raw-mode"] = buttonRawValue;
    top["guitare-enable"] = enableGuitareMode;
    top["guitare-corpus-leds"] = guitareCorpusLedCnt;
    char configKey[30];
    for (int i = 0; i < WLED_MAX_BUTTONS - 1; i++)
    {
      sprintf(configKey, "guitare-button-%d-id", i);
      top[configKey] = guitareButtons[i].wledButtonId;
      sprintf(configKey, "guitare-button-%d-duration", i);
      top[configKey] = guitareButtons[i].noteDuration;
      sprintf(configKey, "guitare-button-%d-attack", i);
      top[configKey] = guitareButtons[i].noteAttack;
      sprintf(configKey, "guitare-button-%d-decay", i);
      top[configKey] = guitareButtons[i].noteDecay;
      sprintf(configKey, "guitare-button-%d-body-hold", i);
      top[configKey] = guitareButtons[i].bodyHold;
      sprintf(configKey, "guitare-button-%d-hue1", i);
      top[configKey] = guitareButtons[i].hue[0];
      sprintf(configKey, "guitare-button-%d_hue2", i);
      top[configKey] = guitareButtons[i].hue[1];
      sprintf(configKey, "guitare-button-%d_hue3", i);
      top[configKey] = guitareButtons[i].hue[2];
    }
  }

  bool readFromConfig(JsonObject &root)
  {
    JsonObject top = root[FPSTR(_name)];

    bool configComplete = !top.isNull();

    configComplete &= getJsonValue(top["enabled"], enabled, true);
    configComplete &= getJsonValue(top["button-raw-mode"], buttonRawValue, false);
    configComplete &= getJsonValue(top["guitare-enable"], enableGuitareMode, false);
    configComplete &= getJsonValue(top["guitare-corpus-leds"], guitareCorpusLedCnt, 0);
    char configKey[31];
    for (int i = 0; i < WLED_MAX_BUTTONS - 1; i++)
    {
      guitareButtons[i].corpuseLeds = guitareCorpusLedCnt;
      sprintf(configKey, "guitare-button-%d-id", i);
      configComplete &= getJsonValue(top[configKey], guitareButtons[i].wledButtonId, 255);
      sprintf(configKey, "guitare-button-%d-duration", i);
      configComplete &= getJsonValue(top[configKey], guitareButtons[i].noteDuration, 2000);
      sprintf(configKey, "guitare-button-%d-attack", i);
      configComplete &= getJsonValue(top[configKey], guitareButtons[i].noteAttack, 200);
      sprintf(configKey, "guitare-button-%d-decay", i);
      configComplete &= getJsonValue(top[configKey], guitareButtons[i].noteDecay, 200);
      sprintf(configKey, "guitare-button-%d-body-hold", i);
      configComplete &= getJsonValue(top[configKey], guitareButtons[i].bodyHold, 1000);
      sprintf(configKey, "guitare-button-%d_hue1", i);
      configComplete &= getJsonValue(top[configKey], guitareButtons[i].hue[0], (i * 85 + 64) & 0xFF);
      sprintf(configKey, "guitare-button-%d_hue2", i);
      configComplete &= getJsonValue(top[configKey], guitareButtons[i].hue[1], (i * 85 + 128) & 0xFF);
      sprintf(configKey, "guitare-button-%d_hue3", i);
      configComplete &= getJsonValue(top[configKey], guitareButtons[i].hue[2], (i * 85 + 194) & 0xFF);
    }
    return configComplete;
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

      // register for virtual button topic
      // TBD
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
  void triggerGuitareOnNote(unsigned long now, GuitareButton *btn);
  void triggerGuitareOffNote(unsigned long now, GuitareButton *btn);
};

const char HummelRummelUsermod::_name[] PROGMEM = "HummelRummelUsermod";

// Copied from button implementation but it's actually independent
#define WLED_DEBOUNCE_THRESHOLD 50    // only consider button input of at least 50ms as valid (debouncing)
#define WLED_LONG_PRESS 600           // long press if button is released after held for at least 600ms
#define WLED_DOUBLE_PRESS 350         // double press if another press within 350ms after a short press
#define WLED_LONG_REPEATED_ACTION 300 // how often a repeated action (e.g. dimming) is fired on long press on button IDs >0

void HummelRummelUsermod::triggerGuitareOnNote(unsigned long now, GuitareButton *btn)
{
  if (btn->linkedNoteID != 255)
  {
    HR_PRINT(btn->linkedNoteID);
    HR_PRINTLN(" note already pressed");
    // this button already plays a note, so we are done here
    return;
  }
  for (int i = 1; i < MAX_GUITARE_NOTES; i++)
  {
    GuitareNote *note = &guitareNotes[i];
    // find a free note
    if (note->startTime == 0)
    {
      HR_PRINT(i);
      HR_PRINTLN(" note triggered");

      // set the start time, reset the release time, the notes hue value and link the button and the note
      note->startTime = now;
      note->releaseTime = 0;
      note->hue = btn->hue[btn->activeHueIndex];
      note->triggerButton = btn;
      btn->linkedNoteID = i;
      return;
    }
  }
}

void HummelRummelUsermod::triggerGuitareOffNote(unsigned long now, GuitareButton *btn)
{
  HR_PRINT(btn->linkedNoteID);
  HR_PRINTLN(" note linked");

  // check if button is in range, this also handles if a button triggers an off note without an on-note (linkedNoteID will be set to 255)
  if (btn->linkedNoteID < MAX_GUITARE_NOTES)
  {
    HR_PRINT(btn->linkedNoteID);
    HR_PRINTLN(" note released");
    // set the release time and reset the linkedNoteID
    guitareNotes[btn->linkedNoteID].releaseTime = now;
    btn->linkedNoteID = 255;
  }
}

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
  if (buttonRawValue || enableGuitareMode)
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

      if (buttonLastState[b])
      {
        HR_PRINTLN("BUTTON ON");
      }
      else
      {
        HR_PRINTLN("BUTTON OFF");
      }
      // apply the macro if one is defined for the
      if (macroButton[b])
        applyPreset(macroButton[b], CALL_MODE_BUTTON_PRESET);

      if (enableGuitareMode)
      {
        if (buttonLastState[b])
        {
          if (b == (WLED_MAX_BUTTONS - 1))
          {
            uint8_t newActiveHueIndex = guitareButtons[0].activeHueIndex + 1 < MAX_HUE ? guitareButtons[0].activeHueIndex + 1 : 0;
            // this is the state switch button
            for (int i = 0; i < WLED_MAX_BUTTONS - 1; i++)
            {
              guitareButtons[i].activeHueIndex = newActiveHueIndex;
            }
          }
          // tiggger notes if configured
          for (int i = 0; i < WLED_MAX_BUTTONS - 1; i++)
          {
            if (guitareButtons[i].wledButtonId == b)
            {
              HR_PRINTLN("Trigger On Note");
              triggerGuitareOnNote(now, &guitareButtons[i]);
            }
          }
        }
        else
        {
          // tiggger off notes if configured
          for (int i = 0; i < WLED_MAX_BUTTONS - 1; i++)
          {
            if (guitareButtons[i].wledButtonId == b)
            {
              HR_PRINTLN("Trigger Off Note");
              triggerGuitareOffNote(now, &guitareButtons[i]);
            }
          }
        }
      }
    }

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

  // momentary button logic, the classic slightly modified logic
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
