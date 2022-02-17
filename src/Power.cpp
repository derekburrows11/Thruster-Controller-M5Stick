
#include "config.h"
#include "Power.h"
#include "Performance.h"
#include "imu.h"

#include "CtrlLink_BLE Server.h"
#include <Thruster_DataLink.h>

//#include "_Main Controller M5.h"

#define LED_RED 10


// Some AXP functions in AXP20X library but not in AXP192 library
// Need to modify AXP192 library to make Read_bit() functions public
// also axp.Read32bit() library function only requested 2 bytes, need to change to request 4 bytes (7/9/2021)
uint8_t axp_GetAdcSamplingRate() {    //Adc rate can be read from 0x84
// from TTGO Watch AXP20X_Class
//   _readByte(AXP202_ADC_SPEED, 1, &val);          // AXP202_ADC_SPEED = 0x84
//    return 25 * (int)pow(2, (val & 0xC0) >> 6);   // result in Hz
  uint8_t val = 25 << ((axp.Read8bit(0x84) & 0xC0) >> 6);   // axp.Read8bit(0x84);   // private function...
  return val;
}
float axp_GetBatChargeCurrent() {    // axp.GetBatChargeCurrent() is wrong as Read12Bit(0x7A) * 0.5, is 13 bit not 12 bit
    return axp.Read13Bit(0x7A) * 0.5;
}
float axp_GetBatDischargeCurrent() {    // axp.GetBatDischargeCurrent() is not defined
    return axp.Read13Bit(0x7C) * 0.5;
}




void AXP192_M5Stick::SetCHGCurrent(uint8_t level) {
    uint8_t data = Read8bit(0x33);
    data &= 0xf0;
    data = data | (level & 0x0f);
    Write1Byte(0x33, data);
}

bool AXP192_M5Stick::isACIN() {
    return 0;
}
bool AXP192_M5Stick::isVBUS() {
    return 0;
}
void AXP192_M5Stick::PrepareToSleep() {

}
void AXP192_M5Stick::RestoreFromLightSleep() {

}

void AXP192_M5Stick::SetLed(bool state) {
    pinMode(LED_RED, OUTPUT);
    digitalWrite(LED_RED, !state);
}


// Wakeup Interrupt routines
//////////////////////////////////
RTC_DATA_ATTR int bootCount = 0;
volatile uint32_t g_wom_count = 0;
volatile uint32_t g_wom_last_millis = 0;
void IRAM_ATTR mpu6886_wake_on_motion_isr(void) {
    g_wom_count++;
    g_wom_last_millis = millis();
}




uint16_t idleTime = 0;          // seconds idle
uint16_t idleTime_Timeout;      // seconds
uint16_t idleTime_Timeout_Battery = 1800;    // seconds
uint16_t idleTime_Timeout_Supply  = 3600;    // seconds
uint16_t sTime;    // seconds since started
bool pwrLEDState;

int sleep_wasSleeping = 0;      // 0=no, 1=lightsleep, 2=deepsleep, 10=powerup



////////////////////////
// class Power
////////////////////////

void Power::init() {
    axp.SetCHGCurrent(axp.kCHG_100mA);      // default is 100mA

//    setCpuFrequencyMhz(9);        // 240, 160, 80, 40, 20, 10 are allowed
    bootCount++;
    log_d("Boot: %d\n", bootCount);

    idleReset();
}


void Power::deepSleep_WOM() {       // set to wake on motion

    // set up ISR to trigger on GPIO35
    delay(100);
    pinMode(GPIO_NUM_35, INPUT);
    delay(100);
    attachInterrupt(GPIO_NUM_35, mpu6886_wake_on_motion_isr, FALLING);


    // set up mpu6886 for low-power operation

    MPU6886_ext imu_ext;
    imu_ext.enableWakeOnMotion(M5.Imu.AFS_16G, 4);      // Interrupts will now be generated.
    
    // wait until IMU ISR hasn't triggered for X milliseconds
    while(1) {
        uint32_t since_last_wom_millis = millis() - g_wom_last_millis;
        if(since_last_wom_millis > 5000) {
            break;
        }
        Serial.printf("waiting : %d", since_last_wom_millis);
        delay(1000);
    }

    
    esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);  // disable all wakeup sources
    // enable waking up on pin 35 (from IMU, AXP or RTC)
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_35, LOW);      // low on interrupt


    //Go to sleep now
    Serial.println("Going to sleep now");
    M5.Axp.SetSleep(); // conveniently turn off screen, etc.
    delay(100);
    esp_deep_sleep_start();     // Never returns from this
}



