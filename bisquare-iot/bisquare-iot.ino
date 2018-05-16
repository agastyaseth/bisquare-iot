//Operating Modes Selection

//#define WemoAlexa
// #define StreetLight_Moons
// #define RGBW_Light
// #define DHT
//#define PIR
//#define IRRemote
//#define IRBlast
//#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321

//Importing Libraries

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <aJSON.h>
#include <Arduino.h> 
#include "DHT.h"
#include "WemoSwitch.h"
#include "WemoManager.h"
#include "RestClient.h"

// #include "CallbackFunction.h"

#include <smartconfig.h> 
#include <osapi.h>
#include <ets_sys.h>
#include <EEPROM.h>

// Defining Modes

//IR Blaster
#ifdef IRBlast
  #include <IRremoteESP8266.h>
  #include <HVAC_IRLIB.h>
 // int IRpin=13;
  IRsend irsend(IRpin);
  #define DHTPIN 2
#endif

//IR Receiver
#ifdef IRRemote
  #include <IRremoteESP8266.h>
  int RECV_PIN = 5;
  IRrecv irrecv(RECV_PIN);
  decode_results results;
  int brightness = 255;
#endif

//For PIR Sensor Integration
#ifdef PIR
  const int PIR_PIN=16;
  int mctr =0;
  int M_status =0;
  #include <SimpleTimer.h>
  SimpleTimer mytimer;
 #endif

 //On/Off function prototypes for Wemo Emulation
#ifdef WemoAlexa
  void redOn();
  void redOff();
  void greenOn();
  void greenOff();
  void blueOn();
  void coolOn();
  void warmOn();
  void blueOff();
  void turnOnAll();
  void turnOffAll() ;
  char deviceName[100]="Light";
#endif

//Streetlight Mode
#ifdef StreetLight_Moons
  const int ledG_pin = 4;
#else
  const int ledG_pin = 12;
#endif
  const int ledR_pin = 14;
  const int ledB_pin = 15;
  const int config_pin = 0;

  //const int DHTPin = 

//WeMo Emulation
#ifdef WemoAlexa
  WemoManager wemoManager;
  WemoSwitch *red = NULL;
  WemoSwitch *green = NULL;
  WemoSwitch *blue = NULL;
  WemoSwitch *all = NULL;
  WemoSwitch *warm = NULL;
  WemoSwitch *cool = NULL;
#endif

WiFiUDP Udp;   //WiFI UDP Object for Smart Connect

int last_R=0, last_G=0, last_B=0;
char MAC_char[20], MAC_char_short[15];
int configButtonState = 1;


// Default IP Address for AP Interface
IPAddress ip(192, 168, 5, 1);
IPAddress subnet(255, 255, 255, 0);

ESP8266WebServer server(80);
String CNXS_CLOUD = "http://34.192.252.146";
//RestClient client = RestClient("http://34.192.252.146:3000");


void configureSoftAP() {
    char apName[20];
    sprintf(apName, "%s%s","cnxs",MAC_char_short);
    Serial.begin(115200);
    Serial.print("FN: configureSoftAP: ");
    Serial.println(apName);
  
    WiFi.softAPConfig(ip, ip, subnet);
    WiFi.softAP(apName, "somethingfishy42#");

}


// General Function to convert String to integer
int stringToInt(String s)
{
  char arr[12];
  s.toCharArray(arr, sizeof(arr));
  return atoi(arr);
}

//MAC Address string generation
void setOwnMac() {
  Serial.begin(115200);
  Serial.print("FN: setOwnMac");
  
  uint8_t MAC_array[6];
  WiFi.macAddress(MAC_array);
  sprintf(MAC_char, "%02x%02x%02x%02x%02x%02x",MAC_array[0],
                                               MAC_array[1],
                                               MAC_array[2],
                                               MAC_array[3],
                                               MAC_array[4],
                                               MAC_array[5]);

  sprintf(MAC_char_short, "%02x%02x%02x",MAC_array[3],
                                               MAC_array[4],
                                               MAC_array[5]);
  
                                               
}



