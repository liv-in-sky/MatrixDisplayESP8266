#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <TimeLib.h>
#include <WiFiUdp.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <FS.h>
#include "WM.h"
#include <ArduinoOTA.h>
#include "utf8_font.h"
#include "res.h"
#include <SPI.h>
#include "Parola_Fonts_data.h"



char refreshSeconds[10] = "60";
char scrollPause[10] = "5";
char url[255] = "";
char scrollSpeed[10] = "25";
String errorHandleRefresh = "!Err IOBroker";
String errorHandleRefresh2 = "!RefreshSec!";

textEffect_t scrollEffectIn  = PA_SCROLL_LEFT;
textEffect_t scrollEffectUp = PA_SCROLL_UP;
textEffect_t scrollEffectDown = PA_SCROLL_DOWN;
textEffect_t scrollEffectRight  = PA_SCROLL_RIGHT;
textPosition_t scrollAlign   = PA_CENTER;

String configFilename     = "sysconf.json";

//fixe Display
/*  HARDWARE_TYPE:
    PAROLA_HW,    ///< Use the Parola style hardware modules.
    GENERIC_HW,   ///< Use 'generic' style hardware modules commonly available.
    ICSTATION_HW, ///< Use ICStation style hardware module.
    FC16_HW       ///< Use FC-16 style hardware module.
*/
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES   16
#define CLK_PIN       D5
#define DATA_PIN      D7
#define CS_PIN        D8
MD_Parola P = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);
//MD_Parola P = MD_Parola(CS_PIN, MAX_DEVICES);
//uint8_t degC[] = {5, 6 , 15 , 9 , 15 , 6 };
//uint8_t line[] = {4,  0, 8, 8, 8 };
//uint8_t plus[] = {5, 8, 8, 62, 8, 8};
//uint8_t block[] =  {3, 255, 255, 255};
//uint8_t heart[] = {5, 28, 62, 124, 62, 28};

char curMessage[75];

int loopCount = 0;
int loopCountBlink = 0;
int resetCount = 0;
int cnt = 0;
long lastMillis = 0;
String valueArray[20];
int valueCount = 0;
int intensity = 1;
String intensityLoad;
int modeCnt = 0;
byte timeSetTryCount = 0;
String settingsArray[5];
int settingsCount = 0;
String settingString2;
int valueCountBefore=0;
int errorCode = 0;
String datum;
int datumJa=1;
bool zeitHelp=false;

//settingString ist variable für laufen: festehend ist aus - laufen ist ein
String settingString = "\"nein\"";
String settingStringAlt;
bool schalten = false;
int modus = 2;
String modusLoad;
int modusAlt = 9 ;
int modusCase = 2;
bool shutty = false;
String errorHandle;
String errorMessage = "! ERROR !";
String errorMessage2;
int check;
bool ntpSave = false;

//WifiManager - don't touch
#define IPSIZE              16
bool shouldSaveConfig        = false;
bool wifiManagerDebugOutput  = true;
char ip[15]      = "0.0.0.0";
char netmask[15] = "0.0.0.0";
char gw[15]      = "0.0.0.0";
bool startWifiManager = false;

const int NTP_PACKET_SIZE = 48;
byte packetBuffer[ NTP_PACKET_SIZE];
unsigned int localNTPport = 2390;
const char* ntpServerName = "ptbtime2.ptb.de";
WiFiUDP NTPudp;

int localCNTRLport = 6610;
char incomingPacket[255];
WiFiUDP CNTRLudp;

#define key1 D1
#define key2 D2
bool key1last = false;
bool key2last = false;

const uint16_t PAUSE_TIME = 2000;

#if  DEBUG
#define PRINT(s, x) { Serial.print(F(s)); Serial.print(x); }
#define PRINTS(x) Serial.print(F(x))
#define PRINTX(s, x)  { Serial.print(F(s)); Serial.print(x, HEX); }
#else
#define PRINT(s, x)
#define PRINTS(x)
#define PRINTX(s, x)
#endif

// Global variables
char  *pc[] =
{
  "abcABC",
  "äöüßÄÖÜ",
  "50€/kg³",
  "Español",
  "30m/s²",
  "Français",
  "20µs/°C",
};

