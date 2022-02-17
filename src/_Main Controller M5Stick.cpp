/*
Thruster Controller
Setup for M5Stick-C Plus
Uses IMU, or Hall sensor to generate a throttle signal

Sets up a BLE server for multiple connections
- Thruster Drive BLE client can connect to read throttle
- Thruster Watch BLE client can connect to read throttle and data from Drive

- Thruster Drive BLE client can also connect to Thruster Watch client ?? to send motor values, and possibly read throttle from wwatch

M5.BtnA is 'M5' button
M5.BtnB is RHS button
M5.Axp.GetBtnPress is power button

*/
#include "config.h"

#include <Utils.h>    // for periodic trigger
#include "Power.h"
#include "Performance.h"
#include "CtrlLink_BLE Server.h"
#include "Display.h"
#include "imu.h"

#include <Thruster_Config.h>
#include <Thruster_DataLink.h>


//#include <esp_pm.h>
//#include <rom/crc.h>
//#include <driver/i2s.h>
//#include <driver/rmt.h>


// declared as extern at top of class definition header files for use of 'global' object in other code
// or maybe should declared as extern in _main.h ??
class Display disp;
class Power power;
class imuMonitor imuMon;
class Performance perf;
class CtrlLink_BLE clink;


// global scope references for M5 class defined members
//M5Display& lcd = M5.Lcd;    // done in Dsiplay.cpp
//AXP192& axp = M5.Axp;
AXP192_M5Stick& axp;       // locally defined in Power.h
MPU6886& imu = M5.Imu;
//M5Touch& touch = M5.Touch;     // for M5Core, not M5Stick
//M5Buttons& buttons = M5.Buttons;    // for M5Core, not M5Stick


//Define variables for data link
struct dataController ctrl;
struct dataDrive drive;


PeriodicTrigger trigFast(20);       // 50Hz trigger
PeriodicTrigger trigLCD(40);        // 25Hz screen update
PeriodicTrigger trigSec(1000);      // 1Hz trigger



//uint32_t loops = 0;
int msTime;             // ms

/*
typedef struct {
    double x;
    double y;
    double z;
} point_3d_t;
*/


void setup() {
    M5.begin();
    Wire.begin(32, 33);     // used for:  

    delay(5000);   // takes some time for the PlatfromIO serial monitor to be ready??
  Serial.println("Starting _Main Controller M5Stick-setup()");

//  trigSec.period = 1000;
//  trigSec.reset();
  log_d("trigSec p: %p", &trigSec);
  log_d("trigSec period 1: %d", trigSec.period);

    Serial.println("Controller M5Stick - Power Init");
    power.init();
    perf.init();
    Serial.println("Controller M5Stick - Display Init");
  log_d("trigSec period 2: %d", trigSec.period);
    disp.init();
  log_d("trigSec period 3: %d", trigSec.period);
    imuMon.init();
  log_d("trigSec period 4: %d", trigSec.period);
    clink.init();
  log_d("trigSec period 5: %d", trigSec.period);

    Serial.println("Controller M5Stick - Starting up Now");
  log_d("trigSec period 6: %d", trigSec.period);


    M5.update();
    if (M5.BtnB.isPressed()) {
        M5.Beep.tone(4000);
        delay(100);
        M5.Beep.mute();
        while(M5.BtnB.isPressed()) {
            M5.update();
            delay(10);
        }
    }
    M5.Axp.ScreenBreath(12);    // brightness 0-12

    disp.pushSprite();
  log_d("trigSec period 7: %d", trigSec.period);

}


void loop() 
{
  msTime = millis();
  perf.loop();
  M5.update();    // for M5 stick does buttons and beep

  if (trigFast.checkTrigger(msTime)) {
//    magno.Check();
    trigFast.done(1);
    trigFast.doneTrigger();
  }

  if (trigLCD.checkTrigger(msTime)) {
    imuMon.update();     // ~1ms exec
    trigLCD.done(1);
    clink.sendTx();
    trigLCD.done(2);

    disp.update();     // ?ms max
    trigLCD.doneTrigger();
  }

  if (trigSec.checkTrigger(msTime)) {
//    log_d("trigSev period ms: %d", trigSec.period);
    power.idleCheck();
    trigSec.done(1);
    trigSec.doneTrigger();
  }

}

