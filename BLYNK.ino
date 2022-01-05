//các thư viện
#define BLYNK_PRINT Serial
#include <DHT.h>
#include <BlynkSimpleEsp32.h>
#ifdef ESP32
#include <WiFi.h>
#include <WiFiMulti.h>
//#include <AsyncTCP.h>
#include <HTTPClient.h>
#include <HTTPUpdate.h>

#else
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESPAsyncTCP.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <BlynkSimpleEsp8266.h>

#endif
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <ESPmDNS.h>

//define các input và output
#define relayBom 33
#define relayDen 25
#define relayQuat 26
#define relayBonphan 27
#define Dongmai 18
#define Momai 19
#define DHTPIN 14
#define DHTTYPE DHT11
#define SOIL_MOIST_1_PIN 32
#define CTHT1 39
#define CTHT2 34
#define cambienmua 36
DHT dht(DHTPIN, DHTTYPE);

//gán kiểu cho các biến
float moist;
float humi;
float temp;
unsigned long time1 = 0;
//boolean flag = false;
//int pinValue;
//float pinValue1;
//int pinValue2;
float null;
float MOIST = 2;
float TEMPERATURE = 22.5;
float HUMIDITY = 75;
bool processing = 0;

//github
const char *fwImageURL = "https://raw.githubusercontent.com/LongLee9x/httpupdate/main/firmware.bin";

//thông số blynk và wifi
WiFiMulti WiFiMulti;
const char* auth = "DF5Uea1eZBhkbpmfMW7Spp7PmVOKa5d9"; // nhap ma Token ket noi Blynk
char ssid[]= "";
char pass[]=  "";

//các widget trong blynk
WidgetLED led_bom(V3);
WidgetLED led_den(V2);
WidgetLED led_quat(V5);
WidgetLED led_bonphan(V6);
WidgetTerminal terminal(V0);

//khai báo thư viện LCD
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);

//kết nối để gửi dữ liệu lên thingspeak
WiFiClient client1;
const int channelID = 1601597;
String writeAPIKey = "RHHX6XDLL2IBW77Y";
const char* server = "api.thingspeak.com";
const int postingInterval = 2 * 1000; // sau 2 giây gửi dữ liệu 1 lần

//#include "RTC_setting.h"
//#include "ota_firmware.h"
//#include "Blynkprocess.h"

void setup()
{
  Serial.begin(115200);
  dht.begin();
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  WiFiMulti.addAP(ssid, pass);
  WiFiManager wifiManager;
//  wifiManager.EC_begin();
//  wifiManager.EC_read();
  wifiManager.autoConnect("ESP server", "12345678");
  Serial.print("Success");
//  const char* auth = wifiManager.getAuth();
  Blynk.begin(auth, ssid, pass);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(CTHT1, INPUT);
  pinMode(CTHT2, INPUT);
  pinMode(relayBom, OUTPUT);
  pinMode(relayDen, OUTPUT);
  pinMode(relayQuat, OUTPUT);
  pinMode(relayBonphan, OUTPUT);
  pinMode(Dongmai, OUTPUT);
  pinMode(Momai, OUTPUT);
  digitalWrite(relayBom, HIGH);
  digitalWrite(relayDen, HIGH);
  digitalWrite(relayQuat, HIGH);
  digitalWrite(relayBonphan, HIGH);
  digitalWrite(Dongmai, LOW);
  digitalWrite(Momai, LOW);

  //hiển thị lên lcd
  lcd.begin(16, 2);
  lcd.init();
  lcd.backlight();
  lcd.print("Xin chao");
  lcd.setCursor(0, 1);
  lcd.print("Vuon rau tu dong");
  delay(5000);
  lcd.clear();
  lcd.print("IP Address");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());
  Serial.println(WiFi.localIP());
  
  // Xoa man hinh terminal va hien thi mot so thong tin ket noi ban dau
  terminal.clear();
  terminal.println(F("Đồ Án Tốt Nghiệp"));
  terminal.println(F("Hệ Thống Chăm Sóc Vườn Rau"));
  terminal.println(F("--------------------------"));
  terminal.println(F("Nhập lệnh help để trợ giúp"));
  terminal.flush();
}

