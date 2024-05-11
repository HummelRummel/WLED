#pragma once

#include "wled.h"

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

#define HUMMELRUMMEL_USERMOD "HummelRummelUsermod"

uint32_t Color(byte r, byte g, byte b)
{
  uint32_t c;
  c = r;
  c <<= 8;
  c |= g;
  c <<= 8;
  c |= b;
  return c;
}
uint32_t Color(CRGB c)
{
  return Color(c[0], c[1], c[2]);
}

uint16_t fill_level(uint32_t bg, uint32_t fg)
{
  uint8_t fillingLevel = SEGMENT.speed;

  uint32 blendedFg = Color(blend(bg, fg, SEGMENT.intensity));
  if (fillingLevel == 0)
  {
    SEGMENT.fill(bg);
  }
  else if (fillingLevel == 100)
  {
    SEGMENT.fill(blendedFg);
  }
  else
  {
    for (int i = 0; i < SEGMENT.length(); i++)
    {
      if (i < fillingLevel)
      {
        SEGMENT.setPixelColor(i, bg);
      }
      else
      {
        SEGMENT.setPixelColor(i, blendedFg);
      }
    }
  }

  return FRAMETIME;
}
uint16_t mode_fill_level_rainbow(void)
{
  return fill_level(SEGCOLOR(0), SEGMENT.color_wheel(SEGENV.call & 0xFF));
}
uint16_t mode_fill_level_color(void)
{
  return fill_level(SEGCOLOR(0), SEGCOLOR(1));
}

static const char _data_FX_MODE_FILL_LEVEL_RAINBOW[] PROGMEM = "Fill Level Rainbow@Level,Intensity;Unfilled,Filled;!;01";
static const char _data_FX_MODE_FILL_LEVEL_COLOR[] PROGMEM = "Fill Level Color@Level,Intensity;Unfilled;!;01";

static const char _data_FX_MODE_GUITARE[] PROGMEM = "Guitare@;;;01";

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

// guitare extension

#ifndef MAX_GUITARE_NOTES
#define MAX_GUITARE_NOTES 10
#endif // MAX_GUITARE_NOTES

#define MIN_NOTE_INTERVAL_MS 200
#define MIN_HUE_UPDATE_INTERVAL 50
#define HUE_INCREMENT 1

struct GuitareButton
{
public:
  GuitareButton() : wledButtonId(255), linkedNoteID(255) {}

  uint8_t wledButtonId;
  unsigned long noteDuration;
  unsigned long noteAttack;
  unsigned long noteHold;
  unsigned long noteDecay;
  CRGB noteColor;
  bool hsvColor;
  uint8_t hueValueNg;
  uint8_t linkedNoteID;
  unsigned long lastTrigger;

  void incrementHue(unsigned long now)
  {
    if (now > lastTrigger + MIN_HUE_UPDATE_INTERVAL)
    {
      hueValueNg = hueValueNg + HUE_INCREMENT;
      lastTrigger = now;
    }
  }
};

struct GuitareNote
{
public:
  unsigned long startTime;
  unsigned long releaseTime;
  GuitareButton *hueButton;
  GuitareButton *triggerButton;
};

GuitareNote *guitareNotesHack;

