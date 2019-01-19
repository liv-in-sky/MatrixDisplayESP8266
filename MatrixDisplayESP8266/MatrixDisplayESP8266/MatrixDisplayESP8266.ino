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

textEffect_t scrollEffectIn  = PA_SCROLL_LEFT;
textEffect_t scrollEffectOut = PA_SCROLL_UP;
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
//uint8_t block[] =  {3, 255, 255, 255};
//uint8_t heart[] = {5, 28, 62, 124, 62, 28};

char curMessage[75];

int loopCount = 0;
int loopCountBlink =0;
int cnt = 0;
long lastMillis = 0;
String valueArray[20];
int valueCount = 0;
int intensity = 0;
int modeCnt = 0;
byte timeSetTryCount = 0;

//settingString ist variable für laufen: festehend ist aus - laufen ist ein
String settingString = "\"nein\"";
String settingStringAlt;
bool schalten = false;
int modus = 2;
int modusAlt =9 ;
int modusCase=2;
bool shutty = false;

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
    case 0x82: if (ascii==0xAC) c = 0x80; // Euro symbol special case
    }
    cPrev = ascii;   // save last char
  }

  PRINTX("\nConverted 0x", ascii);
  PRINTX(" to 0x", c);

  return(c);
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
  Serial.begin(115200);
    PRINTS("\n[Parola UTF-8 display Test]");
  // Do one time in-place conversion of the strings to be displayed
  for (uint8_t i=0; i<ARRAY_SIZE(pc); i++)
    utf8Ascii(pc[i]);

  // Initialise the Parola library
  P.begin();
  P.setInvert(false);
  P.setPause(PAUSE_TIME);
  P.setFont(ExtASCII);

  pinMode(key1, INPUT_PULLUP);
  pinMode(key2, INPUT_PULLUP);
  P.begin();
  P.setIntensity(intensity);
  P.displayClear();
  P.displaySuspend(false);
//  P.addChar('$', degC);
//  P.addChar('-', line);
//  P.addChar('_', block);
//  P.addChar('§', heart);
  P.displayText("run ...", PA_LEFT, 25, 10, PA_PRINT, PA_PRINT);
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
  P.setIntensity(intensity);

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
    P.displayText(curMessage, scrollAlign, String(scrollSpeed).toInt(), String(scrollPause).toInt() * 1000, scrollEffectIn, scrollEffectOut);
    startOTAhandling();
  }
  else ESP.restart();
}

