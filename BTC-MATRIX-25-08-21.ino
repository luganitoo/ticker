#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <Hash.h>
#include <FS.h>


AsyncWebServer server(80);

//RGB LED
#define FASTLED_ESP8266_RAW_PIN_ORDER
#include <FastLED.h>
#define LED_PIN 14
#define NUM_LEDS 1
#define BRIGHTNESS  160
CRGB leds[NUM_LEDS];

//CLOCK
#include <NTPClient.h>
#include <WiFiUdp.h>
long utcOffset = -10800; // UTC
// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffset);
char daysOfTheWeek[7][12] = {"Dom", "Seg", "Ter", "Qua", "Qui", "Sex", "Sab"};


// Variables
int statePower = 1;
int customText = 0;
int clockState = 0;
float athPrice = 0;
float intensity = 0;
String hours = "";
String minutes = "";
String currentTime = "";



//MATRIX
#include <LedControl.h>
int numDevices = 4;
LedControl lc = LedControl(0, 4, 5, numDevices); //DATA(D3) | CLK(D2) | CS/LOAD(D1) | number of matrices ->>
long scrollDelay = 50;   // adjust scrolling speed
unsigned long bufferLong [14] = {0};
//const unsigned char scrollText[] PROGMEM ={BTC + " - "};
//const char scrollText[] = "$ 54,786.23 ";

///////////////////HTTP/////////////////////////
// ############# VARIABLES ############### //



String BASE_URL = "";

// ############# PROTOTYPES ############### //

void initSerial();
void initWiFi();
float httpRequest(String path);

// ############### OBJECTS ################# //

BearSSL::WiFiClientSecure client;
const int API_TIMEOUT = 10000;
HTTPClient http;

///////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////// HTML

String    prefixString = "";
String    apiString = "";
String    keyString = "";
String    fullApi = "";

const char* PARAM_CURRENCY_MASTER = "currencyMaster";
const char* PARAM_API_MASTER = "apiMaster";
const char* PARAM_KEY_MASTER = "keyMaster";

const char* PARAM_COIN_LIST = "coinList";
const char* PARAM_CURRENT_COINS = "currentCoins";

const char* PARAM_STRING = "inputString";
const char* PARAM_INT = "slideSpeed";
const char* PARAM_BRIGHTNESS = "slideBrightness";

const char* PARAM_MAIN_HTML = "mainHtml";

const char* PARAM_PREFIX_0 = "inputPrefix0";
const char* PARAM_API_0 = "inputApi0";
const char* PARAM_KEY_0 = "inputKey0";

const char* PARAM_BUZZER = "inputBuzzer";

const char* PARAM_API_LIST = "inputApiList";
const char* PARAM_POWER = "mainPower";
const char* PARAM_TEXT = "customText";
const char* PARAM_CLOCK = "clockState";

const char* PARAM_SSID = "wifiSSID";
const char* PARAM_PASSWORD = "wifiPassword";

//  <form action="/get" target="hidden-form">
//    <b>ALARM VALUE</b> (%inputBuzzer%): <input type="number " name="inputBuzzer">
//    <input type="submit" value="Submit" onclick="submitMessage()">
//  </form><br>


void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}

//////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
//////////////////////////////FUNCTIONS////////////////////////////////

// ############# HTTP REQUEST ################ //

// SPIFFS RUI COSTA
String readFile(fs::FS &fs, const char * path) {
  //  Serial.printf("Reading file: %s\r\n", path);
  File file = fs.open(path, "r");
  if (!file || file.isDirectory()) {
    Serial.println("- empty file or failed to open file");
    return String();
  }
  //  Serial.println("- read from file:");
  String fileContent;
  while (file.available()) {
    fileContent += String((char)file.read());
  }
  //  Serial.println(fileContent);
  return fileContent;
}

String makeRequest(String path)
{
  http.end();
  http.begin(client, BASE_URL + path);
  int httpCode = http.GET();
  Serial.println(httpCode);

  if (httpCode < 0) {
    ESP.restart();
    Serial.println("request error - " + httpCode);
    return "";

  }

  if (httpCode != HTTP_CODE_OK) {
    //return "not ok";
  }
  //Serial.println("request error - " + httpCode);

  String response =  http.getString();
  http.end();
  Serial.println("response->" + response);
  return response;
}

String httpRequest(String path, String key)
{
  String payload = makeRequest(path);

  if (!payload) {
    return ("Not Found");
  }

  Serial.println("##[RESULT]## ==> " + payload);
  DynamicJsonDocument doc(1024);
  deserializeJson(doc, payload);

  //  if(key.indexOf(",") >= 0){
  //    String value = doc[
  //  }
  //  else{
  //    String value = doc[key];
  //  }
  String value = doc[key];
  // RGBLED
  float variation = doc["priceChangePercent"];
  int bri = readFile(SPIFFS, "/slideBrightness.txt").toInt();
  if ( bri == 3 || bri == 7) {
    if (variation < 0) {
      leds[0] = CRGB(255, 0, 0);
      FastLED.show();
    }
    else if (variation < 1) {
      leds[0] = CRGB(255, 255, 0);
      FastLED.show();
    }
    else if (variation < 10) {
      leds[0] = CRGB(0, 255, 0);
      FastLED.show();
    }
    else {
      leds[0] = CRGB(0, 0, 255);
      FastLED.show();
    }
  }
  else {
    leds[0] = CRGB(0, 0, 0);
    FastLED.show();
  }

  //END RGB LED

  Serial.println("##[PRICE]## ==> " + String(value));

  return (value);
}