void handleGetIdentity() {
  char A_SSID[20];
  char IP_ADD[26];
  Serial.begin(115200);
  Serial.print("FN: handleGetIdentity");

  IPAddress tempip;
  
  Serial.println("Returning MAC and IP");
  Serial.println(MAC_char);
  tempip = WiFi.localIP();
  unsigned int tt = (unsigned int) WiFi.SSID().length()+1;
  WiFi.SSID().toCharArray(A_SSID, tt);
  sprintf(IP_ADD, "%d.%d.%d.%d",tempip[0],tempip[1],tempip[2],tempip[3]);

  Serial.println(A_SSID);
  Serial.println(IP_ADD);
   
  // BUILD JSON Object to return {"sensor":"gps","time":1351824120,"data":[48.756080,2.302038]}
  // Step 1: Reserve memory space
  //
  StaticJsonBuffer<200> jsonBuffer;

  //
  // Step 2: Build object tree in memory
  //
  JsonObject& root = jsonBuffer.createObject();
  root["mac"] = MAC_char;
  root["A_SSID"] = A_SSID;
  root["IP_ADD"] = IP_ADD;

  //
  // Step 3: Generate the JSON string
  //
  //root.printTo(Serial);
  char buffer[256];
  root.printTo(buffer, sizeof(buffer));
  server.send(200, "application/json", buffer);

}

void handleRoot() {
  Serial.begin(115200);
  Serial.print("FN: handleRoot");
  
  //digitalWrite(ledR_pin, HIGH);
  //delay(1000);
  server.send(200, "text/plain", "Hello from ESP8266!");
    
  //digitalWrite(led, 0);
  
}
#ifdef WemoAlexa
void loadDeviceName() {
  EEPROM.begin(256);
  EEPROM.get(0, deviceName);
  //EEPROM.get(0+sizeof(ssid), password);
  char ok[2+1];
  EEPROM.get(0+sizeof(deviceName), ok);
  EEPROM.end();
  if (String(ok) != String("OK")) {
    strcpy(deviceName,"Light");
  }
  Serial.println("Recovered credentials:");
  Serial.println(deviceName);
  //Serial.println(strlen(password)>0?"********":"<no password>");
}

/** Store WLAN credentials to EEPROM */
void saveDeviceName() {
  EEPROM.begin(256);
  EEPROM.put(0, deviceName);
  //EEPROM.put(0+sizeof(ssid), password);
  char ok[2+1] = "OK";
  EEPROM.put(0+sizeof(deviceName), ok);
  EEPROM.commit();
  EEPROM.end();
}

void handleSetDeviceName() {
  String ttt= server.arg("name");
  ttt.toCharArray(deviceName,100);
  saveDeviceName();
  
}

#endif

void handleGetStatus() {
  Serial.begin(115200);
  Serial.println("FN: handleGetStatus");
  
  char buf[25];

  sprintf(buf, "%d%s%d%s%d", last_R," ",last_G, " ", last_B);
  
  Serial.println(buf);
   
  // BUILD JSON Object to return {"sensor":"gps","time":1351824120,"data":[48.756080,2.302038]}
  // Step 1: Reserve memory space
  //
  StaticJsonBuffer<200> jsonBuffer;

  //
  // Step 2: Build object tree in memory
  //
  JsonObject& root = jsonBuffer.createObject();
  root["led1"] = last_R;
  root["led2"] = last_G;
  root["led3"] = last_B;

  //JsonArray& data = root.createNestedArray("data");
  //data.add(48.756080, 6);  // 6 is the number of decimals to print
  //data.add(2.302038, 6);   // if not specified, 2 digits are printed

  //
  // Step 3: Generate the JSON string
  //
  //root.printTo(Serial);
  char buffer[256];
  root.printTo(buffer, sizeof(buffer));
  server.send(200, "application/json", buffer);

}

/* void motionDetect(){
    Serial.begin(115200);
    if(digitalRead(PIR_PIN)==HIGH && !M_status)
    {
      Serial.println("Motion Detected!");
      setLED(255,255,255);
      M_status=1;
    }

    else if(digitalRead(PIR_PIN)==HIGH && M_status)
      M_status++;
} */

