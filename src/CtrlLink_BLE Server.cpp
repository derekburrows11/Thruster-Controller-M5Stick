/*
Controller Mini Throttle - Runs as a low battery powered device
Either:   1. Runs BLE as Client - listening/scanning occasionally if a server wants to connect - a Controller Screen or Thruster Drive
          2. Runs BLE as Server - advertising occasionally so other clients can connect - a Controller Screen or Thruster Drive
1. --------
Listen/Scan occasionally
If found a recently connected server, then connect to that if allowed
If found server wanting to connect then connect
If connected to Thruster Drive, keep checking for Controller Screen to connect to instead.
If we lose connection to Controller Screen while running, also check for direct connection to the Thruster Drive

2. --------
Advertise occasionally
Allow multi connections from a matched Controller Screen and Thruster Drive in order to move to Controller Screen connection
If a Thruster Drive is connected, also advertise to allow a Controller Screen to connect

*/

#include "config.h"
#include "CtrlLink_BLE Server.h"
#include "Display.h"

//#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include <Thruster_DataLink.h>





#define SERVICE_UUID 			"1bc68b2a-f3e3-11e9-81b4-2a2ae2dbcce4"
#define CHARACTERISTIC_RX_UUID 	"1bc68da0-f3e3-11e9-81b4-2a2ae2dbcce4"
#define CHARACTERISTIC_TX_UUID 	"1bc68efe-f3e3-11e9-81b4-2a2ae2dbcce4"

BLEServer *pServer = NULL;
BLEService *pService = NULL;
BLECharacteristic * pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;
int numConnected = 0;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      numConnected++;
      clink.setConnected();
      pServer->getConnectedCount();

    Serial.printf("New connection: %i\n", numConnected);

//      BLEDevice::startAdvertising();      // Keep advertising to allow multi connects
      
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      numConnected--;
      clink.setDisconnected();

      pServer->getConnectedCount();
      Serial.printf("Lost connection: %i\n", numConnected);

    }
};

uint8_t* data = new uint8_t[128];

class MyCharacteristicCallbacks: public BLECharacteristicCallbacks {

    void onWrite(BLECharacteristic *pCharacteristic) {
    data = pCharacteristic->getData();
    }
};


bool CtrlLink_BLE::init()
{
//	uint64_t chipid = ESP.getEfuseMac();
//    String blename = "M5-" + String((uint32_t)(chipid >> 32), HEX);
    String blename = "M5Stick Throttle";

    BLEDevice::init(blename.c_str());
	//BLEDevice::setPower(ESP_PWR_LVL_N12);
	pServer = BLEDevice::createServer();

	pServer->setCallbacks(new MyServerCallbacks());
	pService = pServer->createService(SERVICE_UUID);
	pTxCharacteristic = pService->createCharacteristic(
											CHARACTERISTIC_TX_UUID,
											BLECharacteristic::PROPERTY_NOTIFY
//                      BLECharacteristic::PROPERTY_READ   |
//                      BLECharacteristic::PROPERTY_WRITE  |
//                      BLECharacteristic::PROPERTY_NOTIFY |
//                      BLECharacteristic::PROPERTY_INDICATE
										);
	pTxCharacteristic->addDescriptor(new BLE2902());

	BLECharacteristic * pRxCharacteristic = pService->createCharacteristic(
												CHARACTERISTIC_RX_UUID,
												BLECharacteristic::PROPERTY_WRITE
//                      BLECharacteristic::PROPERTY_READ   |
//                      BLECharacteristic::PROPERTY_WRITE  |
//                      BLECharacteristic::PROPERTY_NOTIFY |
//                      BLECharacteristic::PROPERTY_INDICATE
											);
  	pRxCharacteristic->setCallbacks(new MyCharacteristicCallbacks());


// Start the service
  pService->start();		// optional now if not done later

  // Start advertising - if started service
//  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
//  pAdvertising->addServiceUUID(SERVICE_UUID);
//  pAdvertising->setScanResponse(1);
//  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
//  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
//  pAdvertising->setMinPreferred(0x12);


  Serial.println("Waiting a client connection to notify...");

//	pServer->getAdvertising()->start();
//  BLEDevice::getAdvertising();
  BLEDevice::startAdvertising();
//	pServer->startAdvertising();

    return true;
}