// ###################################### //

// implementacao dos prototypes

void initSerial() {
  Serial.begin(115200);
}

void initWiFi() {
  delay(10);

  const char *SSID = strdup((readFile(SPIFFS, "/wifiSSID.txt")).c_str());
  const char *PASSWORD = strdup((readFile(SPIFFS, "/wifiPassword.txt")).c_str());
  WiFi.mode(WIFI_STA);
  Serial.println("Conectando-se em: " + String(SSID));

 

  //const char* wifiSSID = strdup((readFile(SPIFFS, "/wifiSSID.txt")).c_str());;
  //const char *wifiPass = strdup((readFile(SPIFFS, "/wifiPassword.txt")).c_str());
  //Serial.println("ERA PRA TA AQUI EMBAIXO");
  //    Serial.println(wifiSSID);
  //    Serial.println(wifiPass);
   //   Serial.println("ERA PRA TA AQUI EM CIMA");

  //  IPAddress ip(192, 168, 15, 26);
  //   IPAddress gateway(192, 168, 15, 1);
  //   IPAddress subnet(255, 255, 255, 0);
  //   IPAddress dnsAdrr(1, 1, 1, 1);
  //   WiFi.config(ip, gateway, subnet, dnsAdrr);

  WiFi.begin(SSID, PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
    Serial.println(WiFi.status());
    if(WiFi.status() == WL_CONNECT_FAILED || WiFi.status() == WL_NO_SSID_AVAIL || WiFi.status() == 6 ||  WiFi.status() == 0){
      WiFi.softAP("CRIPTORAPIT", "rapit.com.br");
      Serial.print("ap mode");
      Serial.println("...---...");
      break;
    }
  }
  
  Serial.println();
  Serial.print("Conectado na Rede " + String(SSID) + " | IP => ");
  Serial.println(WiFi.localIP());
  
}



