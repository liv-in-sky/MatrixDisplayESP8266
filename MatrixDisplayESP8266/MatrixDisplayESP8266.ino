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
#define MAX_DEVICES   8
#define CLK_PIN       D5
#define DATA_PIN      D7
#define CS_PIN        D8
MD_Parola P = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);
//MD_Parola P = MD_Parola(CS_PIN, MAX_DEVICES);
//uint8_t degC[] = {5, 6 , 15 , 9 , 15 , 6 };
//uint8_t line[] = {4,  0, 8, 8, 8 };
//uint8_t plus[] = {5, 8, 8, 62, 8, 8};

uint8_t arrowup[] =  {5, 8, 4, 126, 4, 8};
uint8_t heart[] = {5, 28, 62, 124, 62, 28};
uint8_t arrowdown[] = {5, 16, 32, 126, 32, 16};
uint8_t arrowright[] = {5, 8, 8, 42, 28, 8};
uint8_t arrowleft[] = {5, 8, 28, 42, 8, 8};


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
int zeitHolen=600000;
String Zeit;
String currentValue;
bool modeAnzeige=false;

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
bool zeitJa=false;

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
{  static uint8_t cPrev;
  uint8_t c = '\0';

  if (ascii < 0x7f)   // Standard ASCII-set 0..0x7F, no conversion
  {  cPrev = '\0';
    c = ascii;
  } else
  { switch (cPrev)  // Conversion depending on preceding UTF8-character
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


// Sprite Definitions

const uint8_t F_PMAN1 = 6;
const uint8_t W_PMAN1 = 8;
//static const uint8_t PROGMEM pacman1[F_PMAN1 * W_PMAN1] =  // gobbling pacman animation
static const uint8_t  pacman1[F_PMAN1 * W_PMAN1] =  // gobbling pacman animation
{
  0x00, 0x81, 0xc3, 0xe7, 0xff, 0x7e, 0x7e, 0x3c,
  0x00, 0x42, 0xe7, 0xe7, 0xff, 0xff, 0x7e, 0x3c,
  0x24, 0x66, 0xe7, 0xff, 0xff, 0xff, 0x7e, 0x3c,
  0x3c, 0x7e, 0xff, 0xff, 0xff, 0xff, 0x7e, 0x3c,
  0x24, 0x66, 0xe7, 0xff, 0xff, 0xff, 0x7e, 0x3c,
  0x00, 0x42, 0xe7, 0xe7, 0xff, 0xff, 0x7e, 0x3c,
};

const uint8_t F_PMAN2 = 6;
const uint8_t W_PMAN2 = 18;
//static const uint8_t PROGMEM pacman2[F_PMAN2 * W_PMAN2] =  // ghost pursued by a pacman
static const uint8_t  pacman2[F_PMAN2 * W_PMAN2] =  // ghost pursued by a pacman
{
  0x00, 0x81, 0xc3, 0xe7, 0xff, 0x7e, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73, 0xfe,
  0x00, 0x42, 0xe7, 0xe7, 0xff, 0xff, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73, 0xfe,
  0x24, 0x66, 0xe7, 0xff, 0xff, 0xff, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x7b, 0xf3, 0x7f, 0xfb, 0x73, 0xfe,
  0x3c, 0x7e, 0xff, 0xff, 0xff, 0xff, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x73, 0xfb, 0x7f, 0xf3, 0x7b, 0xfe,
  0x24, 0x66, 0xe7, 0xff, 0xff, 0xff, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x73, 0xfb, 0x7f, 0xf3, 0x7b, 0xfe,
  0x00, 0x42, 0xe7, 0xe7, 0xff, 0xff, 0x7e, 0x3c, 0x00, 0x00, 0x00, 0xfe, 0x73, 0xfb, 0x7f, 0xf3, 0x7b, 0xfe,
};

void setup() {
    P.begin();
#if ENA_SPRITE
  P.setSpriteData(pacman1, W_PMAN1, F_PMAN1, pacman1, W_PMAN1, F_PMAN1);// pacman2, W_PMAN2, F_PMAN2);
#endif

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
   P.displayReset();
  P.displaySuspend(false);
  //  P.addChar('$', degC);
    P.addChar('}', arrowdown);
    P.addChar('{', arrowup);
   P.addChar('§', heart);
    P.addChar('[', arrowleft);
   P.addChar(']', arrowright);
   
    P.displayText("here we go ...", PA_LEFT, 15, 10, PA_PRINT, PA_PRINT);
   P.displayAnimate();

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
 // P.setIntensity(intensity-1);

     if (MAX_DEVICES <=8 )  datumJa=0;
     if (MAX_DEVICES > 8 && MAX_DEVICES < 16)  datumJa=1;
     if (MAX_DEVICES >=16)  datumJa=2;


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
    delay(1000);
 Serial.println(WiFi.localIP().toString());
          
    switch (datumJa) {    case 0: { P.displayText("IP ok", PA_RIGHT, 5, 2000, PA_PRINT, PA_RANDOM);break;} 
                          case 1: {WiFi.localIP().toString().toCharArray(curMessage, 16);P.displayText(curMessage, PA_RIGHT, 5, 2000, PA_RANDOM, PA_RANDOM);break;} 
                          case 2: { WiFi.localIP().toString().toCharArray(curMessage, 16);P.displayText(curMessage, PA_RIGHT, 5, 2000, PA_RANDOM, PA_RANDOM);break;} }

  //  WiFi.localIP().toString().toCharArray(curMessage, 16);
  //  utf8Ascii(curMessage);
     
     P.displayAnimate();
   
    startOTAhandling();
  }
  else ESP.restart();

delay(1500);
 digitalWrite(16, HIGH);

     if (MAX_DEVICES<=8) { datumJa=0;}
               else if (MAX_DEVICES>8 && MAX_DEVICES<16) {datum=calcDate(now()); datumJa=1;}
               else {datum=calcDate(now()); datumJa=2;}

               Zeit = calcTime(now());
}

  

void loop() {
  ArduinoOTA.handle();
 // delay (1000);
 
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
    if (modus > 21) modus = 1;
//    String yyy = (String)modus;
//    yyy = "Mode: " + yyy;
//    Serial.println(yyy);
//    P.displayShutdown(false);
//    yyy.toCharArray(curMessage, yyy.length() + 1);
//    P.displayText(curMessage, PA_CENTER, 25, 10, PA_PRINT, PA_PRINT);
//    P.displayAnimate();

    delay(500);

   
  }
  String udpMessage = handleUDP();


//-----ANFANG DATEN HOLEN--------------------------

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

    modusLoad = settingsArray[0];
    modus = modusLoad.toInt();
    if (modusAlt != modus) {shutty = false; ntpSave = true;loopCount=-1;zeitHolen=50000;modeAnzeige=false;
    Serial.println("==========================================" + String(modus) + " ALT:  " + String(modusAlt));
    do {
       if (P.displayAnimate()) {
       String  yyy = (String)modus;
       yyy = "Mode: " + yyy;
       Serial.println("=====================================ANZEIGE=====" + yyy);
       yyy.toCharArray(curMessage, yyy.length() + 1);
        P.displayReset();
       P.displayText(curMessage, PA_CENTER, 25, 1500, PA_PRINT, PA_PRINT);
       P.displayAnimate();
       digitalWrite(16, LOW);
       modeAnzeige=true;} else {delay(5);Serial.println("======================================DELAY 10====" + String(modus) + " ALT:  " + String(modusAlt));}
  } while (!modeAnzeige);
    
    }   //für Datumabfrage und display shutdown
    modusAlt = modus;
    Serial.println( "Modus von IOBROKER ist : " + String(modus));

    
    
    
    if (modus < 1 || modus > 21) {
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
    if (check < 1 || check > 300) {
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



    String valueString = loadDataFromURL(); //"123;123;456";
    if (valueString != "HTTP ERROR"){
      
    char buf[valueString.length() + 1];
    valueString.toCharArray(buf, sizeof(buf));
    char *p = buf;
    char *str;
    valueCount = 0;
    while ((str = strtok_r(p, ";", &p)) != NULL) {
      valueArray[valueCount] = str;
      valueCount++; }

    } else {
      errorCode = 6;
      modus = 99;}


    
    lastMillis = millis();

  }
//-----ENDE DATEN HOLEN--------------------------

    if (valueCount != valueCountBefore){
          valueCountBefore=valueCount;
          loopCount=-1;
         
    }

//Speziell für Mode 7 und 10
if (ntpSave) {
  if (modus != 7 ) loopCountBlink = 0;
  if (modus != 10 ) loopCountBlink = 0;}

  if ( modus==3 || modus==6 || modus==7 || modus==9 || modus==10 ) { zeitJa=false; } else {zeitJa=true;} //modus==2 ||
 // Serial.println("ZeitJa bei Zeit-Need :  " +String(zeitJa));
 // zeitJa=true;
  if (!shutty) P.displayShutdown(false);
  if (modus != 99) {
    digitalWrite(16, HIGH);

if (zeitJa) {
          resetCount = 0;
          zeitHolen++;
          
          if (P.displayAnimate()) {
             loopCount++;
            if (loopCount == valueCount || valueCount == 0) {            //zeitabholung abhängig von Datengröße btw. scrollspeed bzw. 
              if (zeitHolen>1) {
              Zeit = calcTime(now());zeitHolen=0;Serial.println("Zeitholen RESET------------------------------------------");}
              if (Zeit == "00 : 00" || (ntpSave)) { if (!zeitHelp) {datum=calcDate(now()); zeitHelp=true; ntpSave=false;}}   //wegen DATUM Umstellung   
              if (Zeit != "00 : 00" || (!ntpSave)) zeitHelp=false;
              switch (datumJa) {
                           case 0: {Zeit.toCharArray(curMessage, 10);break;} 
                           case 1: {Zeit = Zeit + "  -  " +datum;Zeit.toCharArray(curMessage, 20);break;} 
                           case 2: {Zeit = Zeit + "           " +datum; Zeit.toCharArray(curMessage, 26);break;} }
              loopCount = -1;
            } else {
              currentValue = valueArray[loopCount];
              currentValue.toCharArray(curMessage, currentValue.length() + 1);
              utf8Ascii(curMessage);}  
           }
        //   P.displayReset();
}

    
    switch (modus) {

      case 0:  {  //NOT IN USE durch Fehlerabfrage Modus
             resetCount++;
             if (resetCount >40000 ) {
             resetCount = 0;
             ESP.restart();}   
             if (P.displayAnimate()) {
             errorMessage = "! ERROR ! bad Mode!";
             errorMessage.toCharArray(curMessage, errorMessage.length() + 1);
             P.displayReset();
             P.displayText(curMessage, PA_LEFT, 25, 10, scrollEffectIn, scrollEffectIn);
             P.displayAnimate();
               }
       break; }
      
      case 1:  {

            if (P.displayAnimate()) {
            P.displayReset();
            P.displayText(curMessage, scrollAlign, String(scrollSpeed).toInt(), String(scrollPause).toInt() * 1000, scrollEffectIn, scrollEffectUp);
            P.displayAnimate();}
  
      break;}

      case 2:  {
           resetCount = 0;
           zeitHolen++;
            if (zeitHolen>37000) {
           Zeit = calcTime(now());zeitHolen=0;Serial.println("Zeitholen RESET------------------------------------------");}
           //Serial.println(String(now()));
           if (Zeit == "00 : 00" || (ntpSave)) { if (!zeitHelp) {datum=calcDate(now()); zeitHelp=true; ntpSave=false;Serial.println("Tag holen : " +datum);}}   //wegen DATUM Umstellung   
           if (Zeit != "00 : 00" || (!ntpSave)) zeitHelp=false;  //Serial.println("zeithelp im case : " +String(zeitHelp));

           switch (datumJa) {
                           case 0: {Zeit.toCharArray(curMessage, 10);break;} 
                           case 1: {Zeit = Zeit + "  -  " +datum;Zeit.toCharArray(curMessage, 20);break;} 
                           case 2: {Zeit = Zeit + "           " +datum; Zeit.toCharArray(curMessage, 26);break;} }
                         //  if (P.displayAnimate()) {  
           P.displayReset();
           P.displayText(curMessage, PA_CENTER, String(scrollSpeed).toInt(), 10, PA_PRINT, PA_PRINT);
           P.displayAnimate();//}
           break;}
      
      case 3:  {
          resetCount = 0;
          ntpSave=0;
          if (P.displayAnimate())
          {
 
            loopCount++;
            if (loopCount == valueCount || valueCount == 0) {
            //  String Zeit = calcTime(now());
            //  Zeit.toCharArray(curMessage, 10);
              loopCount = -1;
            } else {
              String currentValue = valueArray[loopCount];
              currentValue.toCharArray(curMessage, currentValue.length() + 1);
              utf8Ascii(curMessage); }
            
            P.displayReset();
            P.displayText(curMessage, scrollAlign, String(scrollSpeed).toInt(), String(scrollPause).toInt() * 1000, scrollEffectIn, scrollEffectIn);
            P.displayAnimate();}
          break;}


      case 4:  {

          if (P.displayAnimate()) {
            P.displayReset();
            P.displayText(curMessage, scrollAlign, String(scrollSpeed).toInt(), String(scrollPause).toInt() * 1000, scrollEffectIn, scrollEffectRight);
            P.displayAnimate();}
      break;}    

      case 5:  {

          if (P.displayAnimate()) {     
            P.displayReset();
            P.displayText(curMessage, scrollAlign, String(scrollSpeed).toInt(), String(scrollPause).toInt() * 1000, scrollEffectIn, scrollEffectIn);
            P.displayAnimate();
          }
      break;}    
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

          break; }

      //BLINKEND EIN ODER MEHR WERTE
      case 7:  {
          resetCount = 0;
          ntpSave=0;

          if (P.displayAnimate()) {

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
              utf8Ascii(curMessage); }

            P.displayReset();
            P.displayText(curMessage, PA_CENTER, String(scrollSpeed).toInt(), String(scrollPause).toInt() * 1000, PA_PRINT, PA_PRINT);
            P.displayAnimate(); }

          break;}

      case 8:  {

          if (P.displayAnimate()) {     
            P.displayReset();
            P.displayText(curMessage, scrollAlign, 150, 3000, scrollEffectDown, scrollEffectDown);
            P.displayAnimate();
          }
      break;}

      case 9: {
//         if (P.displayAnimate()) {
//            curMessage[0] = char(13);
//           P.displayText(curMessage, scrollAlign, 11, 3000, scrollEffectDown, scrollEffectDown);
//           P.displayAnimate();}
        
          ntpSave=0;
          resetCount = 0;
         if (P.displayAnimate()) {  
          P.displayReset();
          P.displayShutdown(true);
          shutty = true;}
          break;}
        
        
  case 10:  {
          resetCount = 0;
          ntpSave=0;

          if (P.displayAnimate()) {
   
            if (valueCount > 1) {
              loopCountBlink++;
              delay(1050);
              String currentValue = valueArray[loopCountBlink - 1];
              currentValue.toCharArray(curMessage, currentValue.length() + 1);
              utf8Ascii(curMessage);
              if (loopCountBlink == valueCount) loopCountBlink = 0;

            } else {  delay(1050);
                      String currentValue = valueArray[0];
                      currentValue.toCharArray(curMessage, currentValue.length() + 1);
                      utf8Ascii(curMessage);}
              
              P.displayReset();
              P.displayText(curMessage, PA_CENTER, 70, String(scrollPause).toInt() * 1000, PA_GROW_UP, PA_GROW_UP);
              P.displayAnimate();}
 
          break;}

      case 11:  {

          if (P.displayAnimate()) {     
            P.displayReset();
            P.displayText(curMessage, scrollAlign, 80, String(scrollPause).toInt() * 1000, PA_SCROLL_DOWN_RIGHT, PA_GROW_UP);
            P.displayAnimate();
          }
      break;}    

      case 12:  {

          if (P.displayAnimate()) {     
            P.displayReset();
            P.displayText(curMessage, scrollAlign, 80, String(scrollPause).toInt() * 1000, PA_SCROLL_UP_RIGHT, PA_SCROLL_DOWN_RIGHT);
            P.displayAnimate();
          }
      break;}  

      case 13:  {

          if (P.displayAnimate()) {     
            P.displayReset();
            P.displayText(curMessage, scrollAlign, 80, String(scrollPause).toInt() * 1000, PA_MESH, PA_MESH);
            P.displayAnimate();
          }
      break;}    

      case 14:  {
          if (P.displayAnimate()) {  
            P.displayReset();
            P.displayText(curMessage, scrollAlign, String(scrollSpeed).toInt(), String(scrollPause).toInt() * 1000, PA_OPENING, PA_CLOSING);
            P.displayAnimate();
          }
      break;}  

      case 15:  {

          if (P.displayAnimate()) {     
            P.displayReset();
            P.displayText(curMessage, scrollAlign, String(scrollSpeed).toInt(), String(scrollPause).toInt() * 1000, PA_WIPE_CURSOR, PA_WIPE_CURSOR);
            P.displayAnimate();
          }
      break;} 

      case 16:  {

          if (P.displayAnimate()) {     
            P.displayReset();
            P.displayText(curMessage, scrollAlign, String(scrollSpeed).toInt(), String(scrollPause).toInt() * 1000, PA_RANDOM, PA_SPRITE);
            P.displayAnimate();
          }
      break;} 
      
      case 17:  {

          if (P.displayAnimate()) {     
            P.displayReset();
            P.displayText(curMessage, scrollAlign, String(scrollSpeed).toInt(), String(scrollPause).toInt() * 1000, PA_SPRITE, PA_SPRITE);
            P.displayAnimate();
          }
      break;}
        
  
      case 18:  {

          if (P.displayAnimate()) {     
            P.displayReset();
            P.displayText(curMessage, scrollAlign, 90, String(scrollPause).toInt() * 1000, PA_DISSOLVE, PA_DISSOLVE);
            P.displayAnimate();
          }
      break;}
      

      case 19:  {

          if (P.displayAnimate()) {     
            P.displayReset();
            P.displayText(curMessage, scrollAlign, String(scrollSpeed).toInt(), String(scrollPause).toInt() * 1000, PA_CLOSING_CURSOR, PA_CLOSING_CURSOR);
            P.displayAnimate();
          }
      break;}

      case 20:  {

          if (P.displayAnimate()) {     
            P.displayReset();
            P.displayText(curMessage, scrollAlign, 50, String(scrollPause).toInt() * 1000, PA_BLINDS, PA_BLINDS);
            P.displayAnimate();
          }
      break;}      

      case 21:  {

          if (P.displayAnimate()) {     
            P.displayReset();
            P.displayText(curMessage, scrollAlign, String(scrollSpeed).toInt(), String(scrollPause).toInt() * 1000, PA_RANDOM, PA_RANDOM);
            P.displayAnimate();
         }
      break;}      

 
    }  

  } else
  {
    digitalWrite(16, LOW);
    resetCount++;
    if (resetCount >50000 ) {
      resetCount = 0;
      ESP.restart();
    }
   
     
   
     switch (errorCode) {
          case 0: { errorMessage="REFRESH DATA !ERROR!" ;break; }
          case 1: { errorMessage="BAD INTENSTITY !ERROR!" ;break; }
          case 2: { errorMessage="SCR SPEED !ERROR!" ;break; }
          case 3: { errorMessage="SCR PAUSE !ERROR!" ;break; }
          case 4: { errorMessage="REF TIME !ERROR!" ;break; }
          case 5: { errorMessage="BAD MODE !ERROR!" ;break; }
          case 6: { errorMessage="HTTP CON !ERROR!" ;break; }
         
         }
      
      if (P.displayAnimate()) {
      Serial.println(errorMessage);
      errorMessage.toCharArray(curMessage, errorMessage.length() + 1);
      utf8Ascii(curMessage);
      //  Serial.println(String(curMessage) + errorMessage + String(errorMessage.length() +1));
      P.displayReset();
      switch (datumJa) {
                           case 0: {P.displayText(curMessage, PA_LEFT, 20, 3000, scrollEffectIn, scrollEffectIn);;break;} 
                           case 1: {P.displayText(curMessage, PA_LEFT, 20, 3000, scrollEffectIn, PA_BLINDS);;break;} 
                           case 2: {P.displayText(curMessage, PA_LEFT, 20, 3000, scrollEffectIn, PA_BLINDS);;break;} }
       P.displayAnimate(); }
  }
}
   