void setLED(int R, int G, int B)
{
  analogWrite(ledR_pin, R);
  analogWrite(ledG_pin, G);
  analogWrite(ledB_pin, B);
  last_R=R;
  last_G=G;
  last_B=B;
}

void handleSetLeds() {
  Serial.begin(115200);
  Serial.print("FN: handleSetLEDs");
  // We can read the desired status of the LEDs from the expected parameters that
  // should be passed in the URL.  We expect two parameters "led1" and "led2".
  String ledR_buffer = server.arg("led1");
  String ledG_buffer = server.arg("led2");
  String ledB_buffer = server.arg("led3");

  int ledR_status = stringToInt(ledR_buffer);
  int ledG_status = stringToInt(ledG_buffer);
  int ledB_status = stringToInt(ledB_buffer);

  //http://192.168.1.169/setleds?led1=(0-255)&led2=(0-255)&led3=(0-255)

#ifdef StreetLight_Moons
  setLED(ledR_pin, ledR_status*4,ledG_status*4,ledB_status*4);
#else
  setLED(ledR_status, ledG_status, ledB_status);
#endif

  
  //changecolor(ledR_status, ledG_status, ledB_status);
  last_R = ledR_status;
  last_G = ledG_status;
  last_B = ledB_status;
  
  server.send(200, "text/plain", "LEDs' status changed!"); 

}



void handleNotFound() {
  Serial.begin(115200);
  Serial.print("FN: handleNotFound");
  //digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  // digitalWrite(led, 0);
}

//String response;

void registerOnCNXScloud(){
  Serial.println("Inside registeronCNXSCloud");
  //WiFiClient client;
  //const char* host="http://34.192.252.146";
  // bool HTTPClient::begin(CNXS_CLOUD,3000,"/api/device/v1/deviceIdentity/");
  HTTPClient http;
  //http.begin(CNXS_CLOUD,3000,"/api/device/v1/deviceIdentity/");
  if (!http.begin("http://34.192.252.146:3000/api/device/v1/deviceIdentity/ "))
   Serial.println("Failed to connect to http"); //Specify request destination 
  //http.begin(CNXS_CLOUD);

  //http.addHeader("Content-Type", "application/json;charset=utf-8");
  http.addHeader("Content-Type", "application/json");
 // http.addHeader("cache-control", "no-cache");    

  StaticJsonBuffer<300> JBuf;
  JsonObject& root = JBuf.createObject(); 
 // char* dev_id = strcat("cnxs", MAC_char_short);
  root["macAddress"] = MAC_char;
  char A_SSID[20];
  char IP_ADD[26];
  IPAddress tempip;
  tempip = WiFi.localIP();
  unsigned int tt = (unsigned int) WiFi.SSID().length()+1;
  WiFi.SSID().toCharArray(A_SSID, tt);
  sprintf(IP_ADD, "%d.%d.%d.%d",tempip[0],tempip[1],tempip[2],tempip[3]);

  root["ipAddress"] = IP_ADD;
  root["ssId"] = A_SSID;
  JsonObject &conf = root.createNestedObject("config");
  
  conf["moodLight"]=true;
  
  #ifdef StreetLight_Moons
  conf["sheetLight"]=true;
  #else 
  conf["sheetLight"]=false;
  #endif
  
  #ifdef DHT
  conf["dth"]=true;
  #else
  conf["dth"]=false;
  #endif

  #ifdef PIR
  conf["pr"]=true;
  #else
  conf["pr"]=false;
  #endif

  #ifdef IRBlast
  conf["irBlast"]=true;
  #else
  conf["irBlast"]=false;
  #endif

  #ifdef Switch
  conf["switch"]=true;
  #else
  conf["switch"]=false;
  #endif

  #ifdef IRRemote
  conf["irReecv"]=true;
  #else
   conf["irReecv"]=false;
  #endif

  #ifdef loadComp
  conf["loadComp"]=true;
  #else
  conf["loadComp"]=false;
  #endif

  root["isActive"]=true;


  char* payload = "{\"macAddress\":\"a10031f1ab\",\"ipAddress\":\"192.168.1.120\",\"ssId\":\"hello\",\"config\":{\"moodLight\":true,\"sheetLight\":false,\"dth\":false,\"pr\":false,\"irBlast\":false,\"switch\":false,\"irReecv\":false,\"loadComp\":false},\"isActive\":true}";
  //root.printTo(payload, sizeof(payload));
  
  Serial.print("Sending JSON: ");
  Serial.println(payload);
  
  //int httpCode = http.POST((uint8_t *)payload,strlen(payload));
  int httpCode = http.POST(payload);
  String return_payload = http.getString(); //Get the response payload
  
  if(httpCode == 201)
  {
      Serial.print("HTTP response code ");
      Serial.println(httpCode);
      Serial.print(return_payload);
  }
  else
  {
    Serial.print("Error Registering Device: ");
    Serial.println(http.errorToString(httpCode).c_str());
  }
  
  http.end();
  


/* if (client.connect(host, 3000)) {

client.println("POST /posts HTTP/1.1");
client.println("Host: http://34.192.252.146");
client.println("Cache-Control: no-cache");
client.println("Content-Type: application/json");
client.print("Content-Length: ");
client.println(payload.length());
client.println();
client.println(payload);

while(!client.available()){
  Serial.print(">");
}

while (client.connected())
{
  if ( client.available() )
  {
    char str=client.read();
   Serial.println(str);
  }      
}
}  */
  Serial.println();
   
}

