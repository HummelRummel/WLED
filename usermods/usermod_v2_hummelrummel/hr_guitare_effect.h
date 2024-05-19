#pragma once

#include "wled.h"

static const char _data_FX_MODE_GUITARE[] PROGMEM = "Guitare@;;;01";

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
    hueValueNg = hueValueNg + 64;
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
          HR_NOTE("play-note-ng: ", now);
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
              intensity = (now - (pTime + note->startTime)) * 256 / guitareNotesHack[j].triggerButton->noteAttack;
              HR_NOTE("attack: ", intensity);
            }
            else
            {
              intensity = 255;
              HR_NOTE("hold: ", intensity);
            }
          }
          else if (now <= note->releaseTime + pTime)
          {
            intensity = 255;
            HR_NOTE("still-hold: ", intensity);
          }
          else if (now < pTime + note->releaseTime + note->triggerButton->noteDecay)
          {
            if (note->startTime + note->triggerButton->noteAttack > note->releaseTime)
            {
              // the release came before the attack could finish so we need to intensity from att at the begin of the release into account
              unsigned long attackIntensity = (note->releaseTime - note->startTime) * 256 / guitareNotesHack[j].triggerButton->noteAttack;
              unsigned long decayIntensity = ((now - (pTime + note->releaseTime)) * 256 / note->triggerButton->noteDecay);
              intensity = decayIntensity < attackIntensity ? attackIntensity - decayIntensity : 0;
              HR_NOTE("pre-attack: ", attackIntensity);
              HR_NOTE("decay: ", decayIntensity);
            }
            else
            {
              // decay after the release
              intensity = 255 - ((now - (pTime + note->releaseTime)) * 256 / note->triggerButton->noteDecay);
              HR_NOTE("decay: ", intensity);
            }
          }
          else
          {
            if (i >= SEGMENT.length() - 1)
            {
              HR_PRINTLN("note freed");

              note->startTime = 0;
            }
            // we are already finished here with this note
            continue;
          }

          CRGB noteSprite = blend(CRGB::Black, CHSV(note->hueButton->hueValueNg + (note->triggerButton->wledButtonId * 85), 255, 255), intensity);
          overlayColor.r = overlayColor.r > noteSprite.r ? overlayColor.r : noteSprite.r;
          overlayColor.g = overlayColor.g > noteSprite.g ? overlayColor.g : noteSprite.g;
          overlayColor.b = overlayColor.b > noteSprite.b ? overlayColor.b : noteSprite.b;
        }
        else
        {
          // old mechanism

          pTime = pTime + guitareNotesHack[j].startTime;
          if (pTime >= now)
          {
            // it's not it's turn yet with this note
            continue;
          }
          else if (pTime + guitareNotesHack[j].triggerButton->noteAttack + guitareNotesHack[j].triggerButton->noteHold + guitareNotesHack[j].triggerButton->noteDecay < now)
          {
            if (i >= SEGMENT.length() - 1)
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