void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\r\n", path);
  File file = fs.open(path, "w");
  if (!file) {
    Serial.println("- failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
}

// Replaces placeholder with stored values
String processor(const String& var) {
  //Serial.println(var);

  if (var == "inputString") {
    return readFile(SPIFFS, "/inputString.txt");
  }
  else if (var == "slideSpeed") {
    return readFile(SPIFFS, "/slideSpeed.txt");
  }
  else if (var == "slideBrightness") {
    return readFile(SPIFFS, "/slideBrightness.txt");
  }
  else if (var == "inputPrefix0") {
    return readFile(SPIFFS, "/inputPrefix0.txt");
  }
  else if (var == "inputApi0") {
    return readFile(SPIFFS, "/inputApi0.txt");
  }
  else if (var == "inputKey0") {
    return readFile(SPIFFS, "/inputKey0.txt");
  }
  else if (var == "mainHtml") {
    return readFile(SPIFFS, "/mainHtml.txt");
  }
  else if (var == "coinList") {
    return readFile(SPIFFS, "/coinList.txt");
  }
  else if (var == "currentCoins") {
    return readFile(SPIFFS, "/currentCoins.txt");
  }
  else if (var == "currencyMaster") {
    return readFile(SPIFFS, "/currencyMaster.txt");
  }
  else if (var == "apiMaster") {
    return readFile(SPIFFS, "/apiMaster.txt");
  }
  else if (var == "keyMaster") {
    return readFile(SPIFFS, "/keyMaster.txt");
  }
  else if (var == "mainPower") {
    return readFile(SPIFFS, "/customText.txt");
  }
  else if (var == "customText") {
    return readFile(SPIFFS, "/customText.txt");
  }
  else if (var == "clockState") {
    return readFile(SPIFFS, "/clockState.txt");
  }
  else if (var == "wifiSSID") {
    return readFile(SPIFFS, "/wifiSSID.txt");
  }
  else if (var == "wifiPassword") {
    return readFile(SPIFFS, "/wifiPassword.txt");
  }     

  return String();
}

void updateHtml() {
  // To access your stored values on inputString, slideSpeed, slideBrightness
  String yourInputString = readFile(SPIFFS, "/inputString.txt");
  Serial.print("*** Your inputString: ");
  Serial.println(yourInputString);

  int yourslideSpeed = readFile(SPIFFS, "/slideSpeed.txt").toInt();
  Serial.print("*** Your slideSpeed: ");
  Serial.println(yourslideSpeed);

  float yourslideBrightness = readFile(SPIFFS, "/slideBrightness.txt").toFloat();
  Serial.print("*** Your slideBrightness: ");
  Serial.println(yourslideBrightness);

  String yourcurrencyMaster = readFile(SPIFFS, "/currencyMaster.txt");
  Serial.print("*** Your currencyMaster: ");
  Serial.println(yourcurrencyMaster);

  String yourapiMaster = readFile(SPIFFS, "/apiMaster.txt");
  Serial.print("*** Your apiMaster: ");
  Serial.println(yourapiMaster);

  String yourkeyMaster = readFile(SPIFFS, "/keyMaster.txt");
  Serial.print("*** Your keyMaster: ");
  Serial.println(yourkeyMaster);

  String yourcoinList = readFile(SPIFFS, "/coinList.txt");
  Serial.print("*** Your coinList: ");
  Serial.println(yourcoinList);

  String yourcurrentCoins = readFile(SPIFFS, "/currentCoins.txt");
  Serial.print("*** Your currentCoins: ");
  Serial.println(yourcurrentCoins);

  String yourmainHtml = readFile(SPIFFS, "/mainHtml.txt");
  Serial.print("*** Your mainHtml: ");
  Serial.println(yourmainHtml);

  String yourInputPrefix0 = readFile(SPIFFS, "/inputPrefix0.txt");
  Serial.print("*** Your PREFIX_0: ");
  Serial.println(yourInputPrefix0);

  String yourInputApi0 = readFile(SPIFFS, "/inputApi0.txt");
  Serial.print("*** Your API_0: ");
  Serial.println(yourInputApi0);

  String yourInputKey0 = readFile(SPIFFS, "/inputKey0.txt");
  Serial.print("*** Your KEY_0: ");
  Serial.println(yourInputKey0);

  String yourInputApiList = readFile(SPIFFS, "/inputApiList.txt");
  Serial.print("*** Your InputApiList: ");
  Serial.println(yourInputApiList);

  String yourMainPower = readFile(SPIFFS, "/mainPower.txt");
  Serial.print("*** Your MainPower: ");
  Serial.println(yourMainPower);

  String yourCustomText = readFile(SPIFFS, "/customText.txt");
  Serial.print("*** Your customText: ");
  Serial.println(yourCustomText);

  String yourClockState = readFile(SPIFFS, "/clockState.txt");
  Serial.print("*** Your clockState: ");
  Serial.println(yourClockState);

    String yourWifiSSID = readFile(SPIFFS, "/wifiSSID.txt");
  Serial.print("*** Your wifiSSID: ");
  Serial.println(yourWifiSSID);

    String yourWifiPassword = readFile(SPIFFS, "/wifiPassword.txt");
  Serial.print("*** Your wifiPassword: ");
  Serial.println(yourWifiPassword);


}


void setupSPIFFS() {
  // Send web page with input fields to client


  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/style.css", "text/css");
  });
  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/script.js", String(), false, processor);
  });

  // Send a GET request to <ESP_IP>/get?inputString=<inputMessage>
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest * request) {
    String inputMessage;
    // GET inputString value on <ESP_IP>/get?inputString=<inputMessage>
    if (request->hasParam(PARAM_STRING)) {
      inputMessage = request->getParam(PARAM_STRING)->value();
      writeFile(SPIFFS, "/inputString.txt", inputMessage.c_str());
    }
    // GET slideSpeed value on <ESP_IP>/get?slideSpeed=<inputMessage>
    else if (request->hasParam(PARAM_INT)) {
      inputMessage = request->getParam(PARAM_INT)->value();
      writeFile(SPIFFS, "/slideSpeed.txt", inputMessage.c_str());
    }
    // GET slideBrightness value on <ESP_IP>/get?slideBrightness=<inputMessage>
    else if (request->hasParam(PARAM_BRIGHTNESS)) {
      inputMessage = request->getParam(PARAM_BRIGHTNESS)->value();
      writeFile(SPIFFS, "/slideBrightness.txt", inputMessage.c_str());
    }
    // GET inputApi0 value on <ESP_IP>/get?inputApi0=<inputMessage>
    else if (request->hasParam(PARAM_PREFIX_0)) {
      inputMessage = request->getParam(PARAM_PREFIX_0)->value();
      writeFile(SPIFFS, "/inputPrefix0.txt", inputMessage.c_str());
    }
    // GET inputApi0 value on <ESP_IP>/get?inputApi0=<inputMessage>
    else if (request->hasParam(PARAM_API_0)) {
      inputMessage = request->getParam(PARAM_API_0)->value();
      writeFile(SPIFFS, "/inputApi0.txt", inputMessage.c_str());
    }
    // GET inputApi0 value on <ESP_IP>/get?inputApi0=<inputMessage>
    else if (request->hasParam(PARAM_KEY_0)) {
      inputMessage = request->getParam(PARAM_KEY_0)->value();
      writeFile(SPIFFS, "/inputKey0.txt", inputMessage.c_str());
    }
    // GET inputApi0 value on <ESP_IP>/get?inputApi0=<inputMessage>
    else if (request->hasParam(PARAM_API_LIST)) {
      inputMessage = request->getParam(PARAM_API_LIST)->value();
      writeFile(SPIFFS, "/inputApiList.txt", inputMessage.c_str());
    }
    // GET inputApi0 value on <ESP_IP>/get?inputApi0=<inputMessage>
    else if (request->hasParam(PARAM_COIN_LIST)) {
      inputMessage = request->getParam(PARAM_COIN_LIST)->value();
      writeFile(SPIFFS, "/coinList.txt", inputMessage.c_str());
    }

    else if (request->hasParam(PARAM_CURRENT_COINS)) {
      inputMessage = request->getParam(PARAM_CURRENT_COINS)->value();
      writeFile(SPIFFS, "/currentCoins.txt", inputMessage.c_str());
    }

    else if (request->hasParam(PARAM_CURRENCY_MASTER)) {
      inputMessage = request->getParam(PARAM_CURRENCY_MASTER)->value();
      writeFile(SPIFFS, "/currencyMaster.txt", inputMessage.c_str());
    }
    else if (request->hasParam(PARAM_API_MASTER)) {
      inputMessage = request->getParam(PARAM_API_MASTER)->value();
      writeFile(SPIFFS, "/apiMaster.txt", inputMessage.c_str());
    }
    else if (request->hasParam(PARAM_KEY_MASTER)) {
      inputMessage = request->getParam(PARAM_KEY_MASTER)->value();
      writeFile(SPIFFS, "/keyMaster.txt", inputMessage.c_str());
    }
    else if (request->hasParam(PARAM_MAIN_HTML)) {
      inputMessage = request->getParam(PARAM_MAIN_HTML)->value();
      writeFile(SPIFFS, "/mainHtml.txt", inputMessage.c_str());
    }
    else if (request->hasParam(PARAM_POWER)) {
      inputMessage = request->getParam(PARAM_POWER)->value();
      writeFile(SPIFFS, "/mainPower.txt", inputMessage.c_str());
    }
    else if (request->hasParam(PARAM_TEXT)) {
      inputMessage = request->getParam(PARAM_TEXT)->value();
      writeFile(SPIFFS, "/customText.txt", inputMessage.c_str());
    }
    else if (request->hasParam(PARAM_CLOCK)) {
      inputMessage = request->getParam(PARAM_CLOCK)->value();
      writeFile(SPIFFS, "/clockState.txt", inputMessage.c_str());
    }
        else if (request->hasParam(PARAM_SSID)) {
      inputMessage = request->getParam(PARAM_SSID)->value();
      writeFile(SPIFFS, "/wifiSSID.txt", inputMessage.c_str());
    }
        else if (request->hasParam(PARAM_PASSWORD)) {
      inputMessage = request->getParam(PARAM_PASSWORD)->value();
      writeFile(SPIFFS, "/wifiPassword.txt", inputMessage.c_str());
    }
    else {
      inputMessage = "No message sent";
    }
    Serial.println(inputMessage);
    request->send(200, "text/text", inputMessage);
  });
  server.onNotFound(notFound);
  server.begin();
}