uint16_t mode_guitare(void)
{
  if (!guitareNotesHack)
    return FRAMETIME;

  uint32_t now = millis();
  for (int i = 0; i < SEGMENT.length(); i++)
  {
    CRGB overlayColor = CRGB::Black;
    uint8_t hueValue = 0;
    uint8_t valValue = 0;
    bool hsvActive;
    for (int j = 0; j < MAX_GUITARE_NOTES; j++)
    {
      GuitareNote *note = &guitareNotesHack[j];
      if (note->startTime != 0)
      {
        unsigned long pTime = (note->triggerButton->noteDuration * i) / SEGMENT.length();
        unsigned long intensity;
        if (note->hueButton)
        {
          // Serial.print("play-note-ng: ");
          // Serial.println(now);
          // ng
          if (now <= pTime + note->startTime)
          {
            // it's not it's turn yet with this note
            continue;
          }
          else if (!note->releaseTime)
          {
            if (now <= pTime + note->startTime + note->triggerButton->noteAttack)
            {
              // attack
              intensity = (now - (pTime = note->startTime)) * 256 / guitareNotesHack[j].triggerButton->noteAttack;
            }
            else
            {
              // hold
              intensity = 255;
            }
          }
          else if (now <= pTime + note->releaseTime + note->triggerButton->noteDecay)
          {
            if (note->startTime + note->triggerButton->noteAttack > note->releaseTime)
            {
              // the release came before the attack could finish so we need to intensity from att at the begin of the release into account
              unsigned long attackIntensity = (note->releaseTime - note->startTime) * 256 / guitareNotesHack[j].triggerButton->noteAttack;
              unsigned long decayIntensity = ((now - (pTime + note->releaseTime)) * 256 / note->triggerButton->noteDecay);
              intensity = decayIntensity < attackIntensity ? attackIntensity - decayIntensity : 0;
              // Serial.print("pre-atk-rls ");
              // Serial.print(attackIntensity);
              // Serial.print("-");
              // Serial.println(decayIntensity);
            }
            else
            {
              // decay after the release
              intensity = 255 - ((now - (pTime + note->releaseTime)) * 256 / note->triggerButton->noteDecay);
              // Serial.print("nrml-rls ");
              // Serial.println(intensity);
            }
          }
          else
          {
            if (i >= SEGMENT.length() - 1)
            {
              // we are done with the note, free it up
              note->startTime = 0;
            }
            // we are already finished here with this note
            continue;
          }

          CRGB noteSprite = blend(CRGB::Black, CHSV(note->hueButton->hueValueNg, 255, 255), intensity);
          overlayColor.r = overlayColor.r > noteSprite.r ? overlayColor.r : noteSprite.r;
          overlayColor.g = overlayColor.g > noteSprite.g ? overlayColor.g : noteSprite.g;
          overlayColor.b = overlayColor.b > noteSprite.b ? overlayColor.b : noteSprite.b;
        }
        else
        {
          // old mechanism
          // Serial.println("play-note-old");

          pTime = pTime + guitareNotesHack[j].startTime;
          if (pTime >= now)
          {
            // it's not it's turn yet with this note
            continue;
          }
          else if (pTime + guitareNotesHack[j].triggerButton->noteAttack + guitareNotesHack[j].triggerButton->noteHold + guitareNotesHack[j].triggerButton->noteDecay < now)
          {
            if (i >= SEGMENT.length())
            {
              guitareNotesHack[j].startTime = 0;
            }
            // we are already finished here with this note
            continue;
          }
          else if (pTime + guitareNotesHack[j].triggerButton->noteAttack >= now)
          {
            // attack
            intensity = (now - pTime) * 256 / guitareNotesHack[j].triggerButton->noteAttack;
          }
          else if (pTime + guitareNotesHack[j].triggerButton->noteAttack + guitareNotesHack[j].triggerButton->noteHold >= now)
          {
            intensity = 255;
          }
          else if (pTime + guitareNotesHack[j].triggerButton->noteAttack + guitareNotesHack[j].triggerButton->noteHold + guitareNotesHack[j].triggerButton->noteDecay >= now)
          {
            // decay
            intensity = 255 - ((now - (pTime + guitareNotesHack[j].triggerButton->noteAttack + guitareNotesHack[j].triggerButton->noteHold)) * 256 / guitareNotesHack[j].triggerButton->noteDecay);
          }
          if (guitareNotesHack[j].triggerButton->hsvColor)
          {
            hueValue = hueValue + blend8(0, guitareNotesHack[j].triggerButton->noteColor.r, intensity);
            valValue = valValue > intensity ? valValue : intensity;
            hsvActive = true;
          }
          else
          {
            CRGB noteSprite = blend(CRGB::Black, guitareNotesHack[j].triggerButton->noteColor, intensity);
            overlayColor.r = overlayColor.r > noteSprite.r ? overlayColor.r : noteSprite.r;
            overlayColor.g = overlayColor.g > noteSprite.g ? overlayColor.g : noteSprite.g;
            overlayColor.b = overlayColor.b > noteSprite.b ? overlayColor.b : noteSprite.b;
          }
        }
      }
    }
    if (hsvActive)
    {
      CHSV hsvOverlay = {hueValue, 255, valValue};
      CRGB rgbHsvOverlay = hsvOverlay;
      overlayColor.r = overlayColor.r > rgbHsvOverlay.r ? overlayColor.r : rgbHsvOverlay.r;
      overlayColor.g = overlayColor.g > rgbHsvOverlay.g ? overlayColor.g : rgbHsvOverlay.g;
      overlayColor.b = overlayColor.b > rgbHsvOverlay.b ? overlayColor.b : rgbHsvOverlay.b;
    }

    SEGMENT.setPixelColor(i, overlayColor);
  }

  return FRAMETIME;
}

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

  // guitare effect
  // presistent config
  bool guitareMode;
  bool guitareModeNg;
  uint8_t guitareCorpusLedCnt;
  GuitareButton guitareButtons[WLED_MAX_BUTTONS];

  // DEBUG
  DisplayType type = FLD_TYPE; // display type
  // DEBUG

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
    if (!enabled || strip.isUpdating())
      return;

    // do your magic here
    if (millis() - lastTime > 5000)
    {
      lastTime = millis();
      // Serial.println("AOM");
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
    top["btn-raw-value"] = buttonRawValue;

    top["gtr-mode"] = guitareMode;
    top["gtr-mode-ng"] = guitareModeNg;
    top["gtr-corpus-led-cnt"] = guitareCorpusLedCnt;
    char configKey[30];
    for (int i = 0; i < WLED_MAX_BUTTONS; i++)
    {
      sprintf(configKey, "gtr_btn_%d_id", i);
      top[configKey] = guitareButtons[i].wledButtonId;
      sprintf(configKey, "gtr_btn_%d_duration", i);
      top[configKey] = guitareButtons[i].noteDuration;
      sprintf(configKey, "gtr_btn_%d_attack", i);
      top[configKey] = guitareButtons[i].noteAttack;
      sprintf(configKey, "gtr_btn_%d_hold", i);
      top[configKey] = guitareButtons[i].noteHold;
      sprintf(configKey, "gtr_btn_%d_decay", i);
      top[configKey] = guitareButtons[i].noteDecay;
      sprintf(configKey, "gtr_btn_%d_color_r", i);
      top[configKey] = guitareButtons[i].noteColor[0];
      sprintf(configKey, "gtr_btn_%d_color_g", i);
      top[configKey] = guitareButtons[i].noteColor[1];
      sprintf(configKey, "gtr_btn_%d_color_b", i);
      top[configKey] = guitareButtons[i].noteColor[2];
      sprintf(configKey, "gtr_btn_%d_hsv_color", i);
      top[configKey] = guitareButtons[i].hsvColor;
    }
  }

  bool readFromConfig(JsonObject &root)
  {
    JsonObject top = root[HUMMELRUMMEL_USERMOD];

    bool configComplete = !top.isNull();

    configComplete &= getJsonValue(top["enabled"], enabled, true);
    configComplete &= getJsonValue(top["btn-raw-value"], buttonRawValue, false);
    configComplete &= getJsonValue(top["gtr-mode"], guitareMode, false);
    configComplete &= getJsonValue(top["gtr-mode-ng"], guitareModeNg, false);
    configComplete &= getJsonValue(top["gtr-corpus-led-cnt"], guitareCorpusLedCnt, 20);
    char configKey[30];
    for (int i = 0; i < WLED_MAX_BUTTONS; i++)
    {
      sprintf(configKey, "gtr_btn_%d_id", i);
      configComplete &= getJsonValue(top[configKey], guitareButtons[i].wledButtonId, 255);
      sprintf(configKey, "gtr_btn_%d_duration", i);
      configComplete &= getJsonValue(top[configKey], guitareButtons[i].noteDuration, 1000);
      sprintf(configKey, "gtr_btn_%d_attack", i);
      configComplete &= getJsonValue(top[configKey], guitareButtons[i].noteAttack, 50);
      sprintf(configKey, "gtr_btn_%d_hold", i);
      configComplete &= getJsonValue(top[configKey], guitareButtons[i].noteHold, 200);
      sprintf(configKey, "gtr_btn_%d_decay", i);
      configComplete &= getJsonValue(top[configKey], guitareButtons[i].noteDecay, 400);
      sprintf(configKey, "gtr_btn_%d_color_r", i);
      configComplete &= getJsonValue(top[configKey], guitareButtons[i].noteColor[0], i == 0 ? 255 : 0);
      sprintf(configKey, "gtr_btn_%d_color_g", i);
      configComplete &= getJsonValue(top[configKey], guitareButtons[i].noteColor[1], i == 1 ? 255 : 0);
      sprintf(configKey, "gtr_btn_%d_color_b", i);
      configComplete &= getJsonValue(top[configKey], guitareButtons[i].noteColor[2], i == 2 ? 255 : 0);
      sprintf(configKey, "gtr_btn_%d_hsv_color", i);
      configComplete &= getJsonValue(top[configKey], guitareButtons[i].hsvColor, false);
    }
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
  uint8_t triggerGuitareOnNote(unsigned long now, GuitareButton *btn, GuitareButton *hueBtn);
  void triggerGuitareOffNote(unsigned long now, uint8_t noteID);
};