//--------------------------------------------------------------------------------------------------------------------

String loadDataFromURL() {

  Serial.println("ntpSave :" + String(ntpSave) + "   zeitHelp :  " + String(zeitHelp));
  
  if (WiFi.status() == WL_CONNECTED) {
    
    HTTPClient http;
    http.setTimeout(3000);
    Serial.println("getState url: " + String(url));
    http.begin(url);
    int httpCode = http.GET();
    String payload = "error";
    if (httpCode > 0) {
       payload = http.getString();
    }
    if (httpCode != 200) {
      Serial.println("HTTP " + String(url) + String(httpCode) + " Daten fail");
      payload = "HTTP ERROR";
    }

     http.end();
 //  payload.replace("\"", "");
  
    if (payload.indexOf("</ret>") > 0) {
      payload = payload.substring(payload.indexOf("<ret>"));
      payload = payload.substring(5, payload.indexOf("</ret>"));
    } else {
      payload = payload.substring(1, payload.length() - 1);
    }

    payload.replace("\\\\", "\\");
    payload.replace("\\\"", "\"");

   if  (payload.indexOf("#left")  >= 0)  {P.addChar('[', arrowleft);  payload.replace("#left", "[");}  else {P.delChar('[');}
   if  (payload.indexOf("#up")    >= 0)  {P.addChar('{', arrowup);    payload.replace("#up", "{");}  else { P.delChar('{'); }
   if  (payload.indexOf("#down")  >= 0)  {P.addChar('}', arrowdown);  payload.replace("#down", "}");}  else {P.delChar('}'); }
   if  (payload.indexOf("#right") >= 0)  {P.addChar(']', arrowright); payload.replace("#right", "]");}  else { P.delChar(']'); }
   if  (payload.indexOf("#heart") >= 0)  {P.addChar('§', heart);      payload.replace("#heart", "§");}  else {P.delChar('§'); }

   if  (payload.indexOf("#heart") >= 0) Serial.println("AAAAhhhhhh");
  
    Serial.println("getState payload = " + payload) + payload.indexOf("#heart");
Serial.println(ESP.getFreeHeap(),DEC);
    return payload;
  } else {
    Serial.println("RESTART WIFI");
  ESP.restart();
}}