void invoke_smart_con() {
  int cnt = 0;
  WiFi.disconnect();
  WiFi.begin();
  WiFi.mode(WIFI_STA);
  delay(500);
   
  analogWrite(ledG_pin, 0);
  analogWrite(ledB_pin, 0);
    
  digitalWrite(ledR_pin, HIGH);

  Serial.println("Initiating SmartConfig...");
  WiFi.beginSmartConfig();
  
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("#");
    if(WiFi.smartConfigDone()){
       Serial.println("SmartConfig Success");
       //break;
     }
    (ledR_pin, LOW);
    delay(250);
  } /* While WiFi not connected */
  
  WiFi.mode(WIFI_AP_STA);
  
  delay(500);
  
  digitalWrite(ledR_pin, LOW);
  
  if (WiFi.status() == WL_CONNECTED)  {
      Serial.println("WL Connected, registering on CNXS CLoud");
      registerOnCNXScloud();
  }

}

void handleInitSC() {
   Serial.println("handleInitSC...");
   server.send(200, "text/plain", "Device going into auto-config mode, please use app to configure AP & password");
   delay(10);
   invoke_smart_con();
}

/*String IpAddress2String(const IPAddress& ipAddress)
{
  return String(ipAddress[0]) + String(".") +\
  String(ipAddress[1]) + String(".") +\
  String(ipAddress[2]) + String(".") +\
  String(ipAddress[3])  ; 
} */
 
#ifdef IRBlast

HvacMode_t strtomode(String str){
  if(str=="hot")
    return HVAC_HOT;
  else if(str=="cold")
    return HVAC_COLD;
  else if(str=="dry")
    return HVAC_DRY;
  else if(str=="fan")
    return HVAC_FAN;
  else if(str=="auto")
    return HVAC_AUTO;
}

HvacFanMode_ strtofan(String str){
  if (str=="1")
    return FAN_SPEED_1;
  else if  (str=="2")
    return FAN_SPEED_2;
  else if (str=="3")
    return FAN_SPEED_3;
  else if (str=="4")
    return FAN_SPEED_4;
  else if (str=="5")
    return FAN_SPEED_5;
   else if (str=="auto")
    return FAN_SPEED_AUTO;
   else if (str=="silent")
    return FAN_SPEED_SILENT;      
}

HvacVanneMode_ strtovanne(String str){
  if (str=="1")
    return VANNE_H1;
  else if  (str=="2")
    return VANNE_H2;
  else if (str=="3")
    return VANNE_H3;
  else if (str=="4")
    return VANNE_H4;
  else if (str=="5")
    return VANNE_H5;
   else if (str=="auto");
    return VANNE_AUTO_MOVE;
      
}

