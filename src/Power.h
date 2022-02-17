#pragma once



// Some AXP functions in AXP20X library but not in AXP192 library
// Need to modify AXP192 library to make Read_bit() functions public
// also axp.Read32bit() library function only requested 2 bytes, need to change to request 4 bytes (7/9/2021)
uint8_t axp_GetAdcSamplingRate();    //Adc rate can be read from 0x84
float axp_GetBatChargeCurrent();       // axp.GetBatChargeCurrent() is wrong as Read12Bit(0x7A) * 0.5, is 13 bit not 12 bit
float axp_GetBatDischargeCurrent();    // axp.GetBatDischargeCurrent() is not defined




class AXP192_M5Stick : public AXP192 {

public:     // additional definitions from M5Core2 AXP192
    enum CHGCurrent{
        kCHG_100mA = 0,
        kCHG_190mA,
        kCHG_280mA,
        kCHG_360mA,
        kCHG_450mA,
        kCHG_550mA,
        kCHG_630mA,
        kCHG_700mA,
        kCHG_780mA,
        kCHG_880mA,
        kCHG_960mA,
        kCHG_1000mA,
        kCHG_1080mA,
        kCHG_1160mA,
        kCHG_1240mA,
        kCHG_1320mA,
    };

// Add functions at appear in M5Core2 library
//	void SetChargeVoltage(uint8_t level);
	void SetCHGCurrent(uint8_t level);
    bool isACIN();
    bool isVBUS();

    void PrepareToSleep();
    void RestoreFromLightSleep();

    void SetLed(bool state);

};


uint8_t axp_GetAdcSamplingRate();       // axp.GetAdcSamplingRate() is not defined.  Adc rate can be read from 0x84
float axp_GetBatChargeCurrent();        // axp.GetBatChargeCurrent() is wrong as Read12Bit(0x7A) * 0.5, is 13 bit not 12 bit
float axp_GetBatDischargeCurrent();     // axp.GetBatDischargeCurrent() is not defined.

extern class Power power;

class Power {

public:
    void init();
    void checkInterrupts();
    int getSeconds();
    void incSeconds();

    int idleCheck();
    int idleTimeOut();
    void idleReset();
    bool idleShutdown();

    void lightSleep();
    void deepSleep();
    void shutdown();

    void deepSleep_WOM();

    void standby();
    void wakeup();
    void restart();

    void get_wakeup_reason_string(char *cbuf, int cbuf_len);

};