////////////////////////////////////////////////////////////////////////////////////////////////////////



//MATRIX
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

const unsigned char font5x7 [] PROGMEM = {      //Numeric Font Matrix (Arranged as 7x font data + 1x kerning data)
  B00000000,  //Space (Char 0x20)
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  3,//cambias el tamaño del espacio entre letras

  B01000000,  //!
  B01000000,
  B01000000,
  B01000000,
  B01000000,
  B00000000,
  B01000000,
  2,

  B10100000,  //"
  B10100000,
  B10100000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  4,

  B01010000,  //#
  B01010000,
  B11111000,
  B01010000,
  B11111000,
  B01010000,
  B01010000,
  6,

  B00100000,  //$
  B01111000,
  B10100000,
  B01110000,
  B00101000,
  B11110000,
  B00100000,
  6,

  B11000000,  //%
  B11001000,
  B00010000,
  B00100000,
  B01000000,
  B10011000,
  B00011000,
  6,

  B01100000,  //&
  B10010000,
  B10100000,
  B01000000,
  B10101000,
  B10010000,
  B01101000,
  6,

  B11000000,  //'
  B01000000,
  B10000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  3,

  B00100000,  //(
  B01000000,
  B10000000,
  B10000000,
  B10000000,
  B01000000,
  B00100000,
  4,

  B10000000,  //)
  B01000000,
  B00100000,
  B00100000,
  B00100000,
  B01000000,
  B10000000,
  4,

  B00000000,  //*
  B00100000,
  B10101000,
  B01110000,
  B10101000,
  B00100000,
  B00000000,
  6,

  B00000000,  //+
  B00100000,
  B00100000,
  B11111000,
  B00100000,
  B00100000,
  B00000000,
  6,

  B00000000,  //,
  B00000000,
  B00000000,
  B00000000,
  B11000000,
  B01000000,
  B10000000,
  3,

  B00000000,  //-
  B00000000,
  B11111000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  6,

  B00000000,  //.
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B11000000,
  B11000000,
  3,

  B00000000,  ///
  B00001000,
  B00010000,
  B00100000,
  B01000000,
  B10000000,
  B00000000,
  6,

  B01110000,  //0
  B10001000,
  B10011000,
  B10101000,
  B11001000,
  B10001000,
  B01110000,
  6,

  B01000000,  //1
  B11000000,
  B01000000,
  B01000000,
  B01000000,
  B01000000,
  B11100000,
  4,

  B01110000,  //2
  B10001000,
  B00001000,
  B00010000,
  B00100000,
  B01000000,
  B11111000,
  6,

  B11111000,  //3
  B00010000,
  B00100000,
  B00010000,
  B00001000,
  B10001000,
  B01110000,
  6,

  B00010000,  //4
  B00110000,
  B01010000,
  B10010000,
  B11111000,
  B00010000,
  B00010000,
  6,

  B11111000,  //5
  B10000000,
  B11110000,
  B00001000,
  B00001000,
  B10001000,
  B01110000,
  6,

  B00110000,  //6
  B01000000,
  B10000000,
  B11110000,
  B10001000,
  B10001000,
  B01110000,
  6,

  B11111000,  //7
  B10001000,
  B00001000,
  B00010000,
  B00100000,
  B00100000,
  B00100000,
  6,

  B01110000,  //8
  B10001000,
  B10001000,
  B01110000,
  B10001000,
  B10001000,
  B01110000,
  6,

  B01110000,  //9
  B10001000,
  B10001000,
  B01111000,
  B00001000,
  B00010000,
  B01100000,
  6,

  B00000000,  //:
  B11000000,
  B11000000,
  B00000000,
  B11000000,
  B11000000,
  B00000000,
  3,

  B00000000,  //;
  B11000000,
  B11000000,
  B00000000,
  B11000000,
  B01000000,
  B10000000,
  3,

  B00010000,  //<
  B00100000,
  B01000000,
  B10000000,
  B01000000,
  B00100000,
  B00010000,
  5,

  B00000000,  //=
  B00000000,
  B11111000,
  B00000000,
  B11111000,
  B00000000,
  B00000000,
  6,

  B10000000,  //>
  B01000000,
  B00100000,
  B00010000,
  B00100000,
  B01000000,
  B10000000,
  5,

  B01110000,  //?
  B10001000,
  B00001000,
  B00010000,
  B00100000,
  B00000000,
  B00100000,
  6,

  B01110000,  //@
  B10001000,
  B00001000,
  B01101000,
  B10101000,
  B10101000,
  B01110000,
  6,

  B01110000,  //A
  B10001000,
  B10001000,
  B10001000,
  B11111000,
  B10001000,
  B10001000,
  6,

  B11110000,  //B
  B10001000,
  B10001000,
  B11110000,
  B10001000,
  B10001000,
  B11110000,
  6,

  B01110000,  //C
  B10001000,
  B10000000,
  B10000000,
  B10000000,
  B10001000,
  B01110000,
  6,

  B11100000,  //D
  B10010000,
  B10001000,
  B10001000,
  B10001000,
  B10010000,
  B11100000,
  6,

  B11111000,  //E
  B10000000,
  B10000000,
  B11110000,
  B10000000,
  B10000000,
  B11111000,
  6,

  B11111000,  //F
  B10000000,
  B10000000,
  B11110000,
  B10000000,
  B10000000,
  B10000000,
  6,

  B01110000,  //G
  B10001000,
  B10000000,
  B10111000,
  B10001000,
  B10001000,
  B01111000,
  6,

  B10001000,  //H
  B10001000,
  B10001000,
  B11111000,
  B10001000,
  B10001000,
  B10001000,
  6,

  B11100000,  //I
  B01000000,
  B01000000,
  B01000000,
  B01000000,
  B01000000,
  B11100000,
  4,

  B00111000,  //J
  B00010000,
  B00010000,
  B00010000,
  B00010000,
  B10010000,
  B01100000,
  6,

  B10001000,  //K
  B10010000,
  B10100000,
  B11000000,
  B10100000,
  B10010000,
  B10001000,
  6,

  B10000000,  //L
  B10000000,
  B10000000,
  B10000000,
  B10000000,
  B10000000,
  B11111000,
  6,

  B10001000,  //M
  B11011000,
  B10101000,
  B10101000,
  B10001000,
  B10001000,
  B10001000,
  6,

  B10001000,  //N
  B10001000,
  B11001000,
  B10101000,
  B10011000,
  B10001000,
  B10001000,
  6,

  B01110000,  //O
  B10001000,
  B10001000,
  B10001000,
  B10001000,
  B10001000,
  B01110000,
  6,

  B11110000,  //P
  B10001000,
  B10001000,
  B11110000,
  B10000000,
  B10000000,
  B10000000,
  6,

  B01110000,  //Q
  B10001000,
  B10001000,
  B10001000,
  B10101000,
  B10010000,
  B01101000,
  6,

  B11110000,  //R
  B10001000,
  B10001000,
  B11110000,
  B10100000,
  B10010000,
  B10001000,
  6,

  B01111000,  //S
  B10000000,
  B10000000,
  B01110000,
  B00001000,
  B00001000,
  B11110000,
  6,

  B11111000,  //T
  B00100000,
  B00100000,
  B00100000,
  B00100000,
  B00100000,
  B00100000,
  6,

  B10001000,  //U
  B10001000,
  B10001000,
  B10001000,
  B10001000,
  B10001000,
  B01110000,
  6,

  B10001000,  //V
  B10001000,
  B10001000,
  B10001000,
  B10001000,
  B01010000,
  B00100000,
  6,

  B10001000,  //W
  B10001000,
  B10001000,
  B10101000,
  B10101000,
  B10101000,
  B01010000,
  6,

  B10001000,  //X
  B10001000,
  B01010000,
  B00100000,
  B01010000,
  B10001000,
  B10001000,
  6,

  B10001000,  //Y
  B10001000,
  B10001000,
  B01010000,
  B00100000,
  B00100000,
  B00100000,
  6,

  B11111000,  //Z
  B00001000,
  B00010000,
  B00100000,
  B01000000,
  B10000000,
  B11111000,
  6,

  B11100000,  //[
  B10000000,
  B10000000,
  B10000000,
  B10000000,
  B10000000,
  B11100000,
  4,

  B00000000,  //(Backward Slash)
  B10000000,
  B01000000,
  B00100000,
  B00010000,
  B00001000,
  B00000000,
  6,

  B11100000,  //]
  B00100000,
  B00100000,
  B00100000,
  B00100000,
  B00100000,
  B11100000,
  4,

  B00100000,  //^
  B01010000,
  B10001000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  6,

  B00000000,  //_
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  B11111000,
  6,

  B10000000,  //`
  B01000000,
  B00100000,
  B00000000,
  B00000000,
  B00000000,
  B00000000,
  4,

  B00000000,  //a
  B00000000,
  B01110000,
  B00001000,
  B01111000,
  B10001000,
  B01111000,
  6,

  B10000000,  //b
  B10000000,
  B10110000,
  B11001000,
  B10001000,
  B10001000,
  B11110000,
  6,

  B00000000,  //c
  B00000000,
  B01110000,
  B10001000,
  B10000000,
  B10001000,
  B01110000,
  6,

  B00001000,  //d
  B00001000,
  B01101000,
  B10011000,
  B10001000,
  B10001000,
  B01111000,
  6,

  B00000000,  //e
  B00000000,
  B01110000,
  B10001000,
  B11111000,
  B10000000,
  B01110000,
  6,

  B00110000,  //f
  B01001000,
  B01000000,
  B11100000,
  B01000000,
  B01000000,
  B01000000,
  6,

  B00000000,  //g
  B01111000,
  B10001000,
  B10001000,
  B01111000,
  B00001000,
  B01110000,
  6,

  B10000000,  //h
  B10000000,
  B10110000,
  B11001000,
  B10001000,
  B10001000,
  B10001000,
  6,

  B01000000,  //i
  B00000000,
  B11000000,
  B01000000,
  B01000000,
  B01000000,
  B11100000,
  4,

  B00010000,  //j
  B00000000,
  B00110000,
  B00010000,
  B00010000,
  B10010000,
  B01100000,
  5,

  B10000000,  //k
  B10000000,
  B10010000,
  B10100000,
  B11000000,
  B10100000,
  B10010000,
  5,

  B11000000,  //l
  B01000000,
  B01000000,
  B01000000,
  B01000000,
  B01000000,
  B11100000,
  4,

  B00000000,  //m
  B00000000,
  B11010000,
  B10101000,
  B10101000,
  B10001000,
  B10001000,
  6,

  B00000000,  //n
  B00000000,
  B10110000,
  B11001000,
  B10001000,
  B10001000,
  B10001000,
  6,

  B00000000,  //o
  B00000000,
  B01110000,
  B10001000,
  B10001000,
  B10001000,
  B01110000,
  6,

  B00000000,  //p
  B00000000,
  B11110000,
  B10001000,
  B11110000,
  B10000000,
  B10000000,
  6,

  B00000000,  //q
  B00000000,
  B01101000,
  B10011000,
  B01111000,
  B00001000,
  B00001000,
  6,

  B00000000,  //r
  B00000000,
  B10110000,
  B11001000,
  B10000000,
  B10000000,
  B10000000,
  6,

  B00000000,  //s
  B00000000,
  B01110000,
  B10000000,
  B01110000,
  B00001000,
  B11110000,
  6,

  B01000000,  //t
  B01000000,
  B11100000,
  B01000000,
  B01000000,
  B01001000,
  B00110000,
  6,

  B00000000,  //u
  B00000000,
  B10001000,
  B10001000,
  B10001000,
  B10011000,
  B01101000,
  6,

  B00000000,  //v
  B00000000,
  B10001000,
  B10001000,
  B10001000,
  B01010000,
  B00100000,
  6,

  B00000000,  //w
  B00000000,
  B10001000,
  B10101000,
  B10101000,
  B10101000,
  B01010000,
  6,

  B00000000,  //x
  B00000000,
  B10001000,
  B01010000,
  B00100000,
  B01010000,
  B10001000,
  6,

  B00000000,  //y
  B00000000,
  B10001000,
  B10001000,
  B01111000,
  B00001000,
  B01110000,
  6,

  B00000000,  //z
  B00000000,
  B11111000,
  B00010000,
  B00100000,
  B01000000,
  B11111000,
  6,

  B00100000,  //{
  B01000000,
  B01000000,
  B10000000,
  B01000000,
  B01000000,
  B00100000,
  4,

  B10000000,  //|
  B10000000,
  B10000000,
  B10000000,
  B10000000,
  B10000000,
  B10000000,
  2,

  B10000000,  //}
  B01000000,
  B01000000,
  B00100000,
  B01000000,
  B01000000,
  B10000000,
  4,

  B00000000,  //~
  B00000000,
  B00000000,
  B01101000,
  B10010000,
  B00000000,
  B00000000,
  6,

  B01100000,  // (Char 0x7F)
  B10010000,
  B10010000,
  B01100000,
  B00000000,
  B00000000,
  B00000000,
  5,

  B00000000,  // smiley
  B01100000,
  B01100110,
  B00000000,
  B10000001,
  B01100110,
  B00011000,
  5
};
// Rotate the buffer
void rotateBufferLong() {
  for (int a = 0; a < 7; a++) {               // Loop 7 times for a 5x7 font
    unsigned long x = bufferLong [a * 2];   // Get low buffer entry
    byte b = bitRead(x, 31);                // Copy high order bit that gets lost in rotation
    x = x << 1;                             // Rotate left one bit
    bufferLong [a * 2] = x;                 // Store new low buffer
    x = bufferLong [a * 2 + 1];             // Get high buffer entry
    x = x << 1;                             // Rotate left one bit
    bitWrite(x, 0, b);                      // Store saved bit
    bufferLong [a * 2 + 1] = x;             // Store new high buffer
  }
}
// Display Buffer on LED matrix
void printBufferLong() {
  for (int a = 0; a < 7; a++) {             // Loop 7 times for a 5x7 font
    unsigned long x = bufferLong [a * 2 + 1]; // Get high buffer entry
    byte y = x;                             // Mask off first character
    lc.setRow(3, a, y);                     // Send row to relevent MAX7219 chip
    x = bufferLong [a * 2];                 // Get low buffer entry
    y = (x >> 24);                          // Mask off second character
    lc.setRow(2, a, y);                     // Send row to relevent MAX7219 chip
    y = (x >> 16);                          // Mask off third character
    lc.setRow(1, a, y);                     // Send row to relevent MAX7219 chip
    y = (x >> 8);                           // Mask off forth character
    lc.setRow(0, a, y);                     // Send row to relevent MAX7219 chip
  }
}
// Load character into scroll buffer
void loadBufferLong(int ascii) {
  if (ascii >= 0x20 && ascii <= 0x7f) {
    for (int a = 0; a < 7; a++) {               // Loop 7 times for a 5x7 font
      unsigned long c = pgm_read_byte_near(font5x7 + ((ascii - 0x20) * 8) + a);     // Index into character table to get row data
      unsigned long x = bufferLong [a * 2];   // Load current scroll buffer
      x = x | c;                              // OR the new character onto end of current
      bufferLong [a * 2] = x;                 // Store in buffer
    }
    byte count = pgm_read_byte_near(font5x7 + ((ascii - 0x20) * 8) + 7);    // Index into character table for kerning data
    for (byte x = 0; x < count; x++) {
      rotateBufferLong();
      printBufferLong();
      delay(scrollDelay);
    }
  }
}