// Copied from button implementation but it's actually independent
#define WLED_DEBOUNCE_THRESHOLD 50    // only consider button input of at least 50ms as valid (debouncing)
#define WLED_LONG_PRESS 600           // long press if button is released after held for at least 600ms
#define WLED_DOUBLE_PRESS 350         // double press if another press within 350ms after a short press
#define WLED_LONG_REPEATED_ACTION 300 // how often a repeated action (e.g. dimming) is fired on long press on button IDs >0

uint8_t HummelRummelUsermod::triggerGuitareOnNote(unsigned long now, GuitareButton *btn, GuitareButton *hueBtn)
{
  for (int i = 0; i < MAX_GUITARE_NOTES; i++)
  {
    if (guitareNotes[i].startTime == 0)
    {
      guitareNotes[i].startTime = now;
      guitareNotes[i].releaseTime = 0;
      guitareNotes[i].triggerButton = btn;
      guitareNotes[i].hueButton = hueBtn;

      btn->lastTrigger = now;
      return i;
    }
  }
  return 255;
}

void HummelRummelUsermod::triggerGuitareOffNote(unsigned long now, uint8_t noteID)
{
  if (noteID < MAX_GUITARE_NOTES)
  {
    guitareNotes[noteID].releaseTime = now;
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
  if (buttonRawValue || guitareMode)
  {
    if (guitareModeNg)
    {
      if ((b == 0) || (b == 3))
      {
        if (isButtonPressed(b))
        {
          // Serial.println("updating hue");
          // these two buttons control hue in the NG mode, the incrementHue ratelimits so no need to do anything here
          guitareButtons[b].incrementHue(now);
        }
        return true;
      }
    }
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

      // buttonLastState[b] ? Serial.println("BUTTON ON") : Serial.println("BUTTON OFF");
      // apply the macro if one is defined for the
      if (macroButton[b])
        applyPreset(macroButton[b], CALL_MODE_BUTTON_PRESET);

      if (guitareMode)
      {
        if (isButtonPressed(b))
        {
          // tiggger notes if configured
          for (int i = 0; i < WLED_MAX_BUTTONS; i++)
          {
            if (guitareButtons[i].wledButtonId == b)
            {
              if (guitareModeNg)
              {
                if ((b != 0) || (b != 3))
                {
                  // Serial.println("Trigger On Note");
                  guitareButtons[i].linkedNoteID = triggerGuitareOnNote(now, &guitareButtons[i], b == 1 ? &guitareButtons[0] : b == 2 ? &guitareButtons[3]
                                                                                                                                      : 0);
                }
              }
              else
              {
                // Serial.println("Trigger On Note");
                guitareButtons[i].linkedNoteID = triggerGuitareOnNote(now, &guitareButtons[i], 0);
              }
            }
          }
        }
        else
        {
          if (guitareModeNg)
          {
            // tiggger off notes if configured
            for (int i = 0; i < WLED_MAX_BUTTONS; i++)
            {
              if (guitareButtons[i].wledButtonId == b)
              {
                if ((b != 0) || (b != 3))
                {
                  // Serial.println("Trigger Off Note");

                  // we are only interested in the pressed event
                  triggerGuitareOffNote(now, guitareButtons[i].linkedNoteID);
                  guitareButtons[i].linkedNoteID = 255;
                }
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
