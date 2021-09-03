#include "M5Atom.h"
#include "tmDeltaTime.h"


#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

uint8_t userMode=0;

const int ledCols[4]={0xf00000,0x00f000,0x0000f0,0x707070};
uint8_t FSM = 0;
void updateLed()
{
  M5.dis.drawpix(0, ledCols[FSM]);
  FSM = (FSM+1)%4;
}

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

#define TLED (21)
#define LEDC_CHANNEL (1)
#define OCT_SHIFT (-1) // 1オクターブ下げる

class TestMidiSong {
private:
  uint16_t* pTones_i;
  const uint16_t *pMididata;
  uint16_t dataSize;
  int ptr=0;
  int32_t remainTime=0;
  uint8_t previousNote=0;
  void setMidiData(uint8_t _statuAndCh, uint8_t _note){
      Serial2.write(_statuAndCh);   // Status "note on" & "MIDI Ch"
      Serial2.write(_note);     // note number
      Serial2.write((_note!=0)?127:0);    // velocity
  }

public:
  void prepare(uint16_t* _pTones_i, const uint16_t* _pMididata, const uint16_t _dataSize) {
    pTones_i = _pTones_i;
    pMididata = _pMididata;
    dataSize = _dataSize;
    ptr=0;
    remainTime=0;
    previousNote=0;
  }

  void reset(){
    ptr=0;
  }

  uint8_t getPreviousNote(){
    return previousNote;
  }

  int32_t update(unsigned long _deltaTime){
    remainTime-=(int32_t)_deltaTime;
    if(remainTime<=0){
      int note = pMididata[ptr+0];
      int time = pMididata[ptr+1];
      note = max(note+13*OCT_SHIFT,0);
      time = max(time*0.8,1.0);
      ptr = (ptr+2) % dataSize;
      remainTime += time;
      //Serial.println(note);

      if(note!=0){ // 短い音を漏らすことがあるようなので
        setMidiData(0x81,previousNote);
      }
      setMidiData((note!=0)?0x91:0x81,(note!=0)?note:previousNote);

      ledcWriteTone(LEDC_CHANNEL,(note!=0)?pTones_i[note]:0);
      previousNote = note;
    }
    return -1;
  }
};


uint16_t tones_i[128];
void createTones(){
  for(int i=0;i<128;++i){
    tones_i[i]=(uint16_t)(440.0*pow(2.0,(i-69)/12.0));
  }
}

void setMidiData(uint8_t _statuAndCh, uint8_t _note){
    Serial2.write(_statuAndCh);   // Status "note on" & "MIDI Ch"
    Serial2.write(_note);     // note number
    Serial2.write((_note!=0)?127:0);    // velocity
}