void scrollFont() {
  for (int counter = 0x20; counter < 0x80; counter++) {
    loadBufferLong(counter);
    //delay(500);
  }
}

// Scroll Message
void scrollMessage(const char * messageString) {
  int counter = 0;
  int myChar = 0;
  do {
    // read back a char
    myChar =  pgm_read_byte_near(messageString + counter);
    if (myChar != 0) {
      loadBufferLong(myChar);
    }
    counter++;
  }
  while (myChar != 0);

}



void showPrice(String prefixString, String apiString, String keyString) {
  // GET HTTP request
  String price = httpRequest(apiString, keyString);

  // PEDAÇO NOVO.. SE PREÇO DER NULO, SE VIRA, E AGORA JOSÉ?
  if (price == 0 || price == nullptr || price == "null" ){
    //delay(7000);
    ESP.restart();
    return;
  }

  Serial.println("<<<<<<<<<<<<<<<========PRICE=========>>>>>>>>>>>>>>");
  Serial.println(price);
  Serial.println("<<<<<<<<<<<<<<<========================>>>>>>>>>>>>>>");
  
  // TREATMENT
  //char result[20]; // Buffer big enough for 7-character float
  //const char * newprice = dtostrf(price, 6, 2, result); // Leave room for too large numbers!
  if (price.indexOf(".") >= 0) {
    if (price.length() > 12) {
      price.remove(price.indexOf("."), price.length() - price.indexOf("."));
    }
    else if (price.length() == 12) {
      price.remove(price.indexOf(".") + 3, price.length() - price.indexOf(".") - 1);
    }
    else {
      price.remove(price.indexOf(".") + 3, price.length() - price.indexOf(".") - 2);
    }

  }
  // SCROLL MESSAGE
  scrollMessage(" ");
  scrollMessage(prefixString.c_str());
  scrollMessage(" ");
  scrollMessage("$");
  scrollMessage(" ");
  scrollMessage(price.c_str());
}






