#pragma once

#include "wled.h"

uint16_t flow_fan(uint32_t color1, uint32_t color2)
{

  uint32_t now = millis();
  uint32_t cycleTime = (255 - SEGMENT.speed) * 20;
  uint32_t onTime = FRAMETIME;
  cycleTime += FRAMETIME * 2;
  uint32_t it = now / cycleTime;
  uint32_t rem = now % cycleTime;

  bool on = false;
  if (it != SEGENV.step // new iteration, force on state for one frame, even if set time is too brief
      || rem <= onTime)
  {
    on = true;
  }

  SEGENV.step = it; // save previous iteration

  uint32_t color = on ? color1 : color2;
  SEGMENT.fill(color);

  return FRAMETIME;
}

/*
 * Flow Rainbow Fan effect. Cycling through the rainbow in a fan strobe.
 */
uint16_t mode_flow_rainbow_fan(void)
{
  return flow_fan(SEGMENT.color_wheel(SEGENV.call & 0xFF), SEGMENT.color_wheel((SEGENV.call + SEGMENT.intensity) & 0xFF));
}

/*
 * Flow Color Fan effect. Uses color1 and color2 for the fan strobe.
 */
uint16_t mode_flow_color_fan(void)
{
  return flow_fan(SEGCOLOR(0), SEGCOLOR(1));
}

uint16_t flow_fade(uint32_t color1, uint32_t color2)
{
  uint32_t now = millis();
  uint32_t blinkCycle = (255 - SEGMENT.speed) * 20;
  uint32_t onTime = FRAMETIME;
  // one cycle consists of
  blinkCycle += FRAMETIME * 2;
  uint32_t episodeLength = FRAMETIME * 2 * SEGLEN;
  uint32_t effectLength = episodeLength * 4;
  uint32_t it = now / blinkCycle;
  uint32_t rem = now % blinkCycle;
  uint32_t currentLedIndex = (now % episodeLength) / (FRAMETIME * 2);
  uint32_t currentEpisode = (now % effectLength) / episodeLength;
  bool oddEpisode = currentEpisode % 2 == 0;
  bool firstPart = currentEpisode < 2;

  bool on = false;
  if (it != SEGENV.step // new iteration, force on state for one frame, even if set time is too brief
      || rem <= onTime)
  {
    on = true;
  }

  SEGENV.step = it; // save previous iteration

  uint32_t colorNew = firstPart ? color1 : color2;
  uint32_t colorOld = firstPart ? color2 : color1;
  if (on == true)
  {
    if (oddEpisode)
    {
      SEGMENT.fill(colorOld);
    }
    else
    {
      SEGMENT.fill(colorNew);
    }
  }
  else
  {
    if (firstPart)
    {
      for (uint16_t i = 0; i < SEGLEN; i++)
      {
        if (i <= currentLedIndex)
        {
          SEGMENT.setPixelColor(i, colorNew);
        }
        else
        {
          SEGMENT.setPixelColor(i, colorOld);
        }
      }
    }
    else
    {
      for (uint16_t i = 0; i < SEGLEN; i++)
      {
        if (i <= currentLedIndex)
        {
          SEGMENT.setPixelColor(SEGLEN - 1 - i, colorNew);
        }
        else
        {
          SEGMENT.setPixelColor(SEGLEN - 1 - i, colorOld);
        }
      }
    }
  }

  return FRAMETIME;
}

/*
 * Flow Rainbow Fade effect. Cycling through the rainbow in a fan strobe.
 */
uint16_t mode_flow_rainbow_fade(void)
{
  return flow_fade(SEGMENT.color_wheel(SEGENV.call & 0xFF), SEGMENT.color_wheel((SEGENV.call + SEGMENT.intensity) & 0xFF));
}

/*
 * Flow Color Fade effect. Uses color1 and color2 for the fan strobe.
 */
uint16_t mode_flow_color_fade(void)
{
  return flow_fade(SEGCOLOR(0), SEGCOLOR(1));
}

// Metadata is probably not correct yet
static const char _data_FX_MODE_FLOW_RAINBOW_FAN[] PROGMEM = "FlowToy Rainbow Fan@!,!;;!;01";
static const char _data_FX_MODE_FLOW_COLOR_FAN[] PROGMEM = "FlowToy Color Fan@!,!;!,!;!;01";
static const char _data_FX_MODE_FLOW_RAINBOW_FADE[] PROGMEM = "FlowToy Rainbow Fade@!,!;;!;01";
static const char _data_FX_MODE_FLOW_COLOR_FADE[] PROGMEM = "FlowToy Color Fade@!,!;!,!;!;01";

class FlowEffectsUsermod : public Usermod
{
public:
  void setup()
  {
    strip.addEffect(255, &mode_flow_rainbow_fan, _data_FX_MODE_FLOW_RAINBOW_FAN);
    strip.addEffect(255, &mode_flow_color_fan, _data_FX_MODE_FLOW_COLOR_FAN);
    strip.addEffect(255, &mode_flow_rainbow_fade, _data_FX_MODE_FLOW_RAINBOW_FADE);
    strip.addEffect(255, &mode_flow_color_fade, _data_FX_MODE_FLOW_COLOR_FADE);
  }

  void loop()
  {
  }

  uint16_t getId()
  {
    return USERMOD_ID_FLOW_EFFECTS;
  }
};
