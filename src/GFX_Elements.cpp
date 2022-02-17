

#include "GFX_Elements.h"
#include "config.h"
//#include <Adafruit_GFX.h>    // Core graphics library
#include <PString.h>

//Adafruit_GFX* _gfx;  // = &tft;
//static Adafruit_GFX* GFX_ELEM::_gfx;   // = &tft;

//extern TFT_eSPI *tft;
//TFT_eSPI* _gfx = tft;
//#define _gfx tft

//M5Display& lcd;
//M5Touch& touch = M5.Touch;



char cBuff[20];
PString strVal(cBuff, sizeof(cBuff));
//String strVal;
//String str;


uint16_t  clrBk = 0;
uint16_t  clrBkBar = 0;


// GFX_ELEM  -------------------------------------


void GFX_ELEM::InitLast() {   // Set as marker
  GFX = GFX_END;
}

void GFX_ELEM::Init() {   // doesn't effect var if GFX_TEXT
  if (GFX & GFX_FLOAT)
    var.fValPrev = -99.99;
  else
    var.iValPrev = -9999;

  if (GFX & GFX_BAR) {
    if (GFX & GFX_VERT)
      var.iSizePrev = h;
    else
      var.iSizePrev = w;
// iSize = fVal * pxPerUnit;	
    if (GFX & GFX_SIGNED) {
      var.iSizePrev = var.iSizePrev >> 1;		// set prev pixel size to positive full scale
      *(float*)param = -var.iSizePrev / pxPerUnit * 2;	// set value to minus full scale (2* to be sure) then zero.  Use two calls to UpdateBar
      UpdateBar();
    }
    *(float*)param = 0;
    UpdateBar();
  }
  else if (GFX & GFX_VALUE)
    var.iSizePrev = 0;

  if (GFX & GFX_BUTTON) {
    elemVisResponse = TURN_OFF;
    elemVisState = 0;
    UpdateButton();
  }

  if (GFX & GFX_TEXT)
    UpdateText();
}

void GFX_ELEM::Init(GFX_ELEM_t gfx_, uint16_t clrFg, uint16_t clrBk, uint16_t x_, uint16_t y_, uint16_t w_, uint16_t h_, uint16_t r_) {
  x = x_;
  y = y_;
  w = w_;
  h = h_;
  rad = r_;
  clr = clrFg;
  clrTouched = clrBk;
  GFX = gfx_;
  format = GFX_FONT1 + GFX_DP0 + GFX_RJUSTIFY;   // = 0
  Init();
}


bool GFX_ELEM::UpdateGfx() {
  CheckPress();
  if (GFX & GFX_BAR)
  	UpdateBar();
  if (GFX & GFX_BUTTON)
    UpdateButton();
  return 1;
}
  
void GFX_ELEM::Update() {     // do CheckPress() first - if button type
  if (GFX & GFX_BUTTON)
    CheckPress();

  if (GFX & GFX_BAR)
    UpdateBar();
  else if (GFX & GFX_VALUE)
    UpdateValue();

  if (GFX & GFX_BUTTON)
    UpdateButton();
  if (GFX & GFX_TEXT)
    UpdateText();
}

void GFX_ELEM::UpdateButton() {
  if (elemVisResponse == TURN_ON) {
    elemVisState += 1;
    if (elemVisState >= 255) {
      elemVisState = 255;
      elemVisResponse = NONE;
    }
    lcd.fillRoundRect(x, y, w, h, rad, clrTouched);
    lcd.drawRoundRect(x, y, w, h, rad, clr);
  }
  if (elemVisResponse == TURN_OFF) {
    elemVisState -= 1;
    if (elemVisState <= 0) {
      elemVisState = 0;
      elemVisResponse = NONE;
    }
    lcd.fillRoundRect(x, y, w, h, rad, TFT_BLACK);
    lcd.drawRoundRect(x, y, w, h, rad, clr);
  }

}

int GFX_ELEM::CheckPress() {
#ifndef TOUCH
  return 0; }
#else

