#pragma once

#include <Vector.h>

extern class imuMonitor imuMon;

class imuMonitor {
public:
    float pitch = 0;
    float roll  = 0;
    float yaw   = 0;
    float tempMPU6886;
    Vector3f acc, gyro, gyroRad, ahrs;


public:
    void init();
    void update();
//    void updateAHRS();
    float getTemp();


};


// MPU6886 extras - as of Dec 2021 these functions are in M5StickC MPU6886 library, but not M%StickC-Plus!
// Also need to change MPU6886.h private functions to protected.
// Done for M5StickCPlus lib version 0.0.5 on 24/1/2022.  Not done for M5Core2 version 0.0.6
class MPU6886_ext : public MPU6886 {
public:
  void enableWakeOnMotion(Ascale ascale, uint8_t thresh_num_lsb);
protected:
  void SetINTPinActiveLogic(uint8_t level);
  void DisableAllIRQ();
  void ClearAllIRQ();
};

