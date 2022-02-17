
#include "config.h"

#include "Display.h"

#include <Thruster_DataLink.h>
#include "imu.h"

// if rotated 90
#define DISP_WIDTH  TFT_HEIGHT
#define DISP_HEIGHT TFT_WIDTH

#define PAGE_Y_TOP 30
#define PAGE_Y_LINE 25
#define X_UNITS 210

#if DISP_USE_SPRITE
  TFT_eSprite lcd = TFT_eSprite(&M5.Lcd);
#else
  M5Display& lcd = M5.Lcd;
#endif

#include <Utils.h>    // for periodic trigger
extern PeriodicTrigger trigFast;       // 50Hz trigger - to check for zeroing
extern PeriodicTrigger trigLCD;        // 25Hz screen update - to check for zeroing
extern PeriodicTrigger trigSec;      // 1Hz trigger - period gets zeroed when creating sprite

//#include <esp_heap_caps.h>

void Display::init() {
  M5.Lcd.setRotation(3);
//    lcd.TFT_eSPI::setRotation(3);     // to match M5.Lcd - allows Disbuff.fillScreen(clr) to work - can use fillSprite(clr) anyway!
//    lcd.createSprite(M5.lcd.width(), M5.lcd.height());
  log_d("trigSec period Disp1: %d  %d  %d", trigSec.period, trigFast.period, trigLCD.period);

#if DISP_USE_SPRITE
  uint32_t sz0 = esp_get_free_heap_size();    // DISP_WIDTH = 240, DISP_HEIGHT = 135
  void* ptSprite = NULL;
  ptSprite = lcd.createSprite(8, 1);   // zeros 'trigSec.period'.  Writing to invalid memory location??  Needs 64800 bytes for 135x240 pixels
  uint32_t sz1 = esp_get_free_heap_size();
  fill(TFT_WHITE);

  log_d("trigSec period Disp2: %d  %d  %d", trigSec.period, trigFast.period, trigLCD.period);
//  log_d("trigSec period Disp2: %d", trigSec.period);
  log_d("Sprite pointer  p: %p", ptSprite);
  log_d("Sprite pointer lx: %lx", ptSprite);
  log_d("Sprite heap change: %d", sz0 - sz1);   // 64820 gets allocated
  log_d("Sprite heap sz0, sz1: %d  %d", sz0, sz1);

#if TFT_ESPI_VERSION
#endif

//  trigFast.period = 23;
//  trigLCD.period = 47;
  trigSec.period = 990;
  log_d("trigSec period Disp2a: %d  %d  %d", trigSec.period, trigFast.period, trigLCD.period);

//  void* mem = malloc(10);

#else
#endif
  fill(TFT_BLACK);
  log_d("trigSec period Disp3: %d", trigSec.period);
  pushSprite();
  log_d("trigSec period Disp4: %d", trigSec.period);


  pageSubs[PAGE_MAIN] = 0;
  pageSubs[PAGE_IMU] = 0;
  pageSubs[PAGE_CTRLPOWER] = 1;
  page_change(PAGE_MAIN);
}

void Display::pushSprite() {
#if DISP_USE_SPRITE
  lcd.pushSprite(0, 0);
#else
      // nothing needed
#endif
}
void Display::fill(uint16_t clr) {
#if DISP_USE_SPRITE
    lcd.fillSprite(clr);
#else
  lcd.fillScreen(BLACK);
#endif
}

void Display::update() {
  page_update();
  pushSprite();
}



void Display::page_changeNext() {
  if (++pageCurr >= PAGE_LAST)
    pageCurr = 0;
  pageCurrSub = 0;
  page_change(pageCurr);
}
void Display::page_changePrev() {
  if (--pageCurr < 0)
    pageCurr = PAGE_LAST - 1;
  pageCurrSub = 0;
  page_change(pageCurr);
}
void Display::page_changeSubNext() {
  if (++pageCurrSub > pageSubs[pageCurr])
    pageCurrSub = pageSubs[pageCurr];
  if (pageSubs[pageCurr] != 0)
    page_change(pageCurr);
}
void Display::page_changeSubPrev() {
  if (--pageCurrSub < 0)
    pageCurrSub = 0;
  if (pageSubs[pageCurr] != 0)
    page_change(pageCurr);
}

void Display::page_change(int page) {
  pageCurr = page;
  GfxElem_Num = 0;    // remove GfxElem's
  msgNum = 0;

  fill(TFT_BLACK);
  page_setup();
}

void Display::page_setup() {
  lcd.setTextFont(1);   // 4 ?
  lcd.setTextSize(2);
  lcd.setTextWrap(0);

  switch (pageCurr + pageCurrSub * PAGE_SUB1) {
  case PAGE_MAIN:
    page_setupMain(); break;
  case PAGE_DRIVEFB:
    page_setupDriveFB(); break;
  case PAGE_CTRLPOWER:
    page_setupCtrlPower(); break;
   case PAGE_CTRLPOWER + PAGE_SUB1:
    page_setupCtrlPowerS1(); break;
 case PAGE_TIMING:
    page_setupTiming(); break;
  case PAGE_IMU:
    page_setupIMU(); break;
  case PAGE_MEMORY:
    page_setupMem(); break;
    break;
  default:
    ;
  }
  page_setup_statusBar();
}


