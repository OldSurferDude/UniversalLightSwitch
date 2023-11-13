#define VER "07"
#define PROGRAM_NAME "XPTCalibrate"

const char* ssid = "ULS-AP"; const char* password = "ULS-password";
#define MQTT_BROKER_IP_ADDRESS "192.168.1.180"

// revision history
  /*
  ver 07
    changed libraries
      EspMQTTClient.h
      SPI.h
      Ucglib.h
      XPT2046.h
  ver 06
    Display on display current contents
      hex bytes and decimal
  ver 05
    got fubarated
  ver 04
    save data to EEPROM
  ver 03
    include OTA
    which requires a timeout for touching the screen so OTA can happen.
  ver 02
  Working ... when properly wired
  Added compiler directive to remove touch
  Added compiler directive to remove display
  ver 01
  nothing on the display
  nothing printed
  * Copyright (c) 2015-2016  Spiros Papadimitriou
  *
  * This file is part of github.com/spapadim/XPT2046 and is released
  * under the MIT License: https://opensource.org/licenses/MIT
  *
  * This software is distributed on an "AS IS" basis,
  * WITHOUT WARRANTY OF ANY KIND, either express or implied.
  */

// defines and includes
  //#include <Arduino.h>
  #include <ESP_EEPROM.h>
  uint16_t touchCalibration[4];
  #include <SPI.h>
  #include <EspMQTTClient.h>
String hostname=WiFi.getHostname();
String localIP;

EspMQTTClient client(
  ssid,                   // WiFi
  password,               // WiFi
  MQTT_BROKER_IP_ADDRESS,  // MQTT Broker server ip
  hostname.c_str()     // Client name that uniquely identify your device
);
  #include <Ucglib.h>
  #define MAX_ROW 159
  #define MAX_COLUMN 127
  #define CHARACTER_ROWS 20
  char dispbuf[CHARACTER_ROWS+1];

  #include <XPT2046.h>



  // Modify the following two lines to match your hardware
  Ucglib_ST7735_18x128x160_HWSPI ucg(/*cd=*/ 4 , /*cs=*/ 15, /*reset=*/ -1);
  XPT2046 touch(/*cs=*/ 16, /*irq=*/ 5);

  #define BACKLIGHT_PIN 2
  bool BacklightStatus = true;  // when output is high, backlight is on 

  char buf[17];
  bool runCalibaration = false;