void Power::incSeconds() {
    ++sTime;
    ++idleTime;
//    log_i("idleTime sec: %d", idleTime);
}

void Power::idleReset() {
    idleTime = 0;
}
// Return idle time seconds count
int Power::idleCheck() {
    incSeconds();
    if (clink.isConnected()) {
        idleReset();
    }

     ctrl.voltageBattery = axp.GetBatVoltage();

    pwrLEDState = !pwrLEDState;
    axp.SetLed(pwrLEDState);

    if (sTime % 30 == 5)
        log_v("Seconds: %d", sTime);

    if (axp.isVBUS() || Axp.isACIN())      // If VBUS (5V Bus) or 'ACin' (USB supply) connected - longer timeout
        idleTime_Timeout = idleTime_Timeout_Supply;
    else
        idleTime_Timeout = idleTime_Timeout_Battery;

    if (idleTime > idleTime_Timeout) {
        log_i("Time sec: %d", idleTime);
        log_i("Timeout sec: %d", idleTime_Timeout);
        idleShutdown();
        idleReset();
    }
    return idleTimeOut();
}
int Power::idleTimeOut() {
  return idleTime_Timeout - idleTime;
}

bool Power::idleShutdown() {
    Serial.println("Power::idleShutdown - Shutting Down");
    Serial.printf("Time %f sec\n", millis()*0.001);
//    motor_vibe(20);        // vibe time doesn't work if delaying instead of polling!!
    delay(20);
//    motor_vibe_off();
    Power::lightSleep();     // is awake when returning
//    Power::deepSleep();
//    Power::shutdown();

    return 1;
}


void Power::lightSleep() {   // 2.5mA, 6days (with 80MHz) - from My-TTGO-Watch
    Serial.println("Power::lightSleep - Going to light sleep");
    Serial.printf("Time %f sec\n", millis()*0.001);
//    adc_Power::off();
//    adc_Power::release();

    perf.memory_logStats();
    log_i("go standby");
    delay(200);

//    adc_Power::release();
//    esp_sleep_enable_touchpad_wakeup();
//    esp_sleep_get_touchpad_wakeup_status();

//    setCpuFrequencyMhz(80);
//    setCpuFrequencyMhz(20);


//    esp_sleep_enable_ext1_wakeup(GPIO_SEL_35, ESP_EXT1_WAKEUP_ALL_LOW);     // AXP202_INT = 35
    //esp_sleep_enable_ext1_wakeup(_BV(AXP202_INT), ESP_EXT1_WAKEUP_ALL_LOW);     // AXP202_INT = 35, or GPIO_SEL_35

//    gpio_wakeup_enable((gpio_num_t)AXP202_INT, GPIO_INTR_LOW_LEVEL);
//    gpio_wakeup_enable((gpio_num_t)BMA423_INT1, GPIO_INTR_HIGH_LEVEL);
//    esp_sleep_enable_gpio_wakeup();    // can wakeup from light sleep only, deep sleep wakes straight away (and resets).
//    M5.Axp.SetSleep();
//    M5.Axp.PowerOff();
//    M5.Axp.PrepareToSleep();
//    M5.Axp.DeepSleep();

// if touch screen
    // set to wake on touch - interrupt CST_INT (39), low when pressed - see Touch.ispressed()
//    gpio_wakeup_enable((gpio_num_t)CST_INT, GPIO_INTR_LOW_LEVEL);

    esp_sleep_enable_gpio_wakeup();    // can wakeup from light sleep only, deep sleep wakes straight away (and resets).

    // below based on M5 light sleep
//    M5.Axp.LightSleep(SLEEP_SEC(10));       // light sleep for 10 seconds, then wake and shut down fully.  Can wake on touch straight away in light sleep.

    esp_sleep_source_t wakeup;
    int numTimerWakeups = 60;        // light sleep for 10 seconds 60 times, then shutdown
    Axp.PrepareToSleep();
    do {
        esp_sleep_enable_timer_wakeup(SLEEP_SEC(10));
        esp_light_sleep_start();        // sleeps here
        wakeup = esp_sleep_get_wakeup_cause();
        if (wakeup != ESP_SLEEP_WAKEUP_TIMER)
            break;
        Serial.printf("Woken on 10sec timer: %d\n", numTimerWakeups);
        Axp.SetLed(1);
        delay(50);
        Axp.SetLed(0);
        M5.update();
        if (M5.BtnA.isPressed())
            break;
        if (M5.BtnB.isPressed())
            break;
        if (numTimerWakeups-- <= 0)
            Power::shutdown();       // doesn't return
    } while (1);

    Axp.RestoreFromLightSleep();


    // check wakeup source, if timer then shutdown fully
//    esp_sleep_source_t wakeup = esp_sleep_get_wakeup_cause();
    if (wakeup == ESP_SLEEP_WAKEUP_GPIO)        // GPIO from touch interrupt CST_INT
        Serial.println("Woken on GPIO pin");
    else if (wakeup == ESP_SLEEP_WAKEUP_TIMER)
        Serial.println("Woken on timer");
    else if (wakeup == ESP_SLEEP_WAKEUP_TOUCHPAD)   // for esp's touchpad pins
        Serial.println("Woken on touch");
    else
        Serial.printf("Woken on other code: %d\n", wakeup);


//    esp_light_sleep_start();    // returns after waking on interrupt.
    sleep_wasSleeping = 1;      // was light sleep
    // Nothing was set to respone to interrupt.  Needs to turn off with 4sec press, then on with 2sec

    Power::wakeup();
//    ttgo->displayWakeup();
//    ttgo->openBL();
//    tft->setTextColor(TFT_RED);
//    tft->println("Waking...");

}


