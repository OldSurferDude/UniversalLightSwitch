#define VER "49"
#define PROGRAM_NAME "Universal Light Switch"

#define DEBUG

const char* ssid = "ULS-AP"; const char* password = "ULS-password";
#define MQTT_BROKER_IP_ADDRESS "192.168.2.180"
/*

  The UniversalLightSwitch
  ------------------------
  The first intent is to be low cost, DIY, and open source
  Features:
    Able to control any device's ONoff
      Future: variable from fully off to fully on
              Color control, as in RGB light strip
    Able to change which device or devices are controlled by the key
    Labels on the buttons that can be changed (including color of key)
    API is through an MQTT broker
    OTA updateable
  The heavy lifting will be done by a home automation program
  Companion prduct actually performs the action
    eg ESP-01, Tasmotized Snooff D1 or Basic R3, power supply, universal power supply, connector to "mains"
      "mains" means 80VAC - 260AC 50-60hz

  ver "49"
    sometimes the key label is not presented.  This seems to be after a reset.
      I suspect that the broker doesn't know that this is a resubscribe.
      Therefore I'll do an unsubscribe before the initial subscribe.
    Also, I obsvered a race condition on all devices powering up simultaneously
      One switch would turn on, which would cause the other switch to turn on,
        but the other switch turned of...back and forth
    using key 2 for debug messages
  ver "48"
    changed broker to 192.168.2.180
  ver "47"
    turned off debug for the last compile but it seems that 
      enabling the EspMQTTClient library function to enable printing of MQTT traffic
      creates an optimal timing environment.  Without it, at least one device crashes.
    Don't want to mess up the tweaks done in 46 (turned off debug for the last compile)
  ver "46"
    ver 45 is working quite well.  Created this in the event that some tweaking needs to be done
    Found a couple of publish statements that were not set to "retain".
      This flag keeps the data on the MQTT broker and is restored on reboot
  ver "45"
    44 seems to be working OK
      OK it takes a long time to get attributes
        added delays, 
          1 second before client.loop()
          0.1 second after client.loop which is just before subscribe
          1 second after subscribe
      OK An activity key's display seems to get loaded twice
          fix to only print type 'p'
      
    To be done:
      OK remove all unecessary print statements
      OK investigate key loaded twice
  ver "44"
    To be done:
      OK get the menu keys to respond as menu keys, that is, load the panel to which they are associated
      OK prevent the non-assigned keys from responding.
  ver "43"
    42 seems to working.  Touch screen, when touched, responds as expected
      "client.loop()"  was put in a few places to ensure the network is updated
    to be done:
      OK set the [RESET] back to actually reseting the device
      o get the menu keys to respond as menu keys, that is, load the panel to which they are associated
      o prevent the non-assigned keys from responding.
      OK clean up boot
      OK put version in upper right
  ver "42"
    caused by open subscription? still fails
    added   EEPROM.begin(8); which seems to be missing.  Is it redundant?

  ver "41"
    oops!  touching touch screen resets device
    things to be done for 41:
      when an activity comes in, the key on the diesplay needs to be updated
      In fact all the keys need to be updated
      The action for each key needs to be linked in.
    40 is now working
      A good WiFi Connection is important!  This could be problematic
      Each subscription must have its own topic, undisturbed until it is unsubscribed
        Apparently only a pointer is passed to the MQTT routine (makes sense, keep memor usage low)
        Thus, for each subscription there must be reserved a space for the topic
        Each key will could have a subscription so that memory would be in the keys struct
        clean up start up display
  ver "40"
    32 compiled and started but as soon as the touch screen was touched, it rebooted.  I suspect a memory problem
    This version is an attempt to minimize memory usage 
      at the cost of longer response time in loading a new panel
      This version will populate the key structure in the callback routine
    I am not confident with the JSON library.  I will try it for this version
      The problem is the program crashes with a malformed JSON
        resolving that problem may take as much coding as parsing less automatically.
    MQTT tropic structure:
    ULS
      /hhhhhh = online/offline device identifier
        /panel = current panel loaded into ULS
      /activity
        /n (activity index) = 0-100 state of the activity 
          /label = JSON of attributes for key {"L":["+----+","|    |","KITCHN","|    |","|    |","+----+"],"C":[0,255,0]}
                                                        ^        ^        ^        ^        ^        ^          ^  ^  ^
            Label for key (must have 6 lines!) ---------+--------+--------+--------+--------+--------=          |  |  |
            Color for key label RGB  ---------------------------------------------------------------------------+--+--+
            If this format is not strictly followed, the program will crash!
      /menu
        /n (menu index) = JSON of attributes for key {"N":1,"L":["+----+","|    |","KTCHEN","|    |","|    |","+----+"],"C":[0,255,0]}
                                                          ^          ^        ^        ^        ^        ^        ^          ^  ^  ^
                          index of next panel -------------+         |        |        |        |        |        |          R  G  B                                            
                          Label for key (must have 6 lines!) --------+--------+--------+--------+--------+--------+          |  |  |
                          Color for key label RGB  --------------------------------------------------------------------------+--+--+
                          If this format is not strictly followed, the program will crash!
      /panel
        /n (panel index) = {{"Type": ["a","a","a","a","a","a"],"Index": [0,0,0,0,0,0,0]} 
        Types are: n (none) | a (activity) | p (panel)
      Index is to the activity or panel to be associated with the key
      In the case of the panel this value will change and become the index to the next panel
      Duplicate index numbers are not allowed, behavoir is unpredictable
  ver "32"
    31 appears to be working, don't want mess it up
    Need to review what happens when a key gets touched.
    MQTT enableDebugMessages is enabled.  This may help with timing
      because it was seen that it was restarting about 4 seconds into the loop
  ver 31
    Universal Light Switch v26 merged into SimpleMQTTClient v10
    A whole bunch of gotchas
    MQTT tropic structure:
    ULS
      /hhhhhh = online/offine device identifier
        /panel = current panel loaded into ULS
      /activity
        /n = 0-100 state of the activity 
      /panel
       /n = 0-254 panel number
        /n = 0-5 key number
          /attr = JSON of attributes for key {"T":"a","I":0,"L":["+----+","|    |","KITCHN","|    |","|    |","+----+"],"C":[0,255,0]}
                                                   ^      ^          ^        ^        ^        ^        ^        ^          ^  ^  ^
            Type a Activty, p Panel ---------------+      |          0        1        2        3        4        5          R  G  B
            Index Associated activity or next panel ------+          |        |        |        |        |        |          |  |  |
            Label for key (must have 6 lines!) ----------------------+--------+--------+--------+--------+--------=          |  |  |
            Color for key lable RGB  ----------------------------------------------------------------------------------------+--+--+
            If this format is not strictly followed, the program will crash!
  versions below refer to SimpleMQTTClient
  ver 10
    Clear the key data and from display
    09 compiled JSON decoding sort of verified in an external program
  ver 09
    JSON decoding of key attributes
    08 compiled
  ver 08
    each key enabled will have a subscription for
      handling activity status changes
      handling attribute changes
    upon panel change each key will be 
      blanked
      unsubscribed from both attributes and, if an activity, activity
      subscribed to the topic the is the attribute for that key and panel
      When(if) the attribute comes back in the main loop,
        if the type is Activity, the key will be subscribed to the activity status
    
  ver 07
    compiles OK but not tested
    It will receive data to the appropriate routine
  ver 06
    Since I can get the key number and current index number from the topic 
      these will be removed from the key structure
      The index number in the key structure is that of the operation the key will perform
  ver 05
    get string number from topic
  ver 04
    Decode JSON
    Create structrue for keys
  
  Ver 03
    essentially reverted to 01
    subscribed to ULS/panel/0/attr
    The JSON for all the buttons will be too long. Created a description for each key 
    
    (key is now used instead of key)
  Ver 02
    two commands to subscribe to the same topic
    set Last Will as retain
  Ver 01
    modified enough so that it should work in my environment
  SimpleMQTTClient.ino
  The purpose of this exemple is to illustrate a simple handling of MQTT and Wifi connection.
  Once it connects successfully to a Wifi network and a MQTT broker, it subscribe to a topic and send a message to it.
  It will also send a message delayed 5 seconds later.
*/