////////////// MAIN ////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////
void setup() {
  client.setInsecure();
  client.setTimeout(API_TIMEOUT);

  String fiat = "";
  float price = 0;


  //LEDS
  //FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
  //FastLED.setBrightness(BRIGHTNESS );


  //WEBSERVER
  if (!SPIFFS.begin()) {
    Serial.println("An Error has occurred while mounting SPIFFS");
  }
  // Send web page with input fields to client
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/style.css", "text/css");
  });
  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/script.js", String(), false, processor);
  });
  server.on("/wifi", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/wifi.html", String(), false, processor);
  });
  

  
  setupSPIFFS();
  
  //HTTP
  initSerial();
  initWiFi();
  StaticJsonDocument<200> doc;


  //MATRIX
  float intensity = readFile(SPIFFS, "/slideBrightness.txt").toFloat();
  for (int x = 0; x < numDevices; x++)
  {
    lc.shutdown(x, false);      //The MAX72XX is in power-saving mode on startup
    lc.setIntensity(x, intensity);      // Set the brightness to default value
    lc.clearDisplay(x);         // and clear the display
  }
  scrollDelay = 20;
  
  timeClient.begin();
  
  //scrollMessage("  Bem vindo ao CRIPTO RAPIT");
  delay(5000); //ERA 7000
}