void loop() {
  ArduinoOTA.handle();
  if (digitalRead(key1) == LOW ) {
    if (!key1last) {
      key1last = true;
      intensity = intensity + 2;
      if (intensity > 14) intensity = 0;
      P.setIntensity(intensity);
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
    if (modus > 7) modus=0;
    String yyy =(String)modus;
    yyy = "Mode: " + yyy;
    Serial.println(yyy);
    P.displayShutdown(false);
    yyy.toCharArray(curMessage, yyy.length() +1);
    P.displayText(curMessage, PA_CENTER, 25, 10, PA_PRINT, PA_PRINT);
    P.displayAnimate();
    
    delay(500);
  }   
  String udpMessage = handleUDP();

  if (((millis() - lastMillis > String(refreshSeconds).toInt() * 1000) || lastMillis == 0 || udpMessage == "update") && String(url) != "") {
    Serial.println("Fetching data from URL...");
    String valueString = "";
    memset(valueArray,0,sizeof(valueArray));
    valueString = loadDataFromURL();
    
   
    modus = loadModeFromURL();
    if (modusAlt != modus) shutty=false;
    modusAlt=modus;
    Serial.println( "Modus von IOBROKER ist : " + String(modus));

    intensity = loadIntensityFromURL();
    Serial.println( "Intensity von IOBROKER ist : " + String(intensity));
    P.setIntensity(intensity);
    
    char buf[valueString.length() + 1];
    valueString.toCharArray(buf, sizeof(buf));
    Serial.println("buf : " + String(buf));
    char *p = buf;
    char *str;
    valueCount = 0;
    while ((str = strtok_r(p, ";", &p)) != NULL) {
      valueArray[valueCount] = str;
      valueCount++;
    }
    memset(buf,0,sizeof(buf));
   
   // Serial.println("buf : " + String(buf));
   Serial.println("valuecCount  : " + String(valueCount));
    lastMillis = millis();
  }

    if (modus != 7) loopCountBlink=0;  
    if (!shutty) P.displayShutdown(false);
   switch (modus) {

    case 0:  {
             P.displayShutdown(true);
             shutty = true;
              break;
             }

    case 1:  {
             if (P.displayAnimate())
    {
        
      loopCount++;
      if (loopCount > valueCount || valueCount == 0) {
        String Zeit = calcTime(now());
        Zeit.toCharArray(curMessage, 10);
        loopCount = -1;
      } else {
        String currentValue = valueArray[loopCount];
        currentValue.toCharArray(curMessage, currentValue.length() + 1);
        utf8Ascii(curMessage);
      
      
      }
      //memset(valueArray,0,sizeof(valueArray));
      P.displayReset();
      P.displayText(curMessage, scrollAlign, String(scrollSpeed).toInt(), String(scrollPause).toInt() * 1000, scrollEffectIn, scrollEffectOut);
       P.displayAnimate();
    }    
  
              break;
             }
         
    case 2:  {
             String Zeit = calcTime(now());
             Zeit.toCharArray(curMessage, 10);
             P.displayReset();
             P.displayText(curMessage, PA_CENTER, String(scrollSpeed).toInt(), 10, PA_PRINT, PA_PRINT);
             P.displayAnimate();
             break;
             }
             
    case 3:  {
          if (P.displayAnimate())
    {
        
      loopCount++;
      if (loopCount > valueCount || valueCount == 0) {
       // String Zeit = calcTime(now());
       // Zeit.toCharArray(curMessage, 10);
        loopCount = -1;
      } else {
        String currentValue = valueArray[loopCount];
        currentValue.toCharArray(curMessage, currentValue.length() + 1);
        utf8Ascii(curMessage);
      
      
      }
      //memset(valueArray,0,sizeof(valueArray));
      P.displayReset();
      P.displayText(curMessage, scrollAlign, String(scrollSpeed).toInt(), 0, scrollEffectIn, scrollEffectIn);
       P.displayAnimate();
    }    
  
             
  
              break;
             }


      case 4:  {
             if (P.displayAnimate())
    {
   
      
      
      loopCount++;
      if (loopCount > valueCount || valueCount == 0) {
        String Zeit = calcTime(now());
        Zeit.toCharArray(curMessage, 10);
        loopCount = -1;
      } else {
        String currentValue = valueArray[loopCount];
        currentValue.toCharArray(curMessage, currentValue.length() + 1);
        utf8Ascii(curMessage);
      
      }
      //memset(valueArray,0,sizeof(valueArray));
      P.displayReset();
      P.displayText(curMessage, scrollAlign, String(scrollSpeed).toInt(), String(scrollPause).toInt() * 1000, scrollEffectIn, scrollEffectRight);
       P.displayAnimate();
    }    

  
              break;
             }
             
    case 5:  {
      
             if (P.displayAnimate())
    {
   
      loopCount++;
      if (loopCount > valueCount || valueCount == 0) {
        String Zeit = calcTime(now());
        Zeit.toCharArray(curMessage, 10);
        loopCount = -1;
      } else {
        String currentValue = valueArray[loopCount];
        currentValue.toCharArray(curMessage, currentValue.length() + 1);
        utf8Ascii(curMessage);
      
      
      }
      //memset(valueArray,0,sizeof(valueArray));
      P.displayReset();
      P.displayText(curMessage, scrollAlign, String(scrollSpeed).toInt(), 0, scrollEffectIn, scrollEffectIn);
      //P.displayText(curMessage, scrollAlign, String(scrollSpeed).toInt(), String(scrollPause).toInt() * 1000, scrollEffectIn, scrollEffectIn);
       P.displayAnimate();
    }    

              break;
             }
//STEHEND EIN WERT
      case 6:  {

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
      
             if (P.displayAnimate())
    {

      if (valueCount > 1) {
        loopCountBlink++;
        delay(1050);
      String currentValue = valueArray[loopCountBlink-1];
        currentValue.toCharArray(curMessage, currentValue.length() + 1);
        utf8Ascii(curMessage);      
       if(loopCountBlink == valueCount) loopCountBlink=0;
        
      } else {
      
      delay(1050);
        String currentValue = valueArray[0];
        currentValue.toCharArray(curMessage, currentValue.length() + 1);
        utf8Ascii(curMessage); 
      }
      

  
     P.displayReset();
     P.displayText(curMessage, PA_CENTER, String(scrollSpeed).toInt(), 10, PA_PRINT, PA_PRINT);
     P.displayAnimate();
       delay(1250);
    }    

  
              break;
             }

 
    
      default: {
             P.displayText("BAD Mode", PA_CENTER, String(scrollSpeed).toInt(), 10, PA_PRINT, PA_PRINT);
             P.displayAnimate();
             delay(5000);
             P.displayText("RES Mode 5", PA_CENTER, String(scrollSpeed).toInt(), 10, PA_PRINT, PA_PRINT);
             P.displayAnimate();
             break;
      }     
   } 
}





String loadDataFromURL() {
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
      Serial.println("HTTP " + String(url) + " fail");
      payload = " HTTP ERROR ";
    }
    http.end();

    if (payload.indexOf("</ret>") > 0) {
      payload = payload.substring(payload.indexOf("<ret>"));
      payload = payload.substring(5, payload.indexOf("</ret>"));
    } else {
      payload = payload.substring(1, payload.length() - 1);
    }
    Serial.println("getState payload = " + payload);
   
    return payload;
  } else 
        Serial.println("RESTART URL");
        ESP.restart();
}