uint8_t utf8Ascii(uint8_t ascii)
{
  static uint8_t cPrev;
  uint8_t c = '\0';

  if (ascii < 0x7f)   // Standard ASCII-set 0..0x7F, no conversion
  {
    cPrev = '\0';
    c = ascii;
  }
  else
  {
    switch (cPrev)  // Conversion depending on preceding UTF8-character
    {
      case 0xC2: c = ascii;  break;
      case 0xC3: c = ascii | 0xC0;  break;
      case 0x82: if (ascii == 0xAC) c = 0x80; // Euro symbol special case
    }
    cPrev = ascii;   // save last char
  }

  PRINTX("\nConverted 0x", ascii);
  PRINTX(" to 0x", c);

  return (c);
}

void utf8Ascii(char* s)
// In place conversion UTF-8 string to Extended ASCII
// The extended ASCII string is always shorter.
{
  uint8_t c, k = 0;
  char *cp = s;

  PRINT("\nConverting: ", s);

  while (*s != '\0')
  {
    c = utf8Ascii(*s++);
    if (c != '\0')
      *cp++ = c;
  }
  *cp = '\0';   // terminate the new string
}



void setup() {
  pinMode(16, OUTPUT);
  digitalWrite(16, LOW);
  Serial.begin(115200);
  PRINTS("\n[Parola UTF-8 display Test]");
  // Do one time in-place conversion of the strings to be displayed
  for (uint8_t i = 0; i < ARRAY_SIZE(pc); i++)
    utf8Ascii(pc[i]);

  // Initialise the Parola library
  P.begin();
  P.setInvert(false);
  P.setPause(PAUSE_TIME);
  P.setFont(ExtASCII);

  pinMode(key1, INPUT_PULLUP);
  pinMode(key2, INPUT_PULLUP);
  P.begin();
  P.setIntensity(intensity-1);
  P.displayClear();
  P.displaySuspend(false);
  //  P.addChar('$', degC);
  //  P.addChar('-', line);
  //  P.addChar('_', block);
  //  P.addChar('§', heart);
    P.displayText("here we go ...", PA_LEFT, 25, 10, PA_PRINT, PA_PRINT);
   P.displayAnimate();
//  String startMessage = "here we go";
//  startMessage.toCharArray(curMessage, 16);
//    utf8Ascii(curMessage);
// P.displayText(curMessage, PA_LEFT, 15, 1500, scrollEffectIn, scrollEffectUp);//PA_LEFT, 25, 10, PA_PRINT, PA_PRINT);
// P.displayAnimate();

  if (digitalRead(key1) == LOW || digitalRead(key2) == LOW) {
    startWifiManager = true;
  }

  if (!SPIFFS.begin()) {
    Serial.println("Failed to mount file system");
  } else {
    if (!loadSysConfig()) {
      Serial.println("Failed to load config");
      startWifiManager = true;
    } else {
      Serial.println("Config loaded");
    }
  }

  //Nachdem die Config geladen wurde...
  P.setIntensity(intensity-1);

  if (doWifiConnect() == true) {
    NTPudp.begin(localNTPport);
    CNTRLudp.begin(localCNTRLport);
    setSyncProvider(getNtpTime);
    setSyncInterval(3600);
    while (timeStatus() == timeNotSet) {
      Serial.println("Waiting for Time set");
      delay(1500);
      timeSetTryCount++;
      if (timeSetTryCount > 4) {
        P.displayText("NTP FAILURE", PA_LEFT, 25, 10, PA_PRINT, PA_PRINT);
        P.displayAnimate();
        delay(2000);
        Serial.println("RESTART Time");
        ESP.restart();
      }
    }
    
    Serial.println(WiFi.localIP().toString());

    WiFi.localIP().toString().toCharArray(curMessage, 16);
    utf8Ascii(curMessage);
      P.displayText(curMessage, PA_LEFT, 15, 0, scrollEffectIn, scrollEffectUp);
     P.displayAnimate();
     //delay(3000);
    startOTAhandling();
  }
  else ESP.restart();

delay(1500);
 digitalWrite(16, HIGH);

if (MAX_DEVICES<=8) { datumJa=0;}
               else if (MAX_DEVICES>8 && MAX_DEVICES<16) {datum=calcDate(now()); datumJa=1;}
               else {datum=calcDate(now()); datumJa=2;}
//Serial.println(datum + String(datumJa));
}



