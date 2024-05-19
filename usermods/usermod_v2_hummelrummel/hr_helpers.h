#pragma once

#ifdef DEBUG_HR
#define HR_PRINT(x) Serial.print(x)
#define HR_PRINTLN(x) Serial.println(x)
#define HR_NOTE(s, n) \
  Serial.print(s);    \
  Serial.println(n)
#else
#define HR_PRINT(x)
#define HR_PRINTLN(x)
#define HR_NOTE(s, n)
#endif

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