bool booting = true;

#include "EspMQTTClient.h"
// capture the unique hostname for this device
String hostname=WiFi.getHostname();

// configure the MQTT client with predfined variables.
EspMQTTClient client(
  ssid,                   // WiFi
  password,               // WiFi
  MQTT_BROKER_IP_ADDRESS,  // MQTT Broker server ip
  hostname.c_str()     // Client name that uniquely identify this device
);

String hostTopic = "ULS/000000";  // predefine to prevent buffer overrun
String ClientPanelTopic = "ULS/000000/panel/000"; // predefine to prevent buffer overrun
String GeneralPanelTopic = "ULS/panel/000";
String versionTopic = "ULS/000000/version";


#include <ESP_EEPROM.h>

#include <SPI.h>
#include <Ucglib.h>  // Required
#include <XPT2046.h>

// Display
// library initialization of connection of the ESP pins 
//   to the pins of the display portion of the display
// the SPI hardware pins are defaulted: MOSI=13,SCLK=14,MISO=12
Ucglib_ST7735_18x128x160_HWSPI ucg( 4, 15, -1);  //cd,cs,reset

#define MAX_ROW 159
#define MAX_COLUMN 127 // pixels per row  less 1 (0 is the first)
#define CHARACTER_ROWS 20 // Characters are 8x8 pixels so the display is 20 high
char dispbuf[CHARACTER_ROWS+1]; // used to transfer characters to the display
// predefined colors for this program
#define xWHITE 0xff,0xff,0xff
#define xRED 0xff,0,0
#define xGREEN 0,0xff,0
#define xBLUE 0,0,0xff
#define xBLACK 0,0,0
#define xLIGHTGREEN 0x0f,0x7f,0x0f

#define PAD_SIZE 50 // touch areas for keys 50x50 pixels
// X-Y location of upper left corner of upper left key
#define XUL 14
#define YUL 5 
// X-Y location of upper left corner of zone for reset touch
#define RESET_ZONE_ULx XUL+2*PAD_SIZE  // 
#define RESET_ZONE_ULy YUL+2*PAD_SIZE+PAD_SIZE/4
// 
#define LabelLineLengthMax 6  // Label lines longer than this will be truncated
#define LabelLines 6 // Label lines in excess of this will be ignored