//terminal
BLYNK_WRITE(V0)
{
  if (String("restart") == param.asStr())
  {
    terminal.print("Restarting");
    for(byte i = 1; i <= 3; i++)
    {
      delay(1000);
      terminal.print('.');
    }
    terminal.flush(); // Đảm bảo mọi thứ được gửi đi
    ESP.restart();
  }
   else if (String("clear") == param.asStr())
  {
    terminal.clear();
  }
  else if (String("weather") == param.asStr())
  {
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    float moist = getMoist();
    terminal.print(F("Nhiệt độ: "));
    terminal.print(t);
    terminal.println(F("°C "));
    terminal.print(F("Độ ẩm: "));
    terminal.print(h);
    terminal.println(F("% "));
    terminal.print(F("Độ ẩm đất: "));
    terminal.print(moist);
    terminal.println(F("% "));
    terminal.flush();
  }
  else if (String("pumpon") == param.asStr())
  {
    digitalWrite(relayBonphan, LOW);
    terminal.println("Bơm bón phân được bật");
  }
  else if (String("pumpoff") == param.asStr())
  {
    digitalWrite(relayBonphan, HIGH);
    terminal.println("Bơm bón phân được tắt");
  }
  else if (String("help") == param.asStr())
  {
    terminal.println("Các lựa chọn:");
 //   terminal.println("updateOTA");
    terminal.println("updateHTTP");
    terminal.println("restart");
    terminal.println("clear");
    terminal.println("weather");
    terminal.println("pumpon");
    terminal.println("pumpoff");
  }
//  else if (String("updateOTA") == param.asStr()) 
//  {
//    terminal.println("Đang cập nhật...");
//    while (String("updateOTA") == param.asStr())
//    {
//    OTAupdate();
//    delay(1000);
//  }
//  }
  else if (String("updateHTTP") == param.asStr())
  {
    while (String("updateHTTP") == param.asStr())
    {
    terminal.println("Đang cập nhật...");  
    InstallUpdates();
    delay(1000);
  }
  }
  else{
    terminal.println("Sai cú pháp");
  }
  terminal.flush();
}

//thông số độ ẩm đất
BLYNK_WRITE(V1)                     //  ham nay duoc goi den khi Widget Vo thay doi trang thai
{
  float pinValue1 = param.asFloat();
  if (pinValue1 != null) {
    MOIST = pinValue1;
  }
  else {
    MOIST = 2;
  }
}

//thông số độ ẩm
BLYNK_WRITE(V4)                     //  ham nay duoc goi den khi Widget Vo thay doi trang thai
{
  float pinValue4 = param.asFloat();
  if (pinValue4 != null) {
    HUMIDITY = pinValue4;
  }
  else {
    HUMIDITY = 75;
  }
}

//thông số nhiệt độ
BLYNK_WRITE(V7)                     //  ham nay duoc goi den khi Widget Vo thay doi trang thai
{
  float pinValue7 = param.asFloat();
  if (pinValue7 != null) {
    TEMPERATURE = pinValue7;
  }
  else {
    TEMPERATURE = 22.5;
  }
}
////    if ( (unsigned long) (millis() - time1) > 10000)
////    {
////
////        // Thay đổi trạng thái đèn led
////        if (digitalRead(pinValue) == 1)
////        {
////            digitalWrite(relayBom, LOW);
////        } else {
////            digitalWrite(relayBom, LOW);
////        }
////
////        // cập nhật lại biến time
////        time1 = millis();
////    }

void loop()
{

  Blynk.run();
  ledbom();
  ledden();
  ledquat();
  ledbonphan();
  autoBom();
  autoDen();
  autoQuat();
  autoMai();
  thingConnect();
  Serial.print("Giá trị MOIST: ");
  Serial.println(MOIST);
  Serial.print("Giá trị TEMPERATURE: ");
  Serial.println(TEMPERATURE);
}

void ledbonphan(){
  if (digitalRead(relayBonphan)==LOW){
    led_bonphan.on(); 
  }
  else{
    led_bonphan.off();
  }
}
void ledquat(){
  if (digitalRead(relayQuat)==LOW){
    led_quat.on();
  }
  else{
    led_quat.off();
  }
}
void ledden() {
  if (digitalRead(relayDen) == LOW) {
    led_den.on();
  }
  else {
    led_den.off();
  }
}
void ledbom() {
  if (digitalRead(relayBom) == LOW) {
    led_bom.on();
  }
  else {
    led_bom.off();
  }
}