HvacWideVanneMode_t strtowvanne(String str){
  if (str=="WLE")
    return WIDE_LEFT_END;
  else if  (str=="WL")
    return WIDE_LEFT;
  else if (str=="WM")
    return WIDE_LEFT;
  else if (str=="WR")
    return WIDE_RIGHT;
  else if (str=="WRE")
    return WIDE_RIGHT_END;     
}

HvacAreaMode_t strtoarea(String str){
  if (str=="swing")
    return AREA_SWING;
  else if  (str=="left")
    return AREA_LEFT;
  else if (str=="auto")
    return AREA_AUTO;
  else if (str=="right")
    return AREA_RIGHT;     
}

HvacProfileMode_t strtoprofile(String str){
  if (str=="normal")
    return NORMAL;
  else if  (str=="quiet")
    return QUIET;
  else if (str=="boost")
    return BOOST; 
}

void handleSendIR(){

  bool state = server.arg("state");
  HvacMode_t Mode= strtomode(server.arg("mode"));
  HvacFanMode_ fan = strtofan(server.arg("fan"));
  HvacVanneMode_ vanne = strtovanne(server.arg("vanne"));
  HvacWideVanneMode_t widevanne = strtowvanne(server.arg("widevanne"));
  HvacAreaMode_t area = strtoarea(server.arg("area"));
  HvacProfileMode_t profile = strtoprofile(server.arg("profile"));
  int temp = stringToInt(server.arg("temp"));
  String actype = server.arg("type");
  StaticJsonBuffer<200> JBuf;
  JsonObject& root = JBuf.createObject();

  // READING FROM DHT
  delay(2000);

  DHT dht (DHTTYPE, DHTPIN);

  // Reading temperature or humidity takes about 250 milliseconds
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print(f);
  Serial.print(" *F\t");
  Serial.print("Heat index: ");
  Serial.print(hic);
  Serial.print(" *C ");
  Serial.print(hif);
  Serial.println(" *F");

 // END OF READING FROM DHT
 
  if (actype=="panasonic")
  {
      sendHvacPanasonic(Mode, temp, fan, vanne, profile, state);
     // readDHT();                                                                                                                                                                                                              
      root["mode"]=server.arg("mode");
      root["temp"]=temp;
      root["fan"]=server.arg("fan");
      root["vanne"]=server.arg("vanne");
      root["profile"]=server.arg("profile");
      root["type"]="panasonic";
      root["state"]=server.arg("state");

  }

  else if (actype=="toshiba")
  {
      sendHvacToshiba(Mode, temp, fan, state);
      root["mode"]=server.arg("mode");
      root["temp"]=temp;
      root["fan"]=server.arg(fan);
      root["state"]=server.arg("state");
  }
  
 else if (actype=="mitsubishi")
  {
      sendHvacMitsubishi(Mode, temp, fan, vanne, state);
      root["mode"]=server.arg("mode");
      root["temp"]=temp;
      root["fan"]=server.arg(fan);
      root["vanne"]=server.arg(vanne);
      root["state"]=server.arg("state");
  }
  root["dht_temp_celc"]=t;
  root["dht_temp_far"]=f;
  root["dht_hum"]=h;
  root["dht_hic"]=hic;
  root["dht_hif"]=hif;
  
  HTTPClient http;
  http.begin(CNXS_CLOUD);
  char payload[256];
  root.printTo(payload, sizeof(payload));
  
  int httpCode = http.POST((uint8_t *)payload,strlen(payload));
  if(httpCode == HTTP_CODE_OK)
  {
      Serial.print("HTTP response code ");
      Serial.println(httpCode);
      String response = http.getString();
  }
  else
  {
    Serial.println("Error in Posting AC status");
  }
  
  http.end();

  Serial.println();
  
}

#endif