// descritption aids in the code
#define Rgb 0
#define rGb 1
#define rgB 2

#define UL 0
#define UR 1
#define ML 2
#define MR 3
#define LL 4
#define LR 5
char keyName[6][3] = {"UL","UR","ML","MR","LL","LR"};

// Touch screen
// library initialization of connection of the ESP pins 
//   to the pins of the touch portion of the display
XPT2046 touch(16, 5); // cs, irq
uint16_t touchCalibration[4]; // Calibration constants
// this values must persit between calls to the touch capture routine
uint16_t x, y; 
uint32_t xSum,ySum,xyCount;
bool displayTouched = false;
bool touchStarted = false;
unsigned long touchTimerStart;
bool resetZoneTouched = false;

 // this sets the time, in milliseconds, that a touch must occur to be acknowldged as a touch
#define MINIMUM_TOUCH_TIME 100


//backlight
#define BACKLIGHT_PIN 2
#define BACKLIGHT_TIMEOUT 300000 // 180 Seconds = 3 minutes
unsigned long BacklightTimeoutStart = millis();
uint8_t BacklightStatus = 100;  // when output is high, backlight is on 
uint8_t BacklightOnState = BacklightStatus;

// used to sync up with MQTT broker
// and avoid loading a panel if it is already loaded
volatile uint8_t currentPanel = 255;
volatile bool incomingPanelAvailable = false;
// used to acknowledge in the callback routine (ISR) that panel data has arrived from the MQTT broker
volatile bool dataToBeProcessedForPanel = false;

// When the panel will be changed, the topic for the new panel is subscribed to.
// The program, in the background, will wait for the data in the topic to arrive
// This variable is used to indicate that the data has arrived
// so that it can be processed in more detail when timme is less critical 
volatile bool panelDataAcquired = false;

volatile uint8_t globalKey = 255;  //used to pass the key number to a callback routine

// The key that will require the activity data will be unkown when activity data 
// arrives in the callback routine.  The activity is last part of the topic and can be extracted
// Using this tabel, the key is associatee with the activity
// The table is populated at the time the topic is subscribed to and unpopulatted when
// the activity is unscribed from
// This technique avoids the requirement that each key have a callback routine for the activity 
// with which it is associated.  Note that this means that each activity associated with a panel 
// must be unique to that panel.
uint8_t activityIndexToKey[256];

// The algorithm to acquire the key attributes (label and color) is:
//   subscribe to the topic that has the label of the menu or activeity data
//   wait for the data
//   capture the date
//   unsubscribe from the topic
// Subscribing requires a callback routine (ISR) which receives the data and operates in the background so
// it is unknown when the data will arrive.  When the call backroutine captures the data and then indicate,
// via this variable, that the data has arrived.  The "wait" section of the calling routine will be waiting for
// the variable to change state.
volatile bool keyUpdateInProgress = false;


//------------------------------------------------------
//------------------------------------------------------
typedef struct {
  uint8_t index = 255;  // pointer to the next panel or the current activity, 255 means not defined
  char type = 'n'; // panel | activity, null means not defined
  String activityTopic = "ULS/activity/255";  // used in minimizing time in callback
  uint8_t state = 255; // state of activity associated with key, 255 means not defined
  uint8_t incomingState = 255;  // used when an activity's status changes
  bool incomingStateChange = false;  // indicates that a state change is available to be processed in the non-ISR code
  uint8_t onState = 100;  // this will change if there is a dimmer involved, This will be the state to set the dimmer when the activity is turned on
  char label[6][7] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}; // allocate space
  int color[3] = {255,255,255};  // RGB color of label
} keyAttrDef;
keyAttrDef keys[6];  // indexed by key

//------------------------------------------------------
//------------------------------------------------------

