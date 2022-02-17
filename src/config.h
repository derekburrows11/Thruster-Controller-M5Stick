// Config for Thruster Controller on M5stick-c Plus
// with 135x240 ST7789V2 LCD
// MPU6886 IMU, 120mAh battery

//#include <M5Core2.h>
#include <M5StickCPlus.h>

//#define Use_SerialBT 1    // defined in _Main
#define UseRadio 1
#define USE_BLE 0   // no BLE files in controller yet - for comms to Li ion battery BMS
#define USE_MAGNO 1

#define DISP_USE_SPRITE 1


// M5 class defined members - defined at global scope in _Main.cpp
#if DISP_USE_SPRITE
    extern TFT_eSprite lcd;
#else
    extern M5Display& lcd;
//M5Display& lcd = M5.Lcd;

#endif

//extern AXP192& axp;
#include "Power.h"
extern AXP192_M5Stick& axp;       // locally defined in Power.h
extern MPU6886& imu;
//extern M5Touch& touch;          // for M5Core, not M5Stick
//extern M5Buttons& buttons;      // for M5Core, not M5Stick