//LOOP
void loop() {

  // RESTART ESP
  server.on("/reboot", HTTP_GET, [](AsyncWebServerRequest * request) {
    ESP.restart();
  });
  
  //DEBUG
  const char* wifiSSID = strdup((readFile(SPIFFS, "/wifiSSID.txt")).c_str());;
  //const char *wifiPass = strdup((readFile(SPIFFS, "/wifiPassword.txt")).c_str());
  Serial.println("======= ESTRANHO");
  Serial.println(wifiSSID);
  //Serial.println(wifiPass);
  //Serial.println("======= STRING");
  //Serial.println(readFile(SPIFFS, "/wifiSSID.txt"));
  //Serial.println(readFile(SPIFFS, "/wifiPassword.txt"));
  // DEBUG

  // IF WIFI NOT CONNECTED
  if(WiFi.status() != WL_CONNECTED){
    scrollMessage("  Wifi: RAPIT");
    scrollMessage(" - Senha: rapit.com.br");
    scrollMessage(" - Chrome: 192.168.4.1/wifi ");  
  }
  // IF IT IS
  else{
    String apiList = readFile(SPIFFS, "/coinList.txt");
    String currentCoins = readFile(SPIFFS, "/currentCoins.txt");
  
    int firstApi = currentCoins.indexOf(";");
    int lastApi = currentCoins.lastIndexOf(";");
    int nApi = lastApi / firstApi;
    
    statePower = readFile(SPIFFS, "/mainPower.txt").toInt();
    
    // BUTTON 0 (LIGA/DESLIGA)
    if (statePower == 1) { // DISPLAY LIGADO ->>>>>>>>>
  
      // LIGA DISPLAY
      intensity = readFile(SPIFFS, "/slideBrightness.txt").toFloat();
      for (int x = 0; x < numDevices; x++)
      {
        lc.setIntensity(x, intensity);
        lc.shutdown(x, false);
      }
  
      //CORE -->
      customText = readFile(SPIFFS, "/customText.txt").toInt();
      clockState = readFile(SPIFFS, "/clockState.txt").toInt();
  
      // ESTADO ZERO -> SEM MOEDA, SEM TEXTO E SEM RELOGIO
      if (currentCoins.length() <= 3 && customText == 0 && clockState == 0) {
        scrollMessage("  Nenhuma moeda selecionada. Escolha pelo APP: ");
        scrollMessage(WiFi.localIP().toString().c_str());
        Serial.println(WiFi.localIP());
        delay(2000);
      }
      // RELOGIO LIGADO
      else if(clockState == 1){
          timeClient.update();
          hours = timeClient.getHours();
          minutes = timeClient.getMinutes();
          if(hours.length() == 1){ hours = "0"+hours; }
          if(minutes.length() == 1){ minutes = "0"+minutes; }
          currentTime = hours + " : " + minutes;
          scrollMessage("   ");
          scrollMessage(currentTime.c_str());
          
          while(timeClient.getSeconds() != 0 && readFile(SPIFFS, "/clockState.txt").toInt() == 1 && readFile(SPIFFS, "/mainPower.txt").toInt() == 1 ){
            delay(1000);
          }
          
          Serial.print(daysOfTheWeek[timeClient.getDay()]);
          Serial.print(", ");
          Serial.print(timeClient.getHours());
          Serial.print(":");
          Serial.print(timeClient.getMinutes());
          Serial.print(":");
          Serial.println(timeClient.getSeconds());
          
      }
      else {     
  
        if (customText == 0) {
          //      prefixString = readFile(SPIFFS, "/inputPrefix1.txt");
          //      apiString = readFile(SPIFFS, "/inputApi1.txt");
          //      keyString = readFile(SPIFFS, "/inputKey1.txt");
          //      showPrice(prefixString, apiString, keyString);
  
  
          // VARIAS APIS AO MSM TEMPO
          String apiMaster = readFile(SPIFFS, "/apiMaster.txt");
          String currencyMaster = readFile(SPIFFS, "/currencyMaster.txt");
  
          int auxx = 0;
          Serial.println(nApi);
          for (int k = 0; k < currentCoins.length(); k++) {
            if (currentCoins[k] == ';') {
              Serial.println(apiList.substring(auxx, k));
              apiString = currentCoins.substring(auxx, k); // AQUI VAI PEGAR "BTC" "ETH" etc..
              keyString = readFile(SPIFFS, "/keyMaster.txt");
              //            prefixString = httpRequest(apiString+currencyMaster, "symbol");
              //            prefixString.replace(currencyMaster, "");
  
              fullApi = apiMaster + apiString + currencyMaster;
              //Serial.println("prefixString----->" + prefixString);
              Serial.println("apiMaster----->" + apiMaster);
              Serial.println("apiString----->" + apiString);
              Serial.println("currencyMaster----->" + currencyMaster);
              Serial.println("fullApi----->" + fullApi);
              showPrice(apiString, fullApi, keyString);
  
              auxx = k + 1;
              //delay(2000);
            }
          }
        }
        else {
          scrollMessage(" ");
          scrollMessage((readFile(SPIFFS, "/inputString.txt")).c_str());
        }
  
        scrollDelay = readFile(SPIFFS, "/slideSpeed.txt").toFloat();
        updateHtml();
      }
  
      // DISPLAY DESLIGADO -->>>>>>>>
    } else {
      for (int x = 0; x < numDevices; x++)
      {
        lc.setIntensity(x, 0);
        lc.shutdown(x, true);
      }
      delay(2000);
    }
  }





}
/////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