//------------------------------------------------------clearKeyAttributes
void clearKeyAttributes(uint8_t key){
                           keys[key].index = 255;
                           keys[key].type='n';
                           keys[key].activityTopic="ULS/activity/255";
                           keys[key].state=255;
                           keys[key].incomingState=255;
                           keys[key].incomingStateChange=false;
                           keys[key].onState=100;
  for (uint8_t l=0;l<6;l++)keys[key].label[l][0]='\0';
  for (uint8_t l=0;l<3;l++)keys[key].color[l] = 0;
}
//------------------------------------------------------printKey
void printKey(uint8_t key){
#ifdef DEBUG
                            Serial.print(' ');Serial.print(keys[key].index);
                            Serial.print(' ');Serial.print(keys[key].type);
                            Serial.print(' ');Serial.print(keys[key].activityTopic);
                            Serial.print(' ');Serial.print(keys[key].state);
                            Serial.print(' ');Serial.print(keys[key].incomingState);
                            Serial.print(' ');Serial.print(keys[key].incomingStateChange);
                            Serial.print(' ');Serial.print(keys[key].onState);
  for (uint8_t l=0;l<6;l++){Serial.print(' ');Serial.print(keys[key].label[l]);}
  for (uint8_t l=0;l<3;l++){Serial.print(' ');Serial.print(keys[key].color[l]);}
  Serial.println();
#endif
}
//------------------------------------------------------displayRESETzone
void displayRESETzone(bool ShowClear){
  if (ShowClear){
    resetZoneTouched = true;
    // put red framee around Reset
    ucg.setColor(0xff,0,0);
    ucg.drawFrame(RESET_ZONE_ULx,RESET_ZONE_ULy,MAX_COLUMN-1,MAX_ROW-1);
    // set position of text
    ucg.setPrintPos(RESET_ZONE_ULx+1,RESET_ZONE_ULy+1);
    // rotate text
    ucg.setPrintDir(1);
    // print text
    ucg.print("RESET");
    // unrotate text
    ucg.setPrintDir(0);
  }
  else {
    resetZoneTouched = false;
    ucg.setColor(xBLACK);
    ucg.drawBox(RESET_ZONE_ULx,RESET_ZONE_ULy,MAX_COLUMN,MAX_ROW);
  }
}
//------------------------------------------------------displayInAZone
void displayInAZone(uint8_t zone){
  // dispbuf already populated
  
  uint8_t bULx,bULy,bLRx,bLRy,px,py;
  // set zone
  switch (zone) {
    case UL : bULx=0  ;bULy=0  ;bLRx=13 ;bLRy=119;px=2  ;py=2  ;break;
    case UR : bULx=113;bULy=0  ;bLRx=127;bLRy=119;px=115;py=2  ;break;
    case LL : bULx=0  ;bULy=120;bLRx=13 ;bLRy=159;px=2  ;py=120;break;
    case LR : bULx=113;bULy=120;bLRx=127;bLRy=159;px=115;py=120;break;
  }  
  ucg.setColor(xBLACK);
  ucg.drawBox(bULx,bULy,bLRx,bLRy);
  ucg.setColor(0,0,0xff);
  ucg.setPrintPos(px,py);
  ucg.setPrintDir(1);
  ucg.print(dispbuf);
  ucg.setPrintDir(0);
}
//------------------------------------------------------sendStatus
void sendStatus(uint8_t key){
  String StatusTopic = "ULS/activity/"+String(keys[key].index);
  client.publish(StatusTopic, String(keys[key].state).c_str(),true);  // send current status
}
//--------------------------------------------------------lightUpKey
void lightUpKey(uint8_t key, bool ONoff){
  uint16_t xUL = XUL+((key)%2)*PAD_SIZE;
  uint16_t yUL = YUL+((key)/2)*PAD_SIZE;
  if (ONoff){
              // set compliment
          //ucg.setColor(0,Buttons[key].color[Rgb]^0xff,Buttons[key].color[rGb]^0xff,Buttons[key].color[rgB]^0xff);
          ucg.setColor(0,xBLACK);
          ucg.drawBox(xUL,yUL,PAD_SIZE,PAD_SIZE);
          ucg.setColor(0,keys[key].color[Rgb],
                      keys[key].color[rGb],
                      keys[key].color[rgB]);
          ucg.drawBox(xUL+1,yUL+1,PAD_SIZE-2,PAD_SIZE-2);
          // set compliment
          //ucg.setColor(0,Buttons[key].color[Rgb]^0xff,Buttons[key].color[rGb]^0xff,Buttons[key].color[rgB]^0xff);
          ucg.setColor(0,xBLACK);
  }
  else { // paint the key for the OFF state
          ucg.setColor(0,keys[key].color[Rgb],
                      keys[key].color[rGb],
                      keys[key].color[rgB]);
          ucg.drawBox(xUL,yUL,PAD_SIZE,PAD_SIZE);
          // set compliment
          //ucg.setColor(0,Buttons[key].color[Rgb]^0xff,Buttons[key].color[rGb]^0xff,Buttons[key].color[rgB]^0xff);
          ucg.setColor(0,xBLACK);
          ucg.drawBox(xUL+1,yUL+1,PAD_SIZE-2,PAD_SIZE-2);
          ucg.setColor(0,keys[key].color[Rgb],
                      keys[key].color[rGb],
                      keys[key].color[rgB]);
  }  
    // position the cursor and print the label of the key
    for (uint8_t line=0;line<6;line++){
      ucg.setPrintPos(xUL+2,yUL+10+line*8);
      ucg.print(keys[key].label[line]);
    }
}

