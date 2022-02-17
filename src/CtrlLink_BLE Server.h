#pragma once


extern class CtrlLink_BLE clink;


class CtrlLink_BLE {

public:
    bool init();
    void close();
    void sendTx();
    void display(); // don't use
    void draw_icon_ble(uint16_t x, uint16_t y, uint16_t color = TFT_BLUE, uint16_t sz = 12);

  void setConnected() { bConnected = 1; numConnections++; }
  void setDisconnected() { bConnected = 0; numConnections--; }
  bool isConnected() { return bConnected; }



protected:
  // ESP_NOW_MAX_DATA_LEN defined as 250 in esp_now.h - check BLE Rx max size
  #define MAX_RX_SIZE 100


  uint16_t packetsRxTotal = 0;
  uint16_t packetsRxError = 0;
  uint16_t packetsRxMe = 0;
  uint16_t packetsTx = 0;
  uint16_t msLastMsgSent;
  uint16_t msLastMsgRx;

  static bool bTxOK;
  bool RxOK;
  bool RxTimedOut;
  bool bConnected;
  int numConnections;

  static bool bRxNewData;
  static bool bRxNewDataReading;    // semaphore to Rx callback
  static bool bRxNewDataMissed;
  static int  iRxNewDataLen;
  static uint8_t buffRx[MAX_RX_SIZE];

  String blename;

};