void loop() {
  ArduinoOTA.handle();
 // delay (1000);
 // Serial.println(ESP.getFreeHeap(),DEC);
  if (digitalRead(key1) == LOW ) {
    if (!key1last) {
      key1last = true;
      intensity = intensity + 2;
      if (intensity > 14) intensity = 1;
      P.setIntensity(intensity-1);
      Serial.println("Set intensity to " + String(intensity));
      if (!saveSysConfig()) {
        Serial.println("Failed to save config");
      } else {
        Serial.println("Config saved");
      }
      delay(50);
    }
  } else {
    key1last = false;
  }


  // Serial.println("wird geschaltet : " +  String(schalten));
  if (digitalRead(key2) == LOW) {
    delay(700);
    modus++;
    if (modus > 12) modus = 1;
    String yyy = (String)modus;
    yyy = "Mode: " + yyy;
    Serial.println(yyy);
    P.displayShutdown(false);
    yyy.toCharArray(curMessage, yyy.length() + 1);
    P.displayText(curMessage, PA_CENTER, 25, 10, PA_PRINT, PA_PRINT);
    P.displayAnimate();

    delay(500);

   
  }
  String udpMessage = handleUDP();




  if (((millis() - lastMillis > String(refreshSeconds).toInt() * 1000) || lastMillis == 0 || udpMessage == "update") && String(url) != "") {
    
    Serial.println("------------------Fetching data from URL...------------------------------------");


   // Serial.println(setModeToURL("1"));  // TEST SENDEN ------------------------------------------------------------------

    settingString2 = configSettingFromURL(); // "5;1;5;3;15";
    if (settingString2 != "HTTP ERROR"){
    Serial.println( "Setting bekommen : " + String(settingString2));
    char buf3[settingString2.length() + 1];
    settingString2.toCharArray(buf3, sizeof(buf3));
    //Serial.println("buf3 : " + String(buf));
    char *q = buf3;
    char *str3;
    settingsCount = 0;
    while ((str3 = strtok_r(q, ";", &q)) != NULL) {
      settingsArray[settingsCount] = str3;
      settingsCount++;
    }
    } else {
      errorCode = 6;
      modus = 99;
    }
       
      
  //  Serial.println("Array Setting platz1 : " + String(settingsArray[0]) + String(settingsArray[1]) + String(settingsArray[2]) + String(settingsArray[3]) + String(settingsArray[4]));

    modusLoad = settingsArray[0];
    modus = modusLoad.toInt();
    if (modusAlt != modus) {shutty = false; ntpSave = true;}   //für Datumabfrage und display shutdown
    modusAlt = modus;
    Serial.println( "Modus von IOBROKER ist : " + String(modus));
    
    if (modus < 0 || modus > 12) {
      modus = 99;
      Serial.println( "ERROR: Mode falsch : " + String(modus));
       errorCode = errorCode <6 ? 5 : 6;
     //  errorCode = 5;
    }



    settingsArray[2].toCharArray(refreshSeconds, 11);
    // configRefreshFromURL().toCharArray(refreshSeconds,11);
    Serial.println( "Abfrage Refresh Time : " + String(refreshSeconds));

    check = String(refreshSeconds).toInt() * 1000;
    if (check < 1000 || check > 600000) {
      errorHandle = "30";
      errorHandle.toCharArray(refreshSeconds, 11);
      Serial.println( "ERROR: refreshTime : " + String(refreshSeconds));
      //  fehlerHandling(errorMessage, "RefreshTime");
       errorCode = errorCode <6 ? 4 : 6;
     //  errorCode = 4;
      modus = 99;
    }

    settingsArray[3].toCharArray(scrollPause, 11);
    // configScrollPauseFromURL().toCharArray(scrollPause,11);
    Serial.println( "Abfrage ScrollPause : " + String(scrollPause));

    check = String(scrollPause).toInt() * 1000;
    if (check < 1 || check > 30000) {
      errorHandle = "5";
      errorHandle.toCharArray(scrollPause, 11);
      Serial.println( "ERROR: Abfrage ScrollPause : " + String(scrollPause));

       errorCode = errorCode <6 ? 3 : 6;
      modus = 99;
    }

    settingsArray[4].toCharArray(scrollSpeed, 11);
    // configSpeedFromURL().toCharArray(scrollSpeed,11);
    Serial.println( "Abfrage scroll Speed URL : " + String(scrollSpeed));

    check = String(scrollSpeed).toInt();
    if (check < 0 || check > 300) {
      errorHandle = "25";
      errorHandle.toCharArray(scrollSpeed, 11);
      Serial.println( "ERROR: Abfrage ScrollSpeed : " + String(scrollSpeed));

       errorCode = errorCode <6 ? 2 : 6;
      modus = 99;
    }

    intensityLoad = settingsArray[1];
    intensity = intensityLoad.toInt();
    Serial.println( "Intensity von IOBROKER ist : " + String(intensity));
    if (intensity < 1 || intensity > 11) {
      intensity = 1;
      Serial.println( "ERROR: Intensity falsch : " + String(intensity + String(errorCode)));
             errorCode = errorCode<6 ? 1 : 6;
             Serial.println( "ERROR: Intensity falsch : " + String(intensity + String(errorCode)));
      modus = 99;
    }
   
    P.setIntensity(intensity - 1);

 //----RESET ARRAY sonst falsche Daten-------memset - sram leak!-------------------------------------     
  
        for( int i = 0; i < valueCount;  ++i ) valueArray[i] = "";
 //--------------------------------------------------------------------------------------------------   

    String valueString = loadDataFromURL(); //"123;123;456";
    char buf[valueString.length() + 1];
    valueString.toCharArray(buf, sizeof(buf));
  //  Serial.println("buf : " + String(buf) + "  valueString :  " + valueString  );
    char *p = buf;
    char *str;
    valueCount = 0;
    while ((str = strtok_r(p, ";", &p)) != NULL) {
      valueArray[valueCount] = str;
      valueCount++;
    }
    
    lastMillis = millis();

  }

    if (valueCount != valueCountBefore){
          valueCountBefore=valueCount;
          loopCount=-1;
         
    }

//Speziell für Mode 7 und 10
if (ntpSave) {
  if (modus != 7 ) loopCountBlink = 0;
  if (modus != 10 ) loopCountBlink = 0;}
  
  if (!shutty) P.displayShutdown(false);
  if (modus != 99) {
    digitalWrite(16, HIGH);
    switch (modus) {
// Serial.println( ESP.getFreeHeap(),DEC);
      case 0:  {

             resetCount++;
            // Serial.println(String(resetCount));
          if (resetCount >40000 ) {
            resetCount = 0;
             ESP.restart();
    }
          
          if (P.displayAnimate())
          {
            errorMessage = "! ERROR ! bad Mode!";
            errorMessage.toCharArray(curMessage, errorMessage.length() + 1);
            P.displayReset();
            P.displayText(curMessage, PA_LEFT, 25, 10, scrollEffectIn, scrollEffectIn);
            P.displayAnimate();
          }
      }
      case 1:  {
          resetCount = 0;
          if (P.displayAnimate())
          {

            loopCount++;
            if (loopCount > valueCount || valueCount == 0) {
              String Zeit = calcTime(now());
         if (Zeit == "00 : 00" || (ntpSave)) { if (!zeitHelp) {datum=calcDate(now()); zeitHelp=true; ntpSave=false;}   //wegen DATUM Umstellung   
           } else { zeitHelp=false;}
             
            switch (datumJa) {
                           case 0: {Zeit.toCharArray(curMessage, 10);break;} 
                           case 1: {Zeit = Zeit + "  -  " +datum;Zeit.toCharArray(curMessage, 20);break;} 
                           case 2: {Zeit = Zeit + "           " +datum; Zeit.toCharArray(curMessage, 26);break;} }
              
         
              loopCount = -1;
            } else {
              String currentValue = valueArray[loopCount];
              currentValue.toCharArray(curMessage, currentValue.length() + 1);
              utf8Ascii(curMessage);


            }
            // //memset(valueArray,0,sizeof(valueArray));
            P.displayReset();
            P.displayText(curMessage, scrollAlign, String(scrollSpeed).toInt(), String(scrollPause).toInt() * 1000, scrollEffectIn, scrollEffectUp);//);PA_SCROLL_DOWN_LEFT, PA_SCROLL_DOWN_LEFT
            // P.displayText(curMessage, PA_LEFT, String(scrollSpeed).toInt(), 10, PA_PRINT, PA_PRINT);
            P.displayAnimate();
          }

          break;
        }

      case 2:  {
          resetCount = 0;
          String Zeit = calcTime(now());
         if (Zeit == "00 : 00" || (ntpSave)) { if (!zeitHelp) {datum=calcDate(now()); zeitHelp=true; ntpSave=false;Serial.println("Tag holen : " +datum);}   //wegen DATUM Umstellung   
           } else { zeitHelp=false;}  //Serial.println("zeithelp im case : " +String(zeitHelp));

          switch (datumJa) {
                           case 0: {Zeit.toCharArray(curMessage, 10);break;} 
                           case 1: {Zeit = Zeit + "  -  " +datum;Zeit.toCharArray(curMessage, 20);break;} 
                           case 2: {Zeit = Zeit + "           " +datum; Zeit.toCharArray(curMessage, 26);break;} }
          

          //     P.displayReset();
          P.displayText(curMessage, PA_CENTER, String(scrollSpeed).toInt(), 10, PA_PRINT, PA_PRINT);
          P.displayAnimate();
          break;
        }
      
      case 3:  {
          resetCount = 0;
          ntpSave=0;
          if (P.displayAnimate())
          {
 
            loopCount++;
            if (loopCount > valueCount || valueCount == 0) {
            //  String Zeit = calcTime(now());
            //  Zeit.toCharArray(curMessage, 10);
              loopCount = -1;
            } else {
              String currentValue = valueArray[loopCount];
              currentValue.toCharArray(curMessage, currentValue.length() + 1);
              utf8Ascii(curMessage);


            }
            // //memset(valueArray,0,sizeof(valueArray));
            P.displayReset();
            P.displayText(curMessage, scrollAlign, String(scrollSpeed).toInt(), String(scrollPause).toInt() * 1000, scrollEffectIn, scrollEffectIn);
            P.displayAnimate();
            
          }



          break;
        }


      case 4:  {
          resetCount = 0;
          if (P.displayAnimate())
          {



            loopCount++;
            if (loopCount > valueCount || valueCount == 0) {
              String Zeit = calcTime(now());
         if (Zeit == "00 : 00" || (ntpSave)) { if (!zeitHelp) {datum=calcDate(now()); zeitHelp=true; ntpSave=false;}   //wegen DATUM Umstellung   
           } else { zeitHelp=false;}
            switch (datumJa) {
                           case 0: {Zeit.toCharArray(curMessage, 10);break;} 
                           case 1: {Zeit = Zeit + "  -  " +datum;Zeit.toCharArray(curMessage, 20);break;} 
                           case 2: {Zeit = Zeit + "           " +datum; Zeit.toCharArray(curMessage, 26);break;} }
              loopCount = -1;
            } else {
              String currentValue = valueArray[loopCount];
              currentValue.toCharArray(curMessage, currentValue.length() + 1);
              utf8Ascii(curMessage);

            }
           // //memset(valueArray,0,sizeof(valueArray));
            P.displayReset();
            P.displayText(curMessage, scrollAlign, String(scrollSpeed).toInt(), String(scrollPause).toInt() * 1000, scrollEffectIn, scrollEffectRight);
            P.displayAnimate();
          }


          break;
        }

      case 5:  {
          resetCount = 0;
          if (P.displayAnimate())
          {

            loopCount++;
            if (loopCount > valueCount || valueCount == 0) {
              String Zeit = calcTime(now());
         if (Zeit == "00 : 00" || (ntpSave)) { if (!zeitHelp) {datum=calcDate(now()); zeitHelp=true; ntpSave=false;Serial.println("Tag holen : " +datum);}   //wegen DATUM Umstellung   
           } else { zeitHelp=false;Serial.println("zeithelp im case : " +String(zeitHelp));}
            switch (datumJa) {
                           case 0: {Zeit.toCharArray(curMessage, 10);break;} 
                           case 1: {Zeit = Zeit + "  -  " +datum;Zeit.toCharArray(curMessage, 20);break;} 
                           case 2: {Zeit = Zeit + "           " +datum; Zeit.toCharArray(curMessage, 26);break;} }
              loopCount = -1;
            } else {
              String currentValue = valueArray[loopCount];
              currentValue.toCharArray(curMessage, currentValue.length() + 1);
              utf8Ascii(curMessage);


            }
          //  //memset(valueArray,0,sizeof(valueArray));
            P.displayReset();
            P.displayText(curMessage, scrollAlign, String(scrollSpeed).toInt(), String(scrollPause).toInt() * 1000, scrollEffectIn, scrollEffectIn);
            //P.displayText(curMessage, scrollAlign, String(scrollSpeed).toInt(), String(scrollPause).toInt() * 1000, scrollEffectIn, scrollEffectIn);
            P.displayAnimate();
          }

          break;
        }
      //STEHEND EIN WERT
      case 6:  {
          ntpSave=0;
          resetCount = 0;

  
          String currentValue = valueArray[0];
          currentValue.toCharArray(curMessage, currentValue.length() + 1);
          utf8Ascii(curMessage);
          //ersetzeUmlaut(curMessage, currentValue.length() + 1);

          P.displayReset();

          P.displayText(curMessage, PA_CENTER, String(scrollSpeed).toInt(), 10, PA_PRINT, PA_PRINT);
          P.displayAnimate();

          break;
        }

      //BLINKEND EIN ODER MEHR WERTE
      case 7:  {
          resetCount = 0;
          ntpSave=0;

          if (P.displayAnimate())
          {

          // Serial.println(String(loopCountBlink) + "   " +  String(valueCount));
            if (valueCount > 1) {
              loopCountBlink++;
              
              delay(1050);
              String currentValue = valueArray[loopCountBlink - 1];
            //  Serial.println(String(loopCountBlink) + "   " + String(valueCount) + "   " + valueArray[loopCountBlink - 1]);
              currentValue.toCharArray(curMessage, currentValue.length() + 1);
              utf8Ascii(curMessage);
              if (loopCountBlink == valueCount) loopCountBlink = 0;
  
        //   Serial.println(String(loopCountBlink) + "   " +  String(valueCount));
            } else {

              delay(1050);
              String currentValue = valueArray[0];
              currentValue.toCharArray(curMessage, currentValue.length() + 1);
              utf8Ascii(curMessage);
            }



            P.displayReset();
            P.displayText(curMessage, PA_CENTER, String(scrollSpeed).toInt(), String(scrollPause).toInt() * 1000, PA_PRINT, PA_PRINT);
            P.displayAnimate();
            //delay(1250);
          }


          break;
        }

      case 8:  {
          resetCount = 0;
          if (P.displayAnimate())
          {



            loopCount++;
            if (loopCount > valueCount || valueCount == 0) {
              String Zeit = calcTime(now());
              
         if (Zeit == "00 : 00" || (ntpSave)) { if (!zeitHelp) {datum=calcDate(now()); zeitHelp=true; ntpSave=false;}   //wegen DATUM Umstellung   
           } else { zeitHelp=false;}
            switch (datumJa) {
                           case 0: {Zeit.toCharArray(curMessage, 10);break;} 
                           case 1: {Zeit = Zeit + "  -  " +datum;Zeit.toCharArray(curMessage, 20);break;} 
                           case 2: {Zeit = Zeit + "           " +datum; Zeit.toCharArray(curMessage, 26);break;} }
              loopCount = -1;
            } else {
              String currentValue = valueArray[loopCount];
              currentValue.toCharArray(curMessage, currentValue.length() + 1);
              utf8Ascii(curMessage);

            }
            //P.displayReset();
            P.displayText(curMessage, scrollAlign, 150, 3000, scrollEffectDown, scrollEffectDown);
            P.displayAnimate();
          }


          break;
        }

      case 9: {
        ntpSave=0;
          resetCount = 0;
         
          P.displayReset();
          P.displayShutdown(true);
          shutty = true;
          break;
        }
        
        //BLINKEND EIN ODER MEHR WERTE
  case 10:  {
          resetCount = 0;
          ntpSave=0;

          if (P.displayAnimate())
          {

           
            if (valueCount > 1) {
              loopCountBlink++;
              delay(1050);
              String currentValue = valueArray[loopCountBlink - 1];
              currentValue.toCharArray(curMessage, currentValue.length() + 1);
              utf8Ascii(curMessage);
              if (loopCountBlink == valueCount) loopCountBlink = 0;

            } else {

              delay(1050);
              String currentValue = valueArray[0];
              currentValue.toCharArray(curMessage, currentValue.length() + 1);
              utf8Ascii(curMessage);
            }



            P.displayReset();
            P.displayText(curMessage, PA_CENTER, 70, String(scrollPause).toInt() * 1000, PA_GROW_UP, PA_GROW_UP);
            P.displayAnimate();
            //delay(1250);
          }


          break;
        }

      case 11:  {
          resetCount = 0;
          if (P.displayAnimate())
          {

            loopCount++;
            if (loopCount > valueCount || valueCount == 0) {
              String Zeit = calcTime(now());
         if (Zeit == "00 : 00" || (ntpSave)) { if (!zeitHelp) {datum=calcDate(now()); zeitHelp=true; ntpSave=false;}   //wegen DATUM Umstellung   
           } else { zeitHelp=false;}
             
            switch (datumJa) {
                           case 0: {Zeit.toCharArray(curMessage, 10);break;} 
                           case 1: {Zeit = Zeit + "  -  " +datum;Zeit.toCharArray(curMessage, 20);break;} 
                           case 2: {Zeit = Zeit + "           " +datum; Zeit.toCharArray(curMessage, 26);break;} }
              
         
              loopCount = -1;
            } else {
              String currentValue = valueArray[loopCount];
              currentValue.toCharArray(curMessage, currentValue.length() + 1);
              utf8Ascii(curMessage);


            }
            // //memset(valueArray,0,sizeof(valueArray));
            P.displayReset();
            P.displayText(curMessage, scrollAlign, 80, String(scrollPause).toInt() * 1000, PA_SCROLL_DOWN_RIGHT, PA_GROW_UP);
            // P.displayText(curMessage, PA_LEFT, String(scrollSpeed).toInt(), 10, PA_PRINT, PA_PRINT);
            P.displayAnimate();
          }

          break;
        }    

      case 12:  {
          resetCount = 0;
          if (P.displayAnimate())
          {

            loopCount++;
            if (loopCount > valueCount || valueCount == 0) {
              String Zeit = calcTime(now());
         if (Zeit == "00 : 00" || (ntpSave)) { if (!zeitHelp) {datum=calcDate(now()); zeitHelp=true; ntpSave=false;}   //wegen DATUM Umstellung   
           } else { zeitHelp=false;}
             
            switch (datumJa) {
                           case 0: {Zeit.toCharArray(curMessage, 10);break;} 
                           case 1: {Zeit = Zeit + "  -  " +datum;Zeit.toCharArray(curMessage, 20);break;} 
                           case 2: {Zeit = Zeit + "           " +datum; Zeit.toCharArray(curMessage, 26);break;} }
              
         
              loopCount = -1;
            } else {
              String currentValue = valueArray[loopCount];
              currentValue.toCharArray(curMessage, currentValue.length() + 1);
              utf8Ascii(curMessage);


            }
            // //memset(valueArray,0,sizeof(valueArray));
            P.displayReset();
            P.displayText(curMessage, scrollAlign, 80, String(scrollPause).toInt() * 1000, PA_SCROLL_UP_RIGHT, PA_SCROLL_DOWN_RIGHT);//PA_GROW_UP
            // P.displayText(curMessage, PA_LEFT, String(scrollSpeed).toInt(), 10, PA_PRINT, PA_PRINT);
            P.displayAnimate();
          }

          break;
        }  

      default: {
               }
    }
      

  } else
  {
    digitalWrite(16, LOW);
    resetCount++;
    if (resetCount >50000 ) {
      resetCount = 0;
      ESP.restart();
    }
   
     // Serial.println("errorCode:  " + String(errorCode) + "modus: " + String(modus));
   
     switch (errorCode) {
          case 0: { errorMessage="! ERROR ! REF TIME" ;break; }
          case 1: { errorMessage="! ERROR ! BAD INTENSTITY" ;break; }
          case 2: { errorMessage="! ERROR ! SCR SPEED" ;break; }
          case 3: { errorMessage="! ERROR ! SCR PAUSE" ;break; }
          case 4: { errorMessage="! ERROR ! REF TIME" ;break; }
          case 5: { errorMessage="! ERROR ! BAD MODE" ;break; }
          case 6: { errorMessage="! ERROR ! HTTP CON" ;break; }
         
         }
        //  errorCode=0;
      if (P.displayAnimate()) {
      Serial.println(errorMessage);
      errorMessage.toCharArray(curMessage, errorMessage.length() + 1);
      utf8Ascii(curMessage);
      //  Serial.println(String(curMessage) + errorMessage + String(errorMessage.length() +1));
      P.displayReset();
      P.displayText(curMessage, PA_LEFT, 25, 10, scrollEffectIn, scrollEffectIn);
      P.displayAnimate();

    }
  }
}
   