//--------------------------------------------------------actOnKeyPressed
void actOnKeyPressed(uint8_t key, bool updateStatusOnMQTT){
  switch(keys[key].type) {
    case 'a' : 
      lightUpKey(key, (keys[key].state>0 && keys[key].state<=100));
      if (updateStatusOnMQTT) sendStatus(key);
      break;
    
    case 'p' : 
    // trigger the events to change panels
#ifdef DEBUG
      Serial.printf("Changing panel from %d to %d\r\n",currentPanel, keys[key].index);      
#endif
      client.publish(ClientPanelTopic,String(keys[key].index),true);
      break;
    
    default : break;
  }

}
//------------------------------------------------------debugMessage
void debugMessage(char debugLabel[],uint8_t debugKey){
  keys[debugKey].color[0] = 255;
  keys[debugKey].color[1] = 0;
  keys[debugKey].color[2] = 0;
  keys[debugKey].state = 0;
  keys[debugKey].type = 'a';
  uint8_t cl = strlen(debugLabel);
  if (cl>36) cl = 36;
  uint8_t clptr = 0;
  while (clptr < cl)
    for (uint8_t line=0;line<6;line++){
      for(uint8_t c=0;c<6;c++)
        if ( debugLabel[clptr] == '\0')
          keys[debugKey].label[line][c] = '\0';
        else
          keys[debugKey].label[line][c] = debugLabel[clptr++];

      keys[debugKey].label[line][6] = '\0';
    }
  lightUpKey(debugKey,false);
}
//--------------------------------------------------------checkTouch
void checkTouch() {
  int key;
  if (touch.isTouching()){
    // if the backlight is off, turn it on
    if (BacklightStatus != BacklightOnState){ 
      BacklightStatus = BacklightOnState;
      analogWrite(BACKLIGHT_PIN,BacklightStatus);
    }
    else{ // the initial touch when dark is ignored
    }
    BacklightTimeoutStart = millis();  // restart backlight timeout timer

    //
    if (touchStarted){  // Wait minimum touch time before acknowledging touch
      if (!displayTouched){
        touch.getPosition(x, y);
        xSum += x;
        ySum += y;
        xyCount++;
        if (millis()-touchTimerStart > MINIMUM_TOUCH_TIME) {
          displayTouched = true; // indicate that the screen was touched
          // visiual feedback
          ucg.setColor(0,0,255,0);
          ucg.drawFrame(2,1,MAX_COLUMN-1,MAX_ROW);  // turn on green frame indicating touched
        }
      }
      else {
        
        // someplace here in the code is where increase or decrease status because of HOLD will occur
        // change of status will have to be sent every 0.500? seconds
        // 10%? change every 1/2? second
        // to no more than 100% and no less than 0%
        // if status is initially 0, it will be increasing and decreasing if 100
        // if it is someplace inbetween, opposite of what it was
        //    This means that the direction needs a place in the struct
        //    that the direction changes at the moment it is determined that a HOLD is occuring
        //        but only one direction change during a HOLD determination.
      }
    }
    else { // this is the initial touch
      touch.getPosition(x, y);  // initialize the averaging
      xSum = x;
      ySum = y;
      xyCount = 1;
      // Start touch timer
      touchStarted = true;
      touchTimerStart = millis();
    }
  }
  else { // untouch occurred!  figure out which key and what to do
    // the location of the last touch point is used
    touchStarted = false;
    if (displayTouched){
      displayTouched = false;
      // remove visual feedback
      ucg.setColor(0,xBLACK);
      ucg.drawFrame(2,1,MAX_COLUMN-1,MAX_ROW); // turn off green frame indicating touched

      // average the location of the key touched
      x = xSum / xyCount;
      y = ySum / xyCount;
      // the screen was touched for a minimum amount of time
      // determine if the touch was one of the buttons
      if (x < XUL || x >= XUL+2*PAD_SIZE || y < YUL || y >= 3*PAD_SIZE+YUL){  //  Was the touch in the pad out of range (or "equal to" so that math works)
        // Is this a RESET touch?
        if (x >= RESET_ZONE_ULx && y > RESET_ZONE_ULy) {
          //for (int i=0;i<10;i++) {displayRESETzone(false);delay(500);displayRESETzone(true);delay(500);}
          if (resetZoneTouched) ESP.restart(); else displayRESETzone(true);
        }
        // do nothing because the touch was not touching a key
        // consider an 8 bit integer function that determines key, 
        // returning the key, or -1 if not a key.
        // this function would be called during HOLD just before status is sent.
      } // 
      else {  // touch was on one of the keys
        // clear resetZoneTouched
        if(resetZoneTouched) displayRESETzone(false);  // will also reset the boolean resetZoneTouched

        // calculate which key was touched
        key = ((x-XUL)/PAD_SIZE) + 2 * ((y-YUL)/PAD_SIZE);
        // change the state of the key if there was no key HOLD
        if (keys[key].state == 0) keys[key].state = keys[key].onState; else keys[key].state = 0;
          
        actOnKeyPressed(key,true);  // toggle key and send state
      }
    }
  }
}
//------------------------------------------------------updateActivity
void updateActivity(uint8_t key){
#ifdef DEBUG
  Serial.println("updateActivity");
#endif
  Serial.printf("\r\nkey: %d state: %d incoming state: %d\r\n",key,keys[key].state,keys[key].incomingState);
  keys[key].incomingStateChange = false; // acknowledge that change has been made
  if (keys[key].state == keys[key].incomingState) return;

  keys[key].state = keys[key].incomingState;
  keys[key].incomingState = 255;  // reset incoming state
  if (keys[key].state > 0) keys[key].onState = keys[key].state; // new on state
  actOnKeyPressed(key,false);  // this is the MQTT informing of a state change
                               // do not publish new state.

  Serial.printf("\r\nkey: %d state: %d\r\n",key,keys[key].state);
  Serial.println("END updateActivity");
  
}