void Display::page_update() {
  lcd.setTextFont(1);   // 4 ?
  lcd.setTextSize(2);
  lcd.setTextWrap(0);
  if (++updateTextCycle >= updateTextMax) {
    updateTextCycle = 0;
    page_updateText_statusBar();
  }
  switch (pageCurr + pageCurrSub*PAGE_SUB1) {
  case PAGE_MAIN:
    page_updateMain(); break;
  case PAGE_DRIVEFB:
    page_updateDriveFB(); break;
  case PAGE_CTRLPOWER + 0:
    page_updateCtrlPower(); break;
  case PAGE_CTRLPOWER + PAGE_SUB1:
    page_updateCtrlPowerS1(); break;
  case PAGE_TIMING:
    page_updateTiming(); break;
  case PAGE_IMU:
    page_updateIMU(); break;
  case PAGE_MEMORY:
    page_updateMem(); break;
  }

// if any Gfx
  for (int i = 0; i < GfxElem_Num; i++)
    GfxElem[i].Update();

}

void Display::page_setup_statusBar() {}
void Display::page_updateText_statusBar() {
  lcd.setTextColor(TFT_YELLOW, TFT_BLACK);
  lcd.setCursor(160, 5);
  lcd.print(ctrl.voltageBattery, 3);
  lcd.print("v");

}



//  Main    ////////
////////////////////
void Display::page_setupMain() {
  lcd.setTextColor(TFT_GREEN, TFT_BLACK);
  //lcd.setTextDatum(TR_DATUM);
 
  lcd.setCursor(0, PAGE_Y_TOP);
  lcd.println("Throttle");

  // Setup GfxElem's for bars and values
  int idx = -1;
  GfxElem[++idx].SetParam(&ctrl.throttle);
  GfxElem[  idx].Init(GFX_fBARH, TFT_YELLOW, TFT_DARKGREY, 20, 100, 200, 20);
  GfxElem[  idx].pxPerUnit = 0.5;   // 2 normally
/*
  GfxElem[++idx].SetParam("Scan");
  GfxElem[  idx].Init(GFX_BUTTONLABEL, TFT_YELLOW, TFT_RED, 10, 30, 80, 40, 10);
  GfxElem[  idx].SetCB1(gfxCB_BLEscan);

  GfxElem[++idx].SetParam("Conn");
  GfxElem[  idx].Init(GFX_BUTTONLABEL, TFT_GREEN, TFT_LIGHTGREY, 150, 30, 70, 40, 10);
  GfxElem[  idx].SetCB1(gfxCB_BLEconnect);
*/
  GfxElem[++idx].InitLast();
  GfxElem_Num = idx;

}
void Display::page_updateMain() {
  if (updateTextCycle == 0)
    page_updateMain_Text();
}
void Display::page_updateMain_Text() {
  lcd.setTextSize(3);
  lcd.setCursor(0, PAGE_Y_TOP + PAGE_Y_LINE * 0);
  lcd.setTextColor(TFT_GREEN, TFT_BLACK);
  lcd.printf("%6.1f%%", ctrl.throttle);

}


void Display::page_setupDriveFB() {}
void Display::page_setupCtrlPower() {}
void Display::page_setupCtrlPowerS1() {}
void Display::page_setupTiming() {}
void Display::page_setupMem() {}

void Display::page_updateDriveFB() {}
void Display::page_updateCtrlPower() {}
void Display::page_updateCtrlPowerS1() {}
void Display::page_updateTiming() {}
void Display::page_updateMem() {}


void Display::page_setupIMU() {}
void Display::page_updateIMU() {
  if (updateTextCycle == 0)
    page_updateIMU_Text();
}
void Display::page_updateIMU_Text() {
  Vector3f& gyro = imuMon.gyro;
  Vector3f& acc = imuMon.acc;

  lcd.setTextSize(2);
//  lcd.setTextFont(1);
  
  lcd.setCursor(0, PAGE_Y_TOP + PAGE_Y_LINE * 0);
  lcd.setTextColor(TFT_GREEN, TFT_BLACK);
  lcd.printf("%5.2f %5.2f %5.2f  ", gyro.x, gyro.y, gyro.z);
  lcd.cursor_x = X_UNITS;
//  lcd.println("d°/s");

  lcd.setCursor(0, PAGE_Y_TOP + PAGE_Y_LINE * 1);
  lcd.setTextColor(TFT_RED, TFT_BLACK);
  lcd.printf("%5.2f %5.2f %5.2f  ", acc.x, acc.y, acc.z);
  lcd.cursor_x = X_UNITS;
//  lcd.println("G");

  lcd.setCursor(0, PAGE_Y_TOP + PAGE_Y_LINE * 2);
  lcd.setTextColor(TFT_BLUE, TFT_BLACK);
  lcd.printf("%5.1f %5.1f %5.1f   ", imuMon.pitch, imuMon.roll, imuMon.yaw);
  lcd.cursor_x = X_UNITS;
//  lcd.println("d°");

  lcd.setCursor(0, PAGE_Y_TOP + PAGE_Y_LINE * 3);
  lcd.setTextColor(TFT_ORANGE, TFT_BLACK);
  lcd.printf("Temp: %.1f °  \xB0 a \260 C \n", imuMon.getTemp());   // deg = ° (\xB0 or \176(dec) or \260(oct)) - not printing with font 4, some fonts only have 96 characters

}  