//  lv_area_t padScan;
//  lv_coord_t padRad = rad;
//  lv_area_set_bysize(&padScan, x, y, w, h);
  touchedElemPrev = touchedElem;

  touchedScreenPrev = touchedScreen;   // should be members of a single 'touch' monitoring class
  touchedScreen = touch.points > 0;   // 0, 1 or 2 for multifinger
  if (touchedScreen) {
   // Zone
    Point pnt;
    pnt = touch.point[0];
  //  pnt.y = touch.point[0].y;
    touchedElem = contains(pnt);
  } else
    touchedElem = 0;


  if (touchedElem && !touchedScreenPrev) {    // first touch on elem
    elemVisResponse = TURN_ON;
  }
  if (!touchedScreen && touchedElemPrev) {     // released elem this scan
    elemVisResponse = TURN_OFF;
    if (FuncClk != nullptr)
      FuncClk(this);
  }
  if (touchedElem && !touchedElemPrev && touchedScreenPrev) {    // dragged onto elem
  }
  if (!touchedElem && touchedElemPrev && touchedScreen) {    // dragged off elem
    elemVisResponse = TURN_OFF;
  }

  return touchedElem;
}
#endif




//  These are defined as member functions from TFT_ESPI_VERSION "2.2.8"
/***************************************************************************************
** Function name:           color16to24
** Description:             convert 16 bit colour to a 24 bit 888 colour value
***************************************************************************************/
uint32_t color16to24(uint16_t color565) {
  uint8_t r = (color565 >> 8) & 0xF8; r |= (r >> 5);
  uint8_t g = (color565 >> 3) & 0xFC; g |= (g >> 6);
  uint8_t b = (color565 << 3) & 0xF8; b |= (b >> 5);
  return ((uint32_t)r << 16) | ((uint32_t)g << 8) | ((uint32_t)b << 0);
}

/***************************************************************************************
** Function name:           color24to16
** Description:             convert 24 bit colour to a 16 bit 565 colour value
***************************************************************************************/
uint32_t color24to16(uint32_t color888) {
  uint16_t r = (color888 >> 8) & 0xF800;
  uint16_t g = (color888 >> 5) & 0x07E0;
  uint16_t b = (color888 >> 3) & 0x001F;
  return (r | g | b);
}



uint32_t color565to24(uint16_t clr565) {
#ifdef TFT_ESPI_VERSION
  return color16to24(clr565);      // is in #define TFT_ESPI_VERSION "2.2.8" - Documents\Arduino\Libraries\TFT_eSPI
#endif

  union {
    uint8_t r, g, b;
    uint32_t clr24;
  };
//   ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
  r = (clr565 >> 8) & 0xF8;
  g = (clr565 >> 3) & 0xFC;
  b = (clr565 << 3) & 0xF8;
  return clr24;
}

uint16_t color24to565(uint32_t clr24) {
  return color24to16(clr24);

  uint16_t clr565;
  union {
    uint8_t r, g, b;
    uint32_t clr24;
  };
  clr565 = lcd.color565(r, g, b);
//   ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
  return clr565;
}

uint32_t color24scale(uint32_t clr24, uint8_t scale) {    // scale by scale/256
  union {
    uint8_t r, g, b;
    uint32_t clr24val;
  };
  clr24val = clr24;
  uint16_t rs, gs, bs;
  rs = r * scale;
  r = rs >> 8;
  gs = g * scale;
  g = gs >> 8;
  bs = b * scale;
  b = bs >> 8;
  return clr24val;
}

