/*
 * THIS IS DEMO FOR SENSOR IN IOT PROJECT
 */
#include <Bounce2.h>

const unsigned int LED_PIN[3] = {10,11,12};
const unsigned int BUTTON[3] = {5,6,7};

const unsigned int DEBOUNCE_DELAY = 20;
Bounce buttons[3];

// prepare buffer
byte rxBuff[10];

// step
byte curStep = 0;

// count
byte count = 0;

// flags
bool flagStart = false;

// status of controllers
byte ctrlStatus[3];

void handleMessage() {
  if(rxBuff[0]==0x0B) {
    float val;
    byte sta;
    if(rxBuff[1]==0x01) {
      // temperature of soil
      val = 23.1+random(100)*0.1-5.0;
    }
    else if(rxBuff[1]==0x02) {
      // humidity of soil
      val = 12.2+random(100)*0.1-5.0;
    }
    else if(rxBuff[1]==0x03) {
      // temperature of air
      val = 25.0+random(100)*0.1-5.0;
    }
    else if(rxBuff[1]==0x04) {
      // humidity of air
      val = 86.8+random(100)*0.1-5.0;
    }
    else if(rxBuff[1]==0x05) {
      // sun light strenth
      val = 10365.0+random(10000)*0.1-500.0;
    }
    else if(rxBuff[1]==0x06) {
      // CO2
      val = 418.0+random(500)*0.1-25.0;
    }
    else if(rxBuff[1]>=0x11 && rxBuff[1]<=0x13) {
      //controllers
      sta = ctrlStatus[rxBuff[1]-0x11];
    }
    else {
      return;
    }
    //send back
    Serial.write((byte)0x5A);
    Serial.write((byte)0x5A);
    Serial.write((byte)0xB0);
    Serial.write(rxBuff[1]);
    byte cs = 0;
    if(rxBuff[1]<=6) {
      //sensors
      Serial.write((byte)0x01); cs = cs+0x01;
      byte *pVal = (byte*)&val;
      for(int i=0;i<4;i++) {
        Serial.write(pVal[i]);
        cs = cs+pVal[i];
      }
      Serial.write((byte)0); Serial.write((byte)0);
    }
    else {
      //controllers
      Serial.write(sta); cs = cs+sta;
      Serial.write((byte)0); Serial.write((byte)0);
      Serial.write((byte)0); Serial.write((byte)0);
      Serial.write((byte)0); Serial.write((byte)0);
    }
    cs = 0-cs;
    Serial.write(cs);
  }
  else if(rxBuff[0]==0x0A) {
    if(rxBuff[1]>=0x11 && rxBuff[1]<=0x13) {
      //controllers
      int index = rxBuff[1]-0x11;
      if((rxBuff[2]==1) || (rxBuff[2]==2)) {
        ctrlStatus[index] = rxBuff[2];//record
        //echo
        Serial.write((byte)0x5A);
        Serial.write((byte)0x5A);
        Serial.write((byte)0xA0);
        Serial.write(rxBuff[1]);
        byte cs = 0;
        Serial.write(ctrlStatus[index]); cs += ctrlStatus[index];
        Serial.write((byte)0); Serial.write((byte)0);
        Serial.write((byte)0); Serial.write((byte)0);
        Serial.write((byte)1); Serial.write((byte)0); cs += 1;
        cs = 0-cs;
        Serial.write(cs);
      }
    }
  }
}

void setup() {
  // setup uart
  Serial.begin(19200,SERIAL_8N1);
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  // initialize step
  curStep = 0;
  // initialize count
  count = 0;
  // initialize flag
  flagStart = false;
  // initialize controller status
  ctrlStatus[0] = 2;
  ctrlStatus[1] = 2;
  ctrlStatus[2] = 2;
  // initialize LEDs
  for(int i=0;i<3;i++) {
    pinMode(LED_PIN[i],OUTPUT);
    digitalWrite(LED_PIN[i],LOW);
  }
  //initialize buttons
  for(int i=0;i<3;i++) {
    buttons[i].attach(BUTTON[i],INPUT);
    buttons[i].interval(DEBOUNCE_DELAY);
  }
}

void loop() {
  //check buttons
  for(int i=0;i<3;i++) {
    if(buttons[i].update() && buttons[i].read()==HIGH) {
      if(ctrlStatus[i]==1) {
        ctrlStatus[i] = 2;
      }
      else {
        ctrlStatus[i] = 1;
      }
    }
  }
  //read serial commands
  byte charIn;
  while(Serial.available()) {
    charIn = Serial.read();//read data
    if(curStep==0) {
      //header1
      if(charIn==0x5a) {
        flagStart = true;
        curStep = 1;
      }
    }
    else if(curStep==1) {
      //header2
      if(charIn==0x5a) {
        curStep = 2;
      }
      else {
        curStep = 0;
      }
    }
    else if((curStep>=2) && (curStep<11)) {
      //data
      rxBuff[curStep-2] = charIn;
      curStep++;
    }
    else if(curStep==11) {
      //check sum
      rxBuff[9] = charIn;
      byte cs=0;
      for(int i=0;i<8;i++) {
        cs += rxBuff[2+i];
      }
      if(cs==0) {
        //success
        handleMessage();
      }
      curStep = 0;
    }
    else {
      curStep = 0;
    }
    //send back before start
    if(!flagStart) {
      Serial.write(charIn);
    }
  }
  //show controller status
  for(int i=0;i<3;i++) {
    if(ctrlStatus[i]==1) {
      digitalWrite(LED_PIN[i],HIGH);
    }
    else {
      digitalWrite(LED_PIN[i],LOW);
    }
  }
  //heartbeat
  if(count==0) {
    digitalWrite(LED_BUILTIN, HIGH);
  }
  else if(count==100) {
    digitalWrite(LED_BUILTIN, LOW);
  }
  count++;
  if(count>=200) {
    count = 0;
  }
  delay(10);
}