//--------------------------------------------------------------------------------------------------------------------

String loadDataFromURL() {

  Serial.println("ntpSave :" + String(ntpSave) + "   zeitHelp :  " + String(zeitHelp));
  
  if (WiFi.status() == WL_CONNECTED) {
    
    HTTPClient http;
    http.setTimeout(3000);
    Serial.println("getState url: " + String(url));
 //http.setReuse(true);
    http.begin(url);
    int httpCode = http.GET();
   String payload = "error";
    if (httpCode > 0) {
      payload = http.getString();
    }
    if (httpCode != 200) {
      Serial.println("HTTP " + String(url) + String(httpCode) + " fail");
      payload = "HTTP ERROR";
    }

     http.end();
    
  
    if (payload.indexOf("</ret>") > 0) {
      payload = payload.substring(payload.indexOf("<ret>"));
      payload = payload.substring(5, payload.indexOf("</ret>"));
    } else {
      payload = payload.substring(1, payload.length() - 1);
    }
    Serial.println("getState payload = " + payload);
 //  Serial.println(ESP.getFreeHeap(),DEC);
  // Serial.println(String(sizeof(valueArray)));
    return payload;
  } else
    Serial.println("RESTART URL");
  ESP.restart();
}

//--------------------------------------------------------------------------------------------------------------------