void Power::deepSleep() {   // ??mA
    Serial.println("Power::deepSleep - Going to deep sleep");
    delay(200);

//    adc_Power::off();        // from TTGO, needed?
    setCpuFrequencyMhz(20);

    M5.Axp.DeepSleep();
//    esp_deep_sleep_start();     // returns??  Nothing set to wake it.  Needs to turn off with 4sec press, then on with 2sec
    sleep_wasSleeping = 2;      // was deep sleep, but doesn't return from deepsleep routine
}


void Power::shutdown() {
    Serial.println("Power::shutdown - shutting down!");
    delay(200);
//    M5.shutdown();
    m5.axp.PowerOff();

//    esp_register_shutdown_handler();

/*    ttgo->power->setTimeOutShutdown();
    ttgo->power->setlongPressTime();
    ttgo->power->setShutdownTime();
*/
}


void Power::wakeup() {
    Serial.println("Power::wakeup - waking up");
//    setCpuFrequencyMhz(240);
    setCpuFrequencyMhz(40);
//    setCpuFrequencyMhz(10);

//    ttgo->startLvglTick();
    //ttgo->displayWakeup();
   // m5.setWakeupButton();
    
//    ttgo->rtc->syncToSystem();
//    lv_disp_trig_activity(NULL);
//    ttgo->bma->enableStepCountInterrupt();

//    float sleepTime = (millis()-lenergySleepStart) * 0.001;
//    Serial.printf("WAKING UP... Slept for %.2f seconds\n", sleepTime);

//    ttgo->bl->adjust(120);
//    m5.Axp.SetLcdVoltage(1000);
//    m5.Axp.SetLCDRSet(1);
//    M5.Axp.RestoreFromLightSleep();     // done already by light sleep
    M5.Axp.ScreenBreath(10);        // brightness - DCDC3

 
}


/* Method to print the reason by which ESP32 has been awoken from sleep */
void Power::get_wakeup_reason_string(char *cbuf, int cbuf_len) {
    esp_sleep_wakeup_cause_t wakeup_reason;
    wakeup_reason = esp_sleep_get_wakeup_cause();
    switch(wakeup_reason) {
        case ESP_SLEEP_WAKEUP_EXT0     : snprintf(cbuf, cbuf_len, "ext0");             break;
        case ESP_SLEEP_WAKEUP_EXT1     : snprintf(cbuf, cbuf_len, "ext1");             break;
        case ESP_SLEEP_WAKEUP_TIMER    : snprintf(cbuf, cbuf_len, "timer");            break;
        case ESP_SLEEP_WAKEUP_TOUCHPAD : snprintf(cbuf, cbuf_len, "touchpad");         break;
        case ESP_SLEEP_WAKEUP_ULP      : snprintf(cbuf, cbuf_len, "ULP");              break;
        default                        : snprintf(cbuf, cbuf_len, "%d",wakeup_reason); break;
    }
}

void Power::restart() {
   ESP.restart();
}


