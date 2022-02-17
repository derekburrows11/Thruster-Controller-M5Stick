#pragma once

#include "GFX_Elements.h"

extern class Display disp;

class Display {
    enum DISPLAY_PAGE_t {
        PAGE_MAIN = 0,
        PAGE_DRIVEFB,
        PAGE_CTRLPOWER,
        PAGE_TIMING,
        PAGE_IMU,
        PAGE_MEMORY,
        PAGE_LAST,

        PAGE_SUB0 = 0,
        PAGE_SUB1 = 20,
        PAGE_SUB2 = PAGE_SUB1 * 2,
        PAGE_SUB3 = PAGE_SUB1 * 3,
    };

    int pageCurr = PAGE_MAIN;   // PAGE_MAIN PAGE_IMU
    int pageCurrSub = 0;
    int pageSubs[PAGE_LAST];

    int updateTextCycle = 0;    // staged text updating
    int updateTextMax = 4;      // number of graphics updates for one text update


    static const int GfxElem_Max = 20;
    int GfxElem_Num = 0;
    GFX_ELEM GfxElem[GfxElem_Max];
    int msgNum = 0;
    int msgInitRow = 180;
    int xCol1, xCol2, xCol3, xCol4;

public:
    void init();
    void pushSprite();
    void fill(uint16_t clr);
    void update();


    void page_change(int page);
    void page_changeNext();
    void page_changePrev();
    void page_changeSubNext();
    void page_changeSubPrev();

    void page_setup();
    void page_update();

    void page_setup_statusBar();
    void page_update_statusBar();
    void page_updateText_statusBar();


    // Pages ////////
    ////////////////
    void page_setupMain();
    void page_updateMain();
    void page_updateMain_Text();

    void page_setupDriveFB();
    void page_updateDriveFB();
    void page_updateDriveFB_Text();

    void page_setupCtrlPower();
    void page_updateCtrlPower();
    void page_updateCtrlPower_Text();

    void page_setupCtrlPowerS1();
    void page_updateCtrlPowerS1();
    void page_updateCtrlPowerS1_Text();

    void page_setupTiming();
    void page_updateTiming();
    void page_updateTiming_Text();

    void page_setupIMU();
    void page_updateIMU();
    void page_updateIMU_Text();

    void page_setupMem();
    void page_updateMem();
    void page_updateMem_Text();

};