const uint16_t mididata[]={0, 768, 86, 235, 0, 5, 86, 475, 0, 5, 88, 235, 0, 5, 88, 235, 0, 5, 88, 235, 0, 5, 89, 475, 0, 5, 94, 715, 0, 245, 89, 235, 0, 5, 89, 235, 0, 5, 91, 475, 0, 5, 91, 715, 0, 5, 89, 1195, 0, 2165, 84, 235, 0, 5, 84, 235, 0, 5, 84, 235, 0, 5, 84, 235, 0, 5, 84, 235, 0, 5, 86, 235, 0, 5, 84, 475, 0, 5, 84, 235, 0, 5, 84, 235, 0, 5, 84, 235, 0, 5, 84, 235, 0, 5, 84, 235, 0, 5, 86, 475, 0, 5, 84, 715, 0, 5, 82, 115, 0, 5, 81, 115, 0, 5, 82, 2395, 0, 725, 79, 475, 0, 5, 79, 475, 0, 5, 79, 235, 0, 5, 79, 235, 0, 5, 79, 235, 0, 5, 79, 235, 0, 725, 82, 235, 0, 245, 82, 235, 0, 5, 84, 235, 0, 4085, 84, 235, 0, 5, 84, 235, 0, 5, 84, 235, 0, 5, 84, 235, 0, 5, 84, 235, 0, 5, 84, 235, 0, 5, 84, 235, 0, 965, 84, 475, 0, 5, 86, 475, 0, 5, 91, 715, 0, 5, 86, 1195, 0, 2165, 88, 235, 0, 5, 88, 235, 0, 5, 88, 235, 0, 5, 88, 235, 0, 5, 88, 235, 0, 5, 88, 235, 0, 5, 88, 235, 0, 965, 89, 475, 0, 5, 91, 475, 0, 5, 91, 475, 0, 245, 89, 1195, 0, 1925, 86, 475, 0, 5, 86, 235, 0, 245, 86, 235, 0, 485, 81, 235, 0, 965, 86, 475, 0, 5, 89, 475, 0, 5, 91, 715, 0, 5, 89, 115, 0, 5, 88, 115, 0, 5, 89, 2395, 0, 485, 84, 955, 0, 5, 86, 955, 0, 5, 88, 955, 0, 5, 84, 955, 0, 5, 81, 1915, 0, 1925, 86, 475, 0, 5, 86, 235, 0, 5, 86, 235, 0, 5, 86, 235, 0, 5, 86, 475, 0, 5, 81, 475, 0, 725, 86, 475, 0, 5, 89, 475, 0, 5, 91, 715, 0, 5, 86, 2155, 0, 965, 88, 955, 0, 5, 88, 955, 0, 5, 88, 475, 0, 485, 84, 235, 0, 5, 84, 235, 0, 5, 86, 235, 0, 245, 88, 115, 0, 5, 91, 115, 0, 5, 96, 115, 0, 5, 100, 115, 0, 5, 103, 115, 0, 5, 100, 115, 0, 5, 96, 115, 0, 5, 91, 115, 0, 5, 88, 115, 0, 5, 91, 115, 0, 5, 96, 115, 0, 5, 100, 115, 0, 5, 103, 115, 0, 5, 100, 115, 0, 5, 96, 115, 0, 5, 91, 115, 0, 245, 0, 20, 0, 20, 96, 75, 100, 20, 103, 20, 0, 85, 0, 20, 0, 20, 96, 75, 100, 20, 103, 20, 0, 85, 0, 20, 0, 20, 96, 75, 100, 20, 103, 20, 0, 85, 0, 20, 0, 20, 96, 75, 100, 20, 103, 20, 0, 85, 0, 20, 0, 20, 96, 75, 100, 20, 103, 20, 0, 85, 0, 20, 0, 20, 96, 75, 100, 20, 103, 20, 0, 85, 0, 20, 0, 20, 96, 75, 100, 20, 103, 20, 0, 85, 93, 235, 0, 5, 93, 475, 0, 5, 89, 475, 0, 5, 89, 235, 0, 5, 89, 475, 0, 5, 91, 235, 0, 5, 91, 235, 0, 5, 89, 475, 0, 5, 88, 235, 0, 5, 88, 235, 0, 5, 89, 235, 0, 5, 89, 235, 0, 5, 91, 235, 0, 5, 91, 235, 0, 5, 91, 475, 0, 5, 93, 475, 0, 5, 91, 1435, 0, 965, 93, 235, 0, 5, 93, 475, 0, 5, 89, 475, 0, 5, 89, 235, 0, 5, 89, 475, 0, 5, 91, 475, 0, 5, 89, 475, 0, 5, 88, 235, 0, 245, 89, 475, 0, 5, 91, 235, 0, 5, 91, 235, 0, 5, 91, 475, 0, 5, 91, 955, 0, 1925, 93, 235, 0, 5, 93, 475, 0, 5, 89, 475, 0, 5, 89, 235, 0, 5, 89, 475, 0, 5, 91, 235, 0, 5, 91, 235, 0, 5, 89, 475, 0, 5, 88, 475, 0, 5, 89, 235, 0, 5, 89, 235, 0, 5, 91, 235, 0, 5, 91, 235, 0, 5, 91, 475, 0, 5, 93, 475, 0, 5, 91, 1435, 0, 965, 86, 235, 0, 5, 86, 475, 0, 5, 88, 475, 0, 5, 88, 235, 0, 5, 89, 475, 0, 5, 94, 235, 0, 5, 94, 235, 0, 5, 93, 235, 0, 5, 93, 235, 0, 5, 89, 235, 0, 5, 89, 235, 0, 5, 91, 475, 0, 5, 91, 715, 0, 5, 89, 235, 0, 5, 89, 955, 0, 1925, 86, 475, 0, 5, 86, 475, 0, 5, 88, 475, 0, 5, 89, 475, 0, 5, 94, 715, 0, 245, 89, 475, 0, 5, 91, 475, 0, 5, 91, 715, 0, 5, 89, 1195, 0, 10 };
TmDeltaTime* pTdt;
TestMidiSong* pTestMidiSong;

void evIntM5(unsigned long _deltaTime){ //M5用
  if(M5.Btn.isPressed()){
    //ptr=0;
    pTestMidiSong->reset();
  }
  M5.update();
}
void evIntMidi(unsigned long _deltaTime){ //MIDI用
  if(userMode!=0)
    return;

  pTestMidiSong->update(_deltaTime);
}

void doBLEOne(uint8_t _value){
  if(_value==0){
      userMode = (userMode==0?1:0);
      setMidiData(0x81,pTestMidiSong->getPreviousNote());
      ledcWriteTone(LEDC_CHANNEL,0);
      updateLed();
  }
}
class MyBLECallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();
      if (value.length() > 0) {
        Serial.println("*********");
        Serial.print("New value: ");
        for (int i = 0; i < value.length(); i++){
          uint8_t val = (uint8_t)value[i]; 
          Serial.print(val);
          doBLEOne(val);
          Serial.print(",");
        }
        Serial.println("*********");
      }
    }
};

void setup()
{
  createTones();
  M5.begin(true, false, true);
  delay(10);

//---
  BLEDevice::init("M5ATOM0330MIDI");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
  );
  pCharacteristic->setCallbacks(new MyBLECallbacks());
  pCharacteristic->setValue("Hello World");
  pService->start();
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
//---

  M5.dis.drawpix(0, 0xf00000);
  Serial.begin(9600);
  Serial2.begin(31250,SERIAL_8N1,19,22);

  ledcSetup(LEDC_CHANNEL,12000, 8);
  ledcAttachPin(TLED,1);
  userMode=0;

  pTestMidiSong = new TestMidiSong();
  pTestMidiSong->prepare(tones_i,mididata,(sizeof(mididata)>>1));

  pTdt= new TmDeltaTime(); // newが嫌なら最初から実体化させておく
  pTdt->Setup();
  pTdt->AddTrig(evIntMidi,5); // 2msecごとにevIntMidi()を呼ぶ
  pTdt->AddTrig(evIntM5,33); // 10msecごとにevIntM5()を呼ぶ
}

void loop()
{
  pTdt->Update(); // loop内で毎回呼ぶ
  delay(1); // 一定かつ適度な間隔で回す
}