//để đóng hoặc mở mái che cần xét đến các yếu tố như sau:
//nếu nhiệt độ quá thấp hoặc nhiệt độ quá cao hoặc mưa quá lớn sẽ đóng mái che
//khi chạm công tắc hành trình mái che sẽ dừng
void autoMai(){
float temperature;
temperature = dht.readTemperature();
float cambienmuaStatus = analogRead(cambienmua);
//Serial.print("Giá trị mưa: ");
//Serial.println(cambienmuaStatus);
if (temperature < 15 || temperature > 27 || cambienmuaStatus > 30){
  digitalWrite(Dongmai, HIGH);
  digitalWrite(Momai, LOW);
  int buttonStatus = digitalRead(CTHT1);
      Serial.println(buttonStatus);
  if (buttonStatus == 0){
    digitalWrite(Dongmai, LOW);
    digitalWrite(Momai, LOW);
      }
}
//button -> button2, buttonStatus -> buttonStatus2
if (temperature > 15 && temperature <27 || cambienmuaStatus < 20){
  digitalWrite(Momai, HIGH);
  digitalWrite(Dongmai, LOW);
  int buttonStatus2 = digitalRead(CTHT2);
  if (buttonStatus2 == 0){
    digitalWrite(Dongmai, LOW);
    digitalWrite(Momai, LOW);
  }
}
}

//độ ẩm cao thì bật quạt và ngược lại
void autoQuat(){
  float humidity;
  humidity = dht.readHumidity();
 if ( humidity > HUMIDITY ){
  digitalWrite(relayQuat, LOW);
 }
 else {
  digitalWrite(relayQuat, HIGH);
 }
  }

//để bật đèn cần xét các yếu tố như sau:
//ban ngày: trời mưa và nhiệt độ trung bình trở lên
//ban ngày: trời không mưa và nhiệt độ trung bình thấp
void autoDen(){
  float cambienmuaStatus = analogRead(cambienmua);
  float temperature;
  temperature = dht.readTemperature();
//  if (cambienmuaStatus > 20 && temperature > 22.5 || temperature > 22.5)
//  {
//    digitalWrite(relayDen, LOW); 
//  }
  if (temperature > TEMPERATURE)
  {
    digitalWrite(relayDen, LOW); 
  }
  else if (temperature < TEMPERATURE || temperature > 24){
    digitalWrite(relayDen, HIGH);
  }
}

//nếu đất khô bơm sẽ được bật và ngược lại
void autoBom() {
  float moist;
  moist = getMoist();
  Serial.print("Giá trị moist: ");
  Serial.println(moist);
  if (moist < MOIST) {
    digitalWrite(relayBom, LOW);
  }
  else {
    digitalWrite(relayBom, HIGH);
  }
}
float getMoist() {
  float i = 0;
  float anaValue = 0;
  for (i = 0; i < 10; i++)
  {
    anaValue += analogRead(SOIL_MOIST_1_PIN);
    delay(50);
  }
  anaValue = anaValue / (i);
  float percent = map(anaValue, 0, 4095, 0, 100);
  percent = 100 - percent;
  return percent;

}
void thingConnect() {
  if (client1.connect(server, 80)) {
    temp  = dht.readTemperature();
    humi  = dht.readHumidity();
    moist = getMoist();
    String body = "field1=" + String(temp, 1) + "&field2=" + String(humi, 1) + "&field3=" + String(moist, 1);
    client1.print("POST /update HTTP/1.1\n");
    client1.print("Host: api.thingspeak.com\n");
    client1.print("Connection: close\n");
    client1.print("X-THINGSPEAKAPIKEY: " + writeAPIKey + "\n");
    client1.print("Content-Type: application/x-www-form-urlencoded\n");
    client1.print("Content-Length: ");
    client1.print(body.length());
    client1.print("\n\n");
    client1.print(body);
    client1.print("\n\n");
  }
}

//cập nhật code bằng mạng nội bộ
void OTAupdate(){
  WiFi.mode(WIFI_STA);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  // ArduinoOTA.setPort(3232);

  // ArduinoOTA.setHostname("myesp32");

  // ArduinoOTA.setPassword("admin");

  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();
  ArduinoOTA.handle();
}

//cập nhật code ở bất kì đâu có mạng
void InstallUpdates(){
  WiFi.mode(WIFI_STA);
  Serial.println(fwImageURL);
  HTTPClient client;
  client.begin(fwImageURL);
//  int httpCode = client.GET();
//  if (httpCode == 200){
  t_httpUpdate_return ret = httpUpdate.update(client, fwImageURL);

    switch (ret) {
      case HTTP_UPDATE_FAILED:
        Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
        break;

      case HTTP_UPDATE_NO_UPDATES:
        Serial.println("HTTP_UPDATE_NO_UPDATES");
        break;

      case HTTP_UPDATE_OK:
        Serial.println("HTTP_UPDATE_OK");
        break;
    }
//  }
    client.end();
  }