void setup() {
  
  pinMode(ledR_pin, OUTPUT);
  pinMode(ledG_pin, OUTPUT);
  pinMode(ledB_pin, OUTPUT);

  pinMode(config_pin, INPUT_PULLUP);

  #ifdef PIR
    pinMode(PIR_PIN, INPUT);
  #endif

  #ifdef IRBlast
    irsend.begin();
  #endif
  //attachInterrupt(config_pin, handleInitSC, CHANGE);

  Serial.begin(115200);
  Serial.println("");
  
  WiFi.printDiag(Serial);
  
  setOwnMac();
  configureSoftAP();
  
#ifdef WemoAlexa
  loadDeviceName();
#endif
  
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin();

  // Start the server
  Udp.begin(49999);
  Serial.println("UDP Server started");

 #ifdef IRRemote
    irrecv.enableIRIn(); // Start the receiver
 #endif

  
  if(WiFi.status() != WL_CONNECTED) {
      int cnt=0;
      Serial.println("Checking if AP connection is possible");
      while (cnt++ <=8 ) {
        Serial.print("#");
        delay(1000);
      }   
  }
  
  Serial.println("");
  if (!WiFi.localIP())
     Serial.println("Could not connect to any AP, looking to be connected to as SoftAP itself");
  else {
     Serial.print("Connected to AP: ");
     Serial.println(WiFi.SSID()); 
     Serial.println("");
     //Register on cloud...
     registerOnCNXScloud();
  }

#ifdef WemoAlexa
     String dev_r = "Red ";
     dev_r.concat(deviceName);
     String dev_g = "Green ";
     dev_g.concat(deviceName);
     String dev_b = "Blue ";
     dev_b.concat(deviceName);
     String dev_cool = "Cool ";
     dev_cool.concat(deviceName);
     String dev_wrm = "Warm ";
     dev_wrm.concat(deviceName);
     wemoManager.begin();
     // Format: Alexa invocation name, local port no, on callback, off callback
     red = new WemoSwitch(dev_r, 8000, redOn, redOff);
     green = new WemoSwitch(dev_g, 8001, greenOn, greenOff);
     blue = new WemoSwitch(dev_b, 8002, blueOn, blueOff);
     warm = new WemoSwitch(dev_wrm, 8004, warmOn, turnOffAll);
     cool = new WemoSwitch(dev_cool, 8005, coolOn, turnOffAll);
     all = new WemoSwitch(deviceName, 8003, turnOnAll, turnOffAll);
     
     wemoManager.addDevice(*red);
     wemoManager.addDevice(*green);
     wemoManager.addDevice(*blue);
     wemoManager.addDevice(*cool);
     wemoManager.addDevice(*warm);
     wemoManager.addDevice(*all);
#endif
     
 // if (MDNS.begin("esp8266")) {
 //  Serial.println("MDNS responder started");
 //}

  server.on("/", handleRoot);

  server.on("/reset", []() {
    server.send(200, "text/plain", "WiFi Configuration is being reset... please reconfigure from app");
    WiFi.disconnect();
    //invoke_smart_con();
  });

  server.onNotFound(handleNotFound);
  server.on("/setleds", HTTP_GET, handleSetLeds);

#ifdef WemoAlexa
  server.on("/setDeviceName", HTTP_GET, handleSetDeviceName);
#endif

  server.on("/getIdentity", HTTP_GET, handleGetIdentity);

  server.on("/getStatus", HTTP_GET, handleGetStatus);

  server.on("/initSmartConfig", HTTP_GET, handleInitSC);
  
#ifdef IRBlast
  server.on("/sendIR", HTTP_GET, handleSendIR);
#endif
 
  server.begin();
  Serial.println("HTTP server started");  
  
}

//int value = 0;