int loadModeFromURL() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.setTimeout(3000);
 
    String url1 = url;
    http.begin(url1+ "Mode");
    
    int httpCode = http.GET();
    String payload = "error";
    if (httpCode > 0) {
      payload = http.getString();
    }
    if (httpCode != 200) {
      Serial.println("Matrix Setting " + String(url) + " fail");
      payload = " HTTP ERROR ";
    }
    http.end();

   
    Serial.println("getState matrix setting = " + payload);
    payload.replace("\"", "");
    int xxx = payload.toInt();
    return xxx;
  } else Serial.println("RESTART MODE");
      ESP.restart();
}
      int loadIntensityFromURL() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.setTimeout(3000);
   
    String url1 = url;
    http.begin(url1+ "Intensity");
    
    int httpCode = http.GET();
    String payload = "error";
    if (httpCode > 0) {
      payload = http.getString();
    }
    if (httpCode != 200) {
      Serial.println("Matrix Setting " + String(url) + " fail");
      payload = " HTTP ERROR ";
    }
    http.end();

   
    Serial.println("getState matrix setting = " + payload);
    payload.replace("\"", "");
    int xxx = payload.toInt();
    if (xxx < 0 || xxx > 14) xxx=0;
    return xxx;
  } else Serial.println("RESTART INTENSITY");
      ESP.restart();
}

// WIRD NOCH GETESTET !!
      void setModeToURL(int modeOut) {
  if (WiFi.status() == WL_CONNECTED) {
   
   
    Serial.println("ModeOut: " + modeOut);
    String url1 = url;
    url1= url1 + "Mode?value=";
    url1.replace("getPlainValue", "set");
    url1=url1 + (String)modeOut;
    //url1=url1 + "&prettyPrint";
    //http.begin(url1+ "");
    Serial.println(url1);
     HTTPClient http;
    http.setTimeout(3000);
    delay (500);
    
    http.begin(url1);
  //  int httpCode = http.GET();
 //   Serial.println(httpCode);
//  String payload = "error";
//   if (httpCode > 0) {
//     Serial.println("RESTART MODEOUT");
//      ESP.reset();
//    }
//    if (httpCode != 200) {
//     Serial.println("Matrix ModeLoad " + String(url) + " fail");
//      payload = " HTTP ERROR ";
//    }
    delay(350);
    http.end();

   
   
  } else Serial.println("RESTART MODEOUT");
      ESP.restart();
}