//------------------------------------------------------updateKeysAttributes
void updateKeysAttributes (const String& topic, const String& message){
#ifdef DEBUG
  Serial.println("updateKeysAttributes");
#endif

  // this callback is an interupt service routine and time here must be minimized
  
  // ULS/activity/label
  // 01234567890123456789
  // ULS/panel/
  uint8_t ptr;
  uint8_t key = globalKey;
  if (keys[key].type == 'p') {
    // this was set in processDataForPanel routine
    // Parse index of next menu
    // the "next panel index" is found right after the colon that is after the N
    ptr = message.indexOf('N'); ptr = message.indexOf(':',ptr) +1;
    keys[key].index = message.substring(ptr).toInt();
  }


  // Extract the label from the JSON
  
  // This is a bit tricky
  // expecting a JSON but deserialize uses a lot of funky memory
  // mPtr will point to the start of the first character of the first line of the label
  // L":["  which is one after the second quote after L
  // The label line ends with "endOfLine will point the first " after mPtr which is 1 after the end of the line
  // but a line cannot be more than 6 characters
  // so copy characters until there have been 6 characters copied or the quote has been encountered
  // 
  uint8_t mPtr = message.indexOf('L');
  // now look for the colon after the L
  mPtr = message.indexOf(':',mPtr);
  // now look for the quote and add 1 to get to the start
  mPtr = message.indexOf('\"',mPtr)+1;
  for (uint8_t line=0;line<6;line++){
    uint8_t max = mPtr+6;
    ptr = 0;
    do {
      if (message.charAt(mPtr) == '\"') break; // check for null string
      keys[key].label[line][ptr]=message.charAt(mPtr);
      ptr++;mPtr++;
    } while (message.charAt(mPtr) != '\"' && mPtr < max);
    keys[key].label[line][ptr] = '\0'; // null terminate the string
    if (line<6){
      // If the line is too long find the first quote,
      // which should be the qoute to which the pointer is pointing,
      // but won't be if the line is more than 6 characters.
      // now look for the comma.
      mPtr = message.indexOf('\"',mPtr); // looks for a quote, even at this point
      // now look for the quote and move the pointer 1 past the quote
      mPtr = message.indexOf('\"',mPtr+1)+1;
    }
  }
  
  // extract the color from the JSON
  
  // find the C
  mPtr = message.indexOf('C');  
  // find the bracket plus 1
  mPtr = message.indexOf('[',mPtr)+1;
  keys[key].color[0] = message.substring(mPtr).toInt();
  // find the comma plus 1
  mPtr = message.indexOf(',',mPtr)+1;
  keys[key].color[1] = message.substring(mPtr).toInt();
  // find the next comma plus 1
  mPtr = message.indexOf(',',mPtr)+1;
  keys[key].color[2] = message.substring(mPtr).toInt();
  
  // printKey(key);    

  keyUpdateInProgress = false;
  
#ifdef DEBUG
  Serial.printf("This panel is %d Next Panel is %d\r\n",currentPanel, keys[key].index);
  Serial.println("END updateKeysAttributes");
#endif
}
//------------------------------------------------------displayUpdating
void displayUpdating(uint8_t key, bool display){
  // position the cursor and print the label of the key
  uint16_t xUL = XUL+((key)%2)*PAD_SIZE+2;
  uint16_t yUL = YUL+((key)/2)*PAD_SIZE+34;
  if(display){ucg.setColor(0,0x255,0x255);ucg.setPrintPos(xUL,yUL);ucg.print("SETUP");}
  else{ucg.setColor(0,xBLACK);ucg.drawBox(xUL,yUL-8,48,8);}
  ucg.setColor(0,xWHITE);
}
//------------------------------------------------------incomingActivityChange
void incomingActivityChange(const String& topic, const String& message){
  // this callback is an interupt service routine and time here must be minimized
  uint8_t activity = topic.substring(13).toInt();
  uint8_t key = activityIndexToKey[activity];  // determines which key is associate with this activity
  // ULS/activity/255
  // 01234567890123
  keys[key].incomingState = message.toInt(); // note change, 
  keys[key].incomingStateChange = true; // flag change for loop
  keyUpdateInProgress = false;
  
}

