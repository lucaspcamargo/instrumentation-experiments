#ifndef CAMARGO_COLOR_H
#define CAMARGO_COLOR_H

#include "common.h"

// ported from http://www.geekymonkey.com/Programming/CSharp/RGB2HSL_HSL2RGB.htm

typedef struct cColor_struct {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} cColor;


void cColorHSL2RGB(float h, float sl, float l, cColor * ret)
{
  float v;
  float r,g,b;

  r = l;   // default to gray
  g = l;
  b = l;
  v = (l <= 0.5) ? (l * (1.0 + sl)) : (l + sl - l * sl);
  if (v > 0)
  {
    float m;
    float sv;
    int sextant;
    float fract, vsf, mid1, mid2;

    m = l + l - v;
    sv = (v - m ) / v;
    h *= 6.0;
    sextant = (int)h;
    fract = h - sextant;
    vsf = v * sv * fract;
    mid1 = m + vsf;
    mid2 = v - vsf;
    
    switch (sextant)
    {
      case 0:
	r = v;
	g = mid1;
	b = m;
	break;
      case 1:
	r = mid2;
	g = v;
	b = m;
	break;
      case 2:
	r = m;
	g = v;
	b = mid1;
	break;
      case 3:
	r = m;
	g = mid2;
	b = v;
	break;
      case 4:
	r = mid1;
	g = m;
	b = v;
	break;
      case 5:
	r = v;
	g = m;
	b = mid2;
	break;
    }
  }
  
  (*ret).r = (uint8_t)(r * 255.0f);
  (*ret).g = (uint8_t)(g * 255.0f);
  (*ret).b = (uint8_t)(b * 255.0f);
  
}

#endif