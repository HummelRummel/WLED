#pragma once

#include "wled.h"
#include "hr_helpers.h"

static const char _data_FX_MODE_FILL_LEVEL_RAINBOW[] PROGMEM = "Fill Level Rainbow@Level,Intensity;Unfilled,Filled;!;01";
static const char _data_FX_MODE_FILL_LEVEL_COLOR[] PROGMEM = "Fill Level Color@Level,Intensity;Unfilled;!;01";

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