String configSettingFromURL() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.setTimeout(3000);
//http.setReuse(true);
    String url1 = url;
    http.begin(url1+ "Setting");

    int httpCode = http.GET();
    String payload = "error";
    if (httpCode > 0) {
      payload = http.getString();
    }
    if (httpCode != 200) {
      Serial.println("Setting " + String(url1) + String(httpCode) + " fail");
      payload = "HTTP ERROR";
    }
    http.end();


   // Serial.println("Config Setting = " + payload);
    payload.replace("\"", "");
    // payload.toCharArray(Speed, payload.length() +1);
    // int xxx = payload.toInt();
    //return xxx;
    // if (payload == "") payload="30";
    return payload;
  } else Serial.println("RESTART MODE");
  ESP.restart();
}

//-----------------------NICHT MEHR BENUTZT----------------------------------------------------


//--------------------------------------------------------------------------------------------------------------------



//--------------------------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------------------------
// WIRD NOCH GETESTET !!
String setModeToURL(String modeOut) {
  if (WiFi.status() == WL_CONNECTED) {


   // Serial.println("ModeOut: " + modeOut);
//    String url1 = url;
//    url1 = url1 + "Mode?value=";
//    url1.replace("getPlainValue", "set");
//    url1 = url1 + (String)modeOut;
// 
//    Serial.println(url1);
    HTTPClient http;
    http.setTimeout(3000);
   
    http.begin("http://192.168.178.59:8087/set/controll-own.0.MATRIX.AlarmModeMatrix?value=1");
    // String payload = http.getString();
  
    int httpCode = http.GET();
    Serial.println(httpCode);
     String payload = http.getString();
     Serial.println(payload);
     http.end();
//
//    Serial.println(httpCode);
//     String payload = "error";
//       if (httpCode > 200) {
//         Serial.println("RESTART MODEOUT");
//         ESP.reset();
//       }
//       if (httpCode != 200) {
//        Serial.println("Matrix ModeLoad " + String(url) + " fail");
//          payload = " HTTP ERROR ";
//        }
       
        http.end();
        return payload;
    
  } else Serial.println("RESTART MODEOUT");
  ESP.restart();
}

//--------------------------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------------------------
String configSpeedFromURL() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.setTimeout(3000);

    String url1 = url;
    http.begin(url1 + "ScrollSpeed");

    int httpCode = http.GET();
    String payload = "error";
    if (httpCode > 0) {
      payload = http.getString();
    }
    if (httpCode != 200) {
      Serial.println("scrollSpeed " + String(url) + " fail");
      payload = " HTTP ERROR ";
    }
    http.end();


    Serial.println("Config scrollSpeed = " + payload);
    payload.replace("\"", "");
    // payload.toCharArray(Speed, payload.length() +1);
    // int xxx = payload.toInt();
    //return xxx;
    if (payload == "") payload = "30";
    return payload;
  } else Serial.println("RESTART MODE");
  ESP.restart();
}