void loop(void) {
  server.handleClient();

#ifdef WemoAlexa
  wemoManager.serverLoop();
#endif
  
  configButtonState = digitalRead(config_pin);
  if (configButtonState < 1) {
    // Invvoke Smart Config
    Serial.println("LOOP: Invoking smart_con");
    Serial.println(configButtonState);
    invoke_smart_con();
  }
  yield();

 #ifdef PIR
    /* Detect Motion */
    if (M_status == 0) {
         if(digitalRead(PIR_PIN)==HIGH) {
             Serial.println("Motion Detected!");
             setLED(255,255,255);
             M_status=1;
          }
         else { /* PIR_PIN Low*/
          
       }
       else { /* Light is ON */

          
       }
    }
 #endif

//Receiving loop for IR Remote

 #ifdef IRRemote
  if (irrecv.decode(&results)) {
    Serial.println(results.value, HEX);
    IR_map(results.value);
    irrecv.resume(); // Receive the next value
  }
 #endif
  

 //t.update();

 /*
 Udp.parsePacket();
 configButtonState = digitalRead(config_pin);

 // check if the pushbutton is pressed.
 // if it is, the buttonState is HIGH:
 if (configButtonState == LOW) {
    // Invvoke Smart Config
    handleInitSC();
 }
 while(Udp.available()){
   Serial.println(Udp.remoteIP());
   Udp.flush();
   delay(5);
 }
 */
} /* loop()*/

//Standard NEC Encoded remote Support
#ifdef IRRemote
void IR_map(unsigned int code)
{
    if(code==0xF7C03F)  {   //on
      setLED(255,255,255);
      }
    else if(code==0xF740BF){   //off
      setLED(0,0,0);
      }
    else if(code==0xF720DF){  //red 
      setLED(255,0,0);
      }
    else if(code==0xF7A05F){   //green
      setLED(0,255,0);
    }

    else if(code==0xF7609F){   //blue
      setLED(0,0,255);
    }
    else if(code==0xF7E01F){   //white
      setLED(255,255,255);
    }
    else if(code==0xF700FF){   //B_UP
      brightness += 10;
      if (brightness > 255) brightness = 255;
      analogWrite(ledR_pin, last_R * brightness / 255);   
      analogWrite(ledG_pin, last_G * brightness / 255);      
      analogWrite(ledB_pin, last_B * brightness / 255);
    }

    else if(code==0xF7807F){   //B_DOWN
      brightness -= 10;
      if (brightness < 0) brightness = 0;
      analogWrite(ledR_pin, last_R * brightness / 255);   
      analogWrite(ledG_pin, last_G * brightness / 255);      
      analogWrite(ledB_pin, last_B * brightness / 255);
    }
}
#endif

#ifdef WemoAlexa
void redOn() {
    Serial.print("Turning on RED light ...");
    //if (last_R == 0)
    last_R = 150;
    last_G = 0;
    last_B = 0;
    analogWrite(ledR_pin, last_R);
    analogWrite(ledG_pin, last_G);
    analogWrite(ledB_pin, last_B);
  
}

void redOff() {
    Serial.print("Turning off RED light");
    analogWrite(ledR_pin, 0);
}

void greenOn() {
    Serial.print("Turning on GREEN light ...");
    //if (last_G == 0)
    last_G = 180;
    last_R = 0;
    last_B = 0;
    setLED(last_R,last_G,last_B);
}

void greenOff() {
    Serial.print("Turning off GREEN light");
    analogWrite(ledG_pin, 0);
}

void blueOn() {
    Serial.print("Turning on BLUE light ...");
  //  if (last_B == 0)
    last_R=0;
    last_G=0;
    last_B = 200;
    setLED(last_R,last_G,last_B);
}

void blueOff() {
    Serial.print("Turning off BLUE light");
    analogWrite(ledB_pin, 0);
}

void coolOn() {
    Serial.print("Turning on BLUE light ...");
  //  if (last_B == 0)
    last_R=56;
    last_G=0;
    last_B = 255;
    setLED(last_R,last_G,last_B);


}

void warmOn() {
    Serial.print("Turning on BLUE light ...");
  //  if (last_B == 0)
    last_R=255;
    last_G=135;
    last_B = 0;
    setLED(last_R,last_G,last_B);

}

void turnOnAll() {
//if ((last_R == 0) && (last_G ==0) && (last_B == 0)) {
  /*Device was off, default ON to R=150, G=150, B=150 */
  last_R=145;
  last_G=141;
  last_B=170;
//}
 
   setLED(last_R,last_G,last_B);
}

void turnOffAll() {
 //digitalWrite(ledB_pin, LOW);  // turn off relay with voltage LOW
 
    setLED(0,0,0);
}
#endif