void CtrlLink_BLE::close() {
    pService->stop();
    pServer->getAdvertising()->stop();
}

int txValue;

void CtrlLink_BLE::sendTx() {		// send at about 20Hz
  if (!deviceConnected) {       // or CtrlLink_BLE::bConnected
    return; 
  }
  
  // Send data
  struct dataControllerToDrive fromCtrl;
  fromCtrl.id = DL_IDbit_DEST_DRIVE1 | DL_IDbit_DEST_DRIVE2 | DL_ID_SRC_CTRL | DL_ID_TYPE_FAST;
  ctrl.packetCount++;
  ctrl.SetData(fromCtrl);

  msLastMsgSent = millis();
    txValue++;

    char str[40];
    sprintf(str, "Bat %5.3f Thr %5.1f", ctrl.voltageBattery, ctrl.throttle);
	pTxCharacteristic->setValue(str);

//	pTxCharacteristic->setValue( (uint8_t*)&fromCtrl, sizeof(fromCtrl) );
//	pTxCharacteristic->setValue(ctrl.throttle);
//	pTxCharacteristic->setValue(txValue);
	pTxCharacteristic->notify();

}


void CtrlLink_BLE::draw_icon_ble(uint16_t x, uint16_t y, uint16_t color, uint16_t sz) {
    lcd.drawLine(x+   0, y+sz*1, x+sz*2, y+sz*3, color);
    lcd.drawLine(x+sz*2, y+sz*3, x+sz*1, y+sz*4, color);
    lcd.drawLine(x+sz*1, y+sz*4, x+sz*1, y+   0, color);
    lcd.drawLine(x+sz*1, y+   0, x+sz*2, y+sz*1, color);
    lcd.drawLine(x+sz*2, y+sz*1, x+   0, y+sz*3, color);
}

void CtrlLink_BLE::display() {

    uint8_t senddata[2] = {0};

    while ((!M5.BtnA.isPressed()) && (!M5.BtnB.isPressed()))
    {
#if DISP_USE_SPRITE
        lcd.fillSprite(TFT_BLACK);
#else
        lcd.fillScreen(TFT_BLACK);
#endif
        if (deviceConnected) {
            draw_icon_ble(180, 16, TFT_BLUE);
            lcd.setTextColor(TFT_LIGHTGREY);
            lcd.setTextSize(3);
            lcd.setCursor(12, 20);
            //lcd.printf("BLE connect!\n");
            lcd.printf("BLE Send\n");
            lcd.setTextSize(5);
            lcd.setCursor(12, 75);
            if( senddata[0] % 4 == 0 )
            {
                lcd.printf("0x%02X>  ",senddata[0]);
            }
            else if( senddata[0] % 4 == 1 )
            {
                lcd.printf("0x%02X>>",senddata[0]);
            }
            else if( senddata[0] % 4 == 2 )
            {
                lcd.printf("0x%02X >>",senddata[0]);
            }
            else if( senddata[0] % 4 == 3 )
            {
                lcd.printf("0x%02X  >",senddata[0]);
            }

            senddata[1]++;
            if( senddata[1] > 3 )
            {
                senddata[1] = 0;
                senddata[0]++;
                pTxCharacteristic->setValue( senddata, 1 );
                pTxCharacteristic->notify();
            }
        }
        else
        {
            lcd.setTextSize(2);
            lcd.setCursor(12, 20);
            lcd.setTextColor( TFT_RED );
            lcd.printf("BLE disconnect\n");
            lcd.setCursor(12, 45);
            lcd.setTextColor(TFT_CYAN);
			
            lcd.printf(String("Name:"+blename+"\n").c_str());
            lcd.setCursor(12, 70);
            lcd.printf("UUID:1bc68b2a\n");
            draw_icon_ble(180, 16, TFT_LIGHTGREY);

            
        }
#if DISP_USE_SPRITE
        lcd.pushSprite(0, 0);
#endif
        
        M5.update();
        delay(100);
    }
    while ((M5.BtnA.isPressed()) || (M5.BtnB.isPressed()))
    {
        M5.update();
        M5.Beep.tone(4000);
        delay(10);
    }
    delay(50);
    M5.Beep.mute();
    lcd.setTextColor(TFT_WHITE);
    pService->stop();
    pServer->getAdvertising()->stop();
}