//------------------------------------------------------processDataForPanel
void processDataForPanel(){
#ifdef DEBUG
  Serial.println("processDataForPanel");
#endif
  // This will only be called when the data for a panel has come in
  // it is called from the loop to miimize the time in the callback routine
  dataToBeProcessedForPanel = false;  // data will be processed here
  
  // this is the master routine for setting the attributes of a key
  // The same routine routine
  for (uint8_t key=0;key<6;key++){
#ifdef DEBUG
    //Serial.printf("\r\n-----key: %d type: %c index: %d\r\n",key,keys[key].type, keys[key].index);
#endif
    bool getAttributes = true;
    String attributesTopic = "ULS/activity/255/label";  // allocate memory to prevent buffer overrun
    switch (keys[key].type) {
      case 'p': 
        attributesTopic = "ULS/menu/" + String(keys[key].index);
        break;
      case 'a' :
        keys[key].activityTopic = "ULS/activity/" + String(keys[key].index); // formulate activity topic
        attributesTopic = keys[key].activityTopic + "/label"; // formulate attributes topic for this key
        activityIndexToKey[keys[key].index] = key;  // set pointer to key from index
        break;
      case 'n' :
        getAttributes = false;
        clearKeyAttributes(key);
        break;
      default :
        getAttributes = false;
        clearKeyAttributes(key);
        break;
    }
    if (getAttributes){
      globalKey = key;  // for passing key to callback (ISR)
      keyUpdateInProgress = true;
      displayUpdating(key,true);  // note, on screen, that the attributes are being updated

      // get attributes from menu (start)-------------------------------------------------------------------------------)

      // Request, by subscribing, the attributes of this key
      // This is not working well in version 44, maybe we can fix it here
      // As of 48, this may still be a problem
      delay(1000); // maybe the subscribe came too fast
      client.loop();
      delay(100); // maybe the subscribe came too fast
      client.subscribe(attributesTopic,updateKeysAttributes);
      delay(1000); // maybe the subscribe came too fast  //                                          delay after subscribe
      client.loop();
      unsigned long wStart = millis()-250; // check network right away
      while(keyUpdateInProgress){  // this will be updated in the callback (ISR)
        if (millis()-wStart > 750){
          wStart = millis();
          client.loop();
#ifdef DEBUG
          Serial.print('.');
#endif
          } // keep network updated
      }
      client.unsubscribe(attributesTopic);
      
      // get attributes from menu (end)-------------------------------------------------------------------------------

      // noW start getting activity (change of state) updates
      if (keys[key].type == 'a') {

        //char debugMessagestr[] = "wait 4state"; debugMessage(debugMessagestr,key);
        keyUpdateInProgress = true;
        delay(1000); // maybe the subscribe came too fast
        client.loop();
        delay(100); // maybe the subscribe came too fast

        client.subscribe(keys[key].activityTopic,incomingActivityChange);
        delay(1000); // maybe the subscribe came too fast
        client.loop();
        Serial.println(" Waiting for callback from first subscribe");
        wStart = millis()-250; // check network right away
        while(keyUpdateInProgress){  // this will be updated in the callback (ISR)
          if (millis()-wStart > 750){
            wStart = millis();
            client.loop();
#ifdef DEBUG
            Serial.print('.');
#endif
          } // keep network updated
        }
      }
      else{
#ifdef DEBUG
        // Serial.print("key type: ");Serial.println(keys[key].type);
#endif
      }

      displayUpdating(key,false);
    }
    //Serial.printf("\r\nkey: %d type: %c state %d",key,keys[key].type,keys[key].state);
    // The key really isn't useful if there is no activity on the MQTT broker
    // The activity needs to have a status
    if (keys[key].type != 'a') lightUpKey(key,false); // show attribute of key.

  }
#ifdef DEBUG
  Serial.println("\r\nEND processDataForPanel");
#endif
}
//------------------------------------------------------acquireDataForPanel
void acquireDataForPanel(const String& topic, const String& message){
#ifdef DEBUG
  Serial.println("acquireDataForPanel");
#endif
  // this callback is an interupt service routine and time here must be minimized

  //  client.unsubscribe(topic); is not done here because it is believed that 
  //  when a topic is unsubscribed, the data is lost

  // 01234567890
  // ULS/panel/n
  uint8_t incomingPanelIndex = topic.substring(10).toInt();
  if (incomingPanelIndex == currentPanel) {panelDataAcquired = true; return; } // what if an update is needed? go to another panel and come back to this one?
  currentPanel = incomingPanelIndex;  // current panel will have to be published to ULS/000000/panel
                               // indoing so, the process will arrive back at this routine, but the next time
                               // the incomingPanelToLoad will be the current panel
                               // this subscription will have to be unsubscribed once this background activity has stoped
                               //
                               // This is done so that a panel can be forced to load by another process publishing to ULS/000000/panel
  
  uint8_t Tptr = message.indexOf("T");  // The JSON has "Type"
          Tptr = message.indexOf('\"',Tptr)+1; // move the point to 1 past the ending quote of "Type"
  uint8_t Aptr = message.indexOf("I");  // The JSON has "Index"
          Aptr = message.indexOf("[",Aptr)+1; // move the point to 1 past the following open bracket
  for (uint8_t key=0;key<6;key++){
    // if the existing key is an activity, unsubscribe from the activity and unset poiniter 
    if (keys[key].type == 'a') client.unsubscribe(keys[key].activityTopic);
    client.loop();  // keep connection alive
    clearKeyAttributes(key);
    // that key was cleaned up, now get the new pointers 
  
    // {"Type":["p","a","n","n","n","n"],"Index":[1,0,0,0,0,0]}
    Tptr = message.indexOf('\"',Tptr)+1; // set the pointer to 1 past quote which should be the type for key 
    keys[key].type = message.charAt(Tptr); // extract the character there.
    Tptr=Tptr+2;  // set the pointer to 1 past the ending quote which sets up for the next search for a quote
    // extract the integer number. 
    keys[key].index = message.substring(Aptr).toInt(); 
    Aptr=message.indexOf(',',Aptr)+1; //find the comma after the interger + 1 (The last key may not find a comma, but the found pointer won't be used)
  }
  
  dataToBeProcessedForPanel = true;
  panelDataAcquired = true; // make sure this won't get unsubscribed until it finishes
  Serial.println("END acquireDataForPanel");
  
}