void GFX_ELEM::UpdateBar() {
	if (GFX & GFX_SIGNED) {
		UpdateBarSigned();
		return;
	}
  float fVal = *(float*)param;
  float fPixels = fVal * pxPerUnit;
//  int iSize = fPixels + 0.5;   // if no fractional line
  int iSize = fPixels;   // if using fractional line, get full color size
  float fPartial = fPixels - iSize;      // decimal part 0 <= x < 1

  // color partialLine = fPartial*clr + (1-fPartial)*clrBkBar

  uint16_t partBar16 = fPartial * 256.0 + 0.5;
  uint16_t clrRes;

  if (partBar16 >= 256)
    clrRes = clr;
  else if (partBar16 = 0)
    clrRes = clrBkBar;
  else {
    uint32_t clr24 = color565to24(clr);
    uint32_t clrBk24 = color565to24(clrBkBar);
    union {
      uint32_t clrRes24;
      uint8_t rgbRes[4];
    };
    uint8_t rgbBar[4];
    uint8_t rgbBkg[4];

    uint8_t partBar = partBar16;        // 1-255
    uint8_t partBkg = 256 - partBar;    // 255-1

    uint16_t rgbRes16[3];
    for (int i; i < 3; i++) {
      rgbRes16[i] = rgbBar[i] * partBar + rgbBkg[i] * partBkg;
      rgbRes[i] = rgbRes16[i] >> 8;
    }
    clrRes = color24to565(clrRes24);
  }


  // draw line @ clrRes



	int iSizePrev = var.iSizePrev;
	int iMax = (GFX & GFX_VERT) ? h : w;
  //Serial.printf("UpdateBar, val: %f\n", fVal);

	if (iSize < 0)
	  iSize = 0;
	else if (iSize > iMax)
	  iSize = iMax;
    int diff = iSize - iSizePrev;
    if (diff == 0)
      return;
  
	if (GFX & GFX_VERT) {
	  if (diff > 0)
		lcd.fillRect(x, y + iSizePrev, w,  diff, clr);
	  else 
		lcd.fillRect(x, y + iSize,     w, -diff, clrBkBar);
	} else {		// Horz
	  if (diff > 0)
		lcd.fillRect(x + iSizePrev, y,  diff, h, clr);
	  else 
		lcd.fillRect(x + iSize,     y, -diff, h, clrBkBar);
	}
	var.iSizePrev = iSize;
}

void GFX_ELEM::UpdateBarSigned() {
    float fVal = *(float*)param;
//    int iSize = (int)(fVal * pxPerUnit + 1000.5) - 1000;		// iSize is signed pixel size from centre [-w/2, w/2]
    int iSize = round(fVal * pxPerUnit);		// iSize is signed pixel size from centre [-w/2, w/2]
//    int iSize = fVal * pxPerUnit;		// iSize is signed pixel size from centre [-w/2, w/2]
	int iSizePrev = var.iSizePrev;
	int iMax = (GFX & GFX_VERT) ? h : w;
	int iHalf = iMax >> 1;
	int pxMid = ((GFX & GFX_VERT) ? y : x) + iHalf;

//	iSize += iHalf;
	if (iSize < -iHalf)
	  iSize = -iHalf;
	else if (iSize > iHalf)
	  iSize = iHalf;
    int diff = iSize - iSizePrev;
    if (diff == 0)
      return;

/*  
check for gray out and for color
except instead of 0 for midway, use iMax/2
if (iSizePrev < 0 && diff > 0) do iSizePrev to min[iSize|0] gray.
if (iSizePrev > 0 && diff < 0) do max[iSize|0] to iSizePrev gray.
if (iSize > 0 && diff > 0) do max[iSizePrev|0] to iSize color
if (iSize < 0 && diff < 0) do iSize to min[iSizePrev|0] color
*/
	if (GFX & GFX_VERT) {
	  if (diff > 0) {		// *****vertical not finished*******
		if (iSizePrev < 0)
		  lcd.fillRect(x, y + iSizePrev, w,  min(diff, (iMax>>1) - iSizePrev), clrBkBar);
		if (iSize > 0)
		  lcd.fillRect(x, y + iSizePrev, w,  diff, clr);
	  } else {	// diff < 0
		if (iSizePrev > 0)
		  lcd.fillRect(x, y + iSize,     w, -diff, clrBkBar);
		if (iSize < 0)
		  lcd.fillRect(x, y + iSize,     w, -diff, clr);
	  }
	
	} else {		// Horz
	//  int start, width;
	  
	  if (diff > 0) {
		if (iSizePrev < 0)
		{  lcd.fillRect(pxMid + iSizePrev,         y, min(diff, -iSizePrev), h, clrBkBar);
//		  Serial.print("Diff >0, SizePrev <0:  ");
//		  start = iSizePrev;  width = min(diff, -iSizePrev);
//		  Serial.print(start);  Serial.print("  ");  Serial.println(width);
		}
		if (iSize > 0)
		{  lcd.fillRect(pxMid + max(iSizePrev, 0), y, min(diff, iSize),      h, clr);
//		  Serial.print("Diff >0, Size >0:  ");
//		  start = max(iSizePrev, 0);  width = min(diff, iSize);
//		  Serial.print(start);  Serial.print("  ");  Serial.println(width);
		}
	  } else {	// diff < 0
		if (iSizePrev > 0)
		{  lcd.fillRect(pxMid + max(iSize, 0),     y, min(-diff, iSizePrev), h, clrBkBar);
//		  Serial.print("Diff <0, SizePrev >0:  ");
//		  start = max(iSize, 0);  width = min(-diff, iSizePrev);
//		  Serial.print(start);  Serial.print("  ");  Serial.println(width);
		}
		if (iSize < 0)
		{  lcd.fillRect(pxMid + iSize,             y, min(-diff, -iSize), h, clr);
//		  Serial.print("Diff <0, Size <0:  ");
//		  start = iSize;  width = min(-diff, -iSize);
//		  Serial.print(start);  Serial.print("  ");  Serial.println(width);
		}
	  }
	}

  var.iSizePrev = iSize;
}

  
void GFX_ELEM::UpdateValue() {
    strVal.begin();
    if (GFX & GFX_FLOAT) {
      float fVal = *(float*)param;
      if (fVal == var.fValPrev)
        return;
      var.fValPrev = fVal;
      strVal.print(fVal, GFX_FORMAT_DP);
    }
    else {    // int
      int iVal = *(uint16_t*)param;
      if (iVal == var.iValPrev)
        return;
      var.iValPrev = iVal;
      strVal.print(iVal);
    }
    UpdateStr();
}


