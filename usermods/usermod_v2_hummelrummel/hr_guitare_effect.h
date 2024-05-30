#pragma once

#include "wled.h"

static const char _data_FX_MODE_GUITARE[] PROGMEM = "Guitare@;;;01";

// guitare extension
#define MAX_GUITARE_NOTES 10
#define MIN_NOTE_INTERVAL_MS 200
#define MAX_HUE 3

struct GuitareButton
{
public:
  GuitareButton() : wledButtonId(255), linkedNoteID(255) {}

  uint8_t wledButtonId;
  unsigned long noteDuration;
  unsigned long noteAttack;
  unsigned long noteDecay;
  uint8_t linkedNoteID;
  uint8_t activeHueIndex;
  uint8_t hue[MAX_HUE];
  unsigned long lastTrigger;
};

struct GuitareNote
{
public:
  unsigned long startTime;
  unsigned long releaseTime;
  uint8_t hue;
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
    for (int j = 0; j < MAX_GUITARE_NOTES; j++)
    {
      GuitareNote *note = &guitareNotesHack[j];
      if (note->startTime != 0)
      {
        unsigned long pTime = (note->triggerButton->noteDuration * i) / SEGMENT.length();
        unsigned long intensity;
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

        CRGB noteSprite = blend(CRGB::Black, CHSV(note->hue, 255, 255), intensity);
        overlayColor.r = overlayColor.r > noteSprite.r ? overlayColor.r : noteSprite.r;
        overlayColor.g = overlayColor.g > noteSprite.g ? overlayColor.g : noteSprite.g;
        overlayColor.b = overlayColor.b > noteSprite.b ? overlayColor.b : noteSprite.b;
      }
    }

    SEGMENT.setPixelColor(i, overlayColor);
  }

  return FRAMETIME;
}