//------------------------------------------------------setup
void setup(){
  Serial.begin(115200);
  delay(2000);  // for some reason, it starts before the print can get going.
  Serial.printf("\r\n%s version %s\r\n",PROGRAM_NAME,VER);

  pinMode(BACKLIGHT_PIN, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  analogWriteRange(100);
  analogWrite(BACKLIGHT_PIN,BacklightStatus);

  ucg.begin(UCG_FONT_MODE_TRANSPARENT);
  //ucg.begin(UCG_FONT_MODE_SOLID);
  //  ucg.setFont(ucg_font_helvB08_tr);
  ucg.setFont(ucg_font_amstrad_cpc_8r);
  ucg.clearScreen();

  displayRESETzone(true);
  
  hostTopic = "ULS/" + hostname.substring(hostname.length()-6);
  snprintf(dispbuf,sizeof(dispbuf),"%s",hostTopic.substring(4).c_str());
  displayInAZone(UL);
  snprintf(dispbuf,sizeof(dispbuf),"%s",VER);
  displayInAZone(LL);

  touch.begin(ucg.getWidth(), ucg.getHeight());  // Must be done before setting rotation
  // run XPTCalibrate_02 to get calibration numbers
  EEPROM.begin(8); // calibration has 4 points each of 2 bytes
  EEPROM.get(0,touchCalibration);
  touch.setCalibration(touchCalibration[0],touchCalibration[1],touchCalibration[2],touchCalibration[3]);

  // Optional functionalities of EspMQTTClient
  client.enableDebuggingMessages(); // Enable debugging messages sent to serial output // not so optional, see version notes
  //client.enableHTTPWebUpdater(); // Enable the web updater. User and password default to values of MQTTUsername and MQTTPassword. These can be overridded with enableHTTPWebUpdater("user", "password").
  client.enableOTA(); // Enable OTA (Over The Air) updates. Password defaults to MQTTPassword. Port is the default OTA port. Can be overridden with enableOTA("password", port).
  client.enableLastWillMessage(hostTopic.c_str(), "offline",true);  // You can activate the retain flag by setting the third parameter to true
}

//------------------------------------------------------onConnectionEstablished
// This function is called once everything is connected (Wifi and MQTT)
// WARNING : YOU MUST IMPLEMENT IT IF YOU USE EspMQTTClient
void onConnectionEstablished(){
#ifdef DEBUG
  Serial.println("onConnectionEstablished");
#endif
  client.publish(hostTopic,"online",true);
  versionTopic = hostTopic + "/version";
  client.publish(versionTopic, VER,true);
  
  // display IP address on display
  WiFi.localIP().toString().toCharArray(dispbuf,16);
  displayInAZone(UR);


  // if this is the first connection since boot or any subsequent connection
  //  the subscriptions are lost.  Therefore start over as if it were the first boot
  // get the index of current panel from the MQTT broker, that's the payload at ULS/000000/panel
  ClientPanelTopic = hostTopic+"/panel";
  client.subscribe(ClientPanelTopic,[](const String & payload) { // Now wait for callback
      // callback
      // now get the data for that panel
      //uint8_t incomingPanelIndex = payload.toInt();  // get for current panel
      //topic = "ULS/panel/" + String(incomingPanelIndex);
      uint8_t newPanel = payload.toInt();
#ifdef DEBUG
  //Serial.printf("Callback Incoming Panel: %d CurrentPanel: %d\r\n",newPanel,currentPanel);
#endif  
      if (newPanel != currentPanel ) {
        GeneralPanelTopic = "ULS/panel/" + String(newPanel);
        client.subscribe(GeneralPanelTopic,acquireDataForPanel);
      }
    }
    
    // this is kind of tricky
    // the initial panel is at ULS/000000/panel
    // this command subscribes to that topic which will return the panel that needs to be loaded
    // but we don't know when that will happen, only that the callback routine will get that panel index
    // and subscribe to the topic of that panel. When the data for that panel comes in,
    // the "acquireDataForPanel" is called.  See that routine for further documentattion
    // The subscription to the panel topic will be unsubscribed once the data is acquired
    //
    // Note the subscription to the orignal topic, ULS/000000/panel, will still be subscribed.
    // Any changes to that topic will subsequently end up at the acquireDataForPanel routine.
    // the first line of acquireDataForPanel is to check to see if there is a change in panel
    // and won't make any changes
  );
}

unsigned long checkLoopStart = millis()-1000;  // to prevent checking MQTT too often
//------------------------------------------------------loop
void loop(){
  if (millis()-checkLoopStart > 500){ // every second check loop
    client.loop(); // MQTT keep alive, send data, receive data
    checkLoopStart = millis();
  }

  checkTouch();

  if (panelDataAcquired){
    panelDataAcquired = false;
    client.unsubscribe(GeneralPanelTopic);
  }

  if (dataToBeProcessedForPanel) processDataForPanel();
  
  for (uint8_t keyToCheck = 0;keyToCheck<6;keyToCheck++){
    if (keys[keyToCheck].incomingStateChange) updateActivity(keyToCheck);
  }

  // check to see if it is time to turn off the backlight
  if (millis()-BacklightTimeoutStart > BACKLIGHT_TIMEOUT && BacklightStatus){
    BacklightStatus = 0;
    analogWrite(BUILTIN_LED,BacklightStatus);
  }
}