//------------------------------------------------------displayContents
void displayContents(){
  uint16_t content[4];
  EEPROM.get(0,content[0]);
  EEPROM.get(2,content[1]);
  EEPROM.get(4,content[2]);
  EEPROM.get(6,content[3]);

  uint8_t bcontent[8];
  EEPROM.get(0,bcontent[0]);
  EEPROM.get(1,bcontent[1]);
  EEPROM.get(2,bcontent[2]);
  EEPROM.get(3,bcontent[3]);
  EEPROM.get(4,bcontent[4]);
  EEPROM.get(5,bcontent[5]);
  EEPROM.get(6,bcontent[6]);
  EEPROM.get(7,bcontent[7]);

  snprintf(buf,sizeof(buf),"%4d %4d",content[0],content[1]);
  ucg.setPrintPos(5, 100);
  ucg.print(buf);
  snprintf(buf,sizeof(buf),"%02X%02X %02X%02X",bcontent[1],bcontent[0],bcontent[3],bcontent[2]);
  ucg.setPrintPos(5, 110);
  ucg.print(buf);


  snprintf(buf,sizeof(buf),"%d %d",content[2],content[3]);
  ucg.setPrintPos(5, 120);
  ucg.print(buf);
  snprintf(buf,sizeof(buf),"%02X%02X %02X%02X",bcontent[5],bcontent[4],bcontent[7],bcontent[6]);
  ucg.setPrintPos(5, 130);
  ucg.print(buf);


}
bool calibrated = false;
bool calibrateIt = false;
//------------------------------------------------------calibratePoint
static void calibratePoint(uint16_t x, uint16_t y, uint16_t &vi, uint16_t &vj) {
  // Draw cross
  ucg.setColor(0x00, 0xff, 0x00);
  ucg.drawHLine(x - 8, y, 16);
  ucg.drawVLine(x, y - 8, 16);

  while (!touch.isTouching()) {
    delay(10);
    client.loop();  // see if there is an OTA request
  }
  touch.getRaw(vi, vj);
// Erase by overwriting with black
  ucg.setColor(0, 0, 0);
  ucg.drawHLine(x - 8, y, 16);
  ucg.drawVLine(x, y - 8, 16);
}
bool thanked = true;
//------------------------------------------------------checkTouch
void checkTouch(){
  uint16_t x,y;
    if (touch.isTouching()){
      if (calibrated){
        ucg.setColor(0,0,0);
        ucg.drawBox(0,24,127,159);
        ucg.drawBox(0,0,127,9);
        ucg.setPrintPos(3,9);
        ucg.setColor(255, 255, 255);
        ucg.print("CALIBRATION");
          
        calibrated = false;
      }
      touch.getPosition(x, y); 
      Serial.print(x);Serial.print(' ');Serial.println(y);
      if (y < 24) {
        calibrateIt = true;
        ucg.setColor(0,0,0);
        ucg.drawBox(0,0,127,24);
        ucg.setColor(255,0,0);
        ucg.setPrintPos(3,9);
        ucg.print("Calibrating");
        client.loop();
        delay(5000);
        client.loop();
      }
        
      if (!thanked){
        ucg.setColor(0,0,0);
        ucg.drawBox(3,142,75,150);
        ucg.setColor(0,255,0);
        ucg.setPrintPos(3,150);
        ucg.print("Thank you");    
        thanked = true;
      }
    }
    else{
      calibrateIt = false;
      if(thanked){
        ucg.setColor(0,0,0);
        ucg.drawBox(3,142,75,150);
        ucg.setColor(255,0,0);
        ucg.setPrintPos(3,150);
        ucg.print("Touch me!");    
        thanked = false;
      }
    }
}
//------------------------------------------------------calibrate
void calibrate() {
  ucg.setColor(0xff, 0x00, 0x00);
  displayContents();

  uint16_t x1, y1, x2, y2;
  uint16_t vi1, vj1, vi2, vj2;

  touch.getCalibrationPoints(x1, y1, x2, y2);

  calibratePoint(x1, y1, vi1, vj1);
  delay(500);
  client.loop();
  calibratePoint(x2, y2, vi2, vj2);

  touch.setCalibration(vi1, vj1, vi2, vj2);

  // store in buffer for EEPROM
  EEPROM.put(0,vi1);
  EEPROM.put(2,vj1);
  EEPROM.put(4,vi2);
  EEPROM.put(6,vj2);

  // write the buffer to EEPROM
  String commitResult;
  if (EEPROM.commit()) commitResult = "success"; else commitResult = "fail";
  Serial.printf("Commit %s\r\n",commitResult);

  Serial.printf("%d,%d,%d,%d\r\n", (int)vi1, (int)vj1, (int)vi2, (int)vj2);

  ucg.setColor(0,0,0);
  ucg.drawBox(0,67,127,125);
  ucg.setColor(0xff, 0xff, 0xff);
  ucg.setPrintPos(3, 75);
  ucg.print("Cal params:");
  snprintf(buf, sizeof(buf), "%d,%d", (int)vi1, (int)vj1);
  ucg.setPrintPos(3, 100);
  ucg.print(buf);
  snprintf(buf, sizeof(buf), "%d,%d", (int)vi2, (int)vj2);
  ucg.setPrintPos(3, 125);
  ucg.print(buf);
  snprintf(buf, sizeof(buf), "Commit %s",commitResult);
  ucg.setPrintPos(3, 32);
  ucg.print(buf);
  
  calibrated = true;
  client.loop();
  ucg.setColor(0,0,0);
  ucg.drawBox(0,0,127,9);
  ucg.setPrintPos(3,9);
  ucg.setColor(0, 255, 0);
  ucg.print("CALIBRATED!");

  delay(1000);
  client.loop();

  calibrateIt = false;

}
//------------------------------------------------------setup
void setup() {
  Serial.begin(115200);
  Serial.printf("\r\n%s version %d\r\n",PROGRAM_NAME,VER);

  pinMode(BACKLIGHT_PIN, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  analogWriteRange(100);
  analogWrite(BACKLIGHT_PIN,BacklightStatus);


  EEPROM.begin(8); // calibration has 4 points each of 2 bytes

  // initialize graphics driver
  ucg.begin(UCG_FONT_MODE_TRANSPARENT);

// inititalize touch driver
  touch.begin(ucg.getWidth(), ucg.getHeight());
//  touch.begin(128, 160);

// Turn on backlight
  pinMode(BACKLIGHT_PIN, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  digitalWrite(BACKLIGHT_PIN,BacklightStatus);

// blank screen
  ucg.clearScreen();
  ucg.setFont(ucg_font_amstrad_cpc_8r);

  ucg.setPrintPos(3,9);
  ucg.setColor(255, 255, 255);
  ucg.print("CALIBRATION");


// run calibration
//  calibrate();  // No rotation!!

  // Optional functionalities of EspMQTTClient
  client.enableDebuggingMessages(); // Enable debugging messages sent to serial output
  //client.enableHTTPWebUpdater(); // Enable the web updater. User and password default to values of MQTTUsername and MQTTPassword. These can be overridded with enableHTTPWebUpdater("user", "password").
  client.enableOTA(); // Enable OTA (Over The Air) updates. Password defaults to MQTTPassword. Port is the default OTA port. Can be overridden with enableOTA("password", port).
  //client.enableLastWillMessage(hostTopic.c_str(), "offline",true);  // You can activate the retain flag by setting the third parameter to true
}
//------------------------------------------------------onConnectionEstablished
// This function is called once everything is connected (Wifi and MQTT)
// WARNING : YOU MUST IMPLEMENT IT IF YOU USE EspMQTTClient
void onConnectionEstablished(){
  ucg.setColor(0,255,255);
  ucg.setPrintPos(3,16);
  ucg.print("Touch screen top");
  ucg.setPrintPos(3,24);
  ucg.print("   to start   ");
  //         0123456789ABCDEF

}

unsigned long checkLoopStart = millis()-500;  // to prevent checking MQTT too often
//------------------------------------------------------loop
void loop(){

  if (millis()-checkLoopStart > 500){ // every second check loop
    client.loop(); // MQTT keep alive, send data, receive data

    checkLoopStart = millis();
    if (client.isConnected()) {
      checkTouch();
      if (calibrateIt) calibrate();
    }
    else{
      Serial.print('.');
    }

  }
}