void GFX_ELEM::UpdateStr() {  // const PString& str doesn't work for SAMD M0
    uint8_t iChars = strVal.length();
    uint8_t iFont = GFX_FORMAT_FONT;
    uint8_t pxChar = iFont * 6;
    int px;
    int8_t iCharsToBlank = var.iSizePrev - iChars;
  
    if (iCharsToBlank > 0) {
      uint8_t pyChar = iFont * 8;
      if (!GFX_FORMAT_LJUSTIFY)    // Right Justified
        px = x - var.iSizePrev * pxChar;
      else    // Left Justified
        px = x + iChars * pxChar;
      do {
        lcd.fillRect(px, y, pxChar, pyChar, clrBk);
        px += pxChar;
      } while (--iCharsToBlank > 0);
    }
    px = x;
    if (!GFX_FORMAT_LJUSTIFY)   // Right Justified
      px -= iChars * pxChar;   // * FontSize * font columns
    
    lcd.setTextSize(iFont);
    lcd.setTextColor(clr, clrBk);    // text and background color
    lcd.setCursor(px, y);
    lcd.print(strVal);
    var.iSizePrev = iChars;
}

void GFX_ELEM::UpdateText() const {  // const PString& str doesn't work for SAMD M0
    strVal.begin();
    if (GFX & GFX_PROGMEM) {   // in PROGMEM not SRAM
	    char buff[sizeof(cBuff)];
      strncpy_P(buff, (char*)param, sizeof(cBuff));
	    strVal.print(buff);				// print string so it sets it's length!
    } else
      strVal.print((char*)param);
    uint8_t iChars = strVal.length();
    uint8_t iFont = GFX_FORMAT_FONT;
    uint8_t pxChar = iFont * 6;
    int px = x;
  
    if (!GFX_FORMAT_LJUSTIFY)   // Right Justified
      px -= iChars * pxChar;   // * FontSize * font columns
    lcd.setTextSize(iFont);
    if (GFX & GFX_BUTTON)
      lcd.setTextColor(clr);    // text color, transparent backgroung
    else
      lcd.setTextColor(clr, clrBk);    // text and background color
    lcd.setCursor(px, y);
    lcd.print(strVal);
}