//--------------------------------------------------------------------------------------------------------------------

String configSettingFromURL() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.setTimeout(3000);
    String url1 = url;
    http.begin(url1+ "Setting");

    int httpCode = http.GET();
    String payload = "error";
    if (httpCode > 0) {
      payload = http.getString();
    }
    if (httpCode != 200) {
      Serial.println("Setting " + String(url1) + String(httpCode) + " Setting fail");
      payload = "HTTP ERROR";
    }
    http.end();

    payload.replace("\"", "");

    return payload;
  } else {Serial.println("RESTART WIFI");
  ESP.restart();
}}

//-----------------------NICHT MEHR BENUTZT----------------------------------------------------


// WIRD NOCH GETESTET - WEMOS SENDET!!
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
//String configSpeedFromURL() {
//  if (WiFi.status() == WL_CONNECTED) {
//    HTTPClient http;
//    http.setTimeout(3000);
//
//    String url1 = url;
//    http.begin(url1 + "ScrollSpeed");
//
//    int httpCode = http.GET();
//    String payload = "error";
//    if (httpCode > 0) {
//      payload = http.getString();
//    }
//    if (httpCode != 200) {
//      Serial.println("scrollSpeed " + String(url) + " fail");
//      payload = " HTTP ERROR ";
//    }
//    http.end();
//
//
//    Serial.println("Config scrollSpeed = " + payload);
//    payload.replace("\"", "");
//
//    if (payload == "") payload = "30";
//    return payload;
//  } else Serial.println("RESTART MODE");
//  ESP.restart();
//}
