#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <FirebaseESP32.h>


// #include <ESP8266WiFi.h>
//Relays for switching appliances
#define Relay1            26
#define Relay2            27
#define Relay3            14
#define Relay4            12
#define Relay5            4
#define Relay6            16

//DHT11 for reading temperature and humidity value
#define DHTPIN            23
#define DHTTYPE DHT11     // DHT 11

DHT dht(DHTPIN, DHTTYPE);

// Update these with values suitable for your network.
// const char* ssid = "Rith";
// const char* password = "11111111";
const char* ssid = "AIFARM ROBOTICS FACTORY";
const char* password = "@AIFARM2022";
const char* mqtt_server = "broker.hivemq.com"; 
const int mqttPort = 1883; //HiveMQ Port
const char* username = "narith";
const char* pass = "rith@11";

// Firebase Realtime Database parameters
// const char* firebase_host = "https://test1-5d3ed-default-rtdb.asia-southeast1.firebasedatabase.app";
// const char* firebase_auth = "RXF6wuGJzwynvtozNwKjZCLWJqBJLEgjjQdCOle7";

// const char* firebase_host = "https://project-46176-default-rtdb.asia-southeast1.firebasedatabase.app";
// const char* firebase_auth = "AIzaSyANT396h2oJsvHKD40ygo1YgilQYyCuJP4";

const char* firebase_host = "https://apphydro-5e7fb-default-rtdb.asia-southeast1.firebasedatabase.app";
const char* firebase_auth = "CQLZ4lCVvWkn17nA6vetLraBDYOrbmc8yemwsg2s";
const char* firebase_path_hum = "Data/Humidity";
const char* firebase_path_temp = "Data/Temperature";

unsigned long timer = 0;
const long utcOffsetInSeconds = 25200;

// Initialize WiFi and Firebase
WiFiClient wifiClient;
FirebaseData firebaseData;

WiFiUDP ntpUDP;
// initialized to a time offset of 10 hours
// NTPClient timeClient(ntpUDP,"pool.ntp.org", 36000, 60000);
NTPClient timeClient(ntpUDP,"pool.ntp.org",25200);
//.................................
// String DBnm = "DHT11Sensor";
String Temperatur_Path = "DeviceSensor/Temperature";
String Humidity_Path = "DeviceSensor/Humidity";
String TD = "Tem";
String HD = "Hum";
int count = 0;

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;
String fireStatus = "";

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(9600);
  dht.begin();
  setup_wifi();
  Firebase.begin(firebase_host, firebase_auth);
  //.......................
  timeClient.begin();

  // Start reading pH sensor data
  pinMode(Relay1, OUTPUT);
  pinMode(Relay2, OUTPUT);
  pinMode(Relay3, OUTPUT);
  pinMode(Relay4, OUTPUT);
  pinMode(Relay5, OUTPUT);
  pinMode(Relay6, OUTPUT);
  // controlrelay();
}

void loop() {

  if(count < 5) {
    count++;
    timeClient.update();
  }

    // String hr, mn, sc;
    String yr, date, day, hr, mn, sc;
    // if (timeClient.getYear() < 10) {
    //   yr = "0" + String(timeClient.getYear());
    // }
    // else {
    //   yr = String(timeClient.getYear());
    // }

    if (timeClient.getEpochTime() < 10) {
      date = "0" + String(timeClient.getEpochTime());
    }
    else {
      date = String(timeClient.getEpochTime());
    }

    // if (timeClient.getDay() < 10) {
    //   day = "0" + String(timeClient.getDay());
    // }
    // else {
    //   day = String(timeClient.getDay());
    // }
    
    if (timeClient.getHours() < 10) {
      hr = "0" + String(timeClient.getHours());
    }
    else{
      hr = String(timeClient.getHours());
    }

    if (timeClient.getMinutes() <10) {
      mn = "0" + String(timeClient.getMinutes());
    }
    else{
      mn = String(timeClient.getMinutes());
    }

    if (timeClient.getSeconds() < 10) {
      sc = "0" + String(timeClient.getSeconds());
    }
    else{
      sc = String(timeClient.getSeconds());
    }
    // String TimeNow = hr + ":" + mn + ":" + sc;
    String TimeNow = date + " " + hr + ":" + mn + ":" + sc;
    // String TimeNowHum = hr + ":" + mn;
    // String TimeNowTem = hr + ":" + mn;
    Serial.print(TimeNow);

    //.....................................
  int h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("failed to load DHT sensor !");
    delay(1000);
    return;
  }
  //...........................

  Serial.print("Humidity: ");
  Serial.println(h);
  Serial.print("%");

  Serial.print("Temperature: ");
  Serial.println(t);
  Serial.print("C");
  Serial.println("");

  String strHum = String(h) + "%";
  String strTem = String(t) + "C";
  //....................

  String firebasePathhum = String(firebase_path_hum) + "%";
  String firebasePathtemp = String(firebase_path_temp) + "C";
  // String DBaddH = Humidity_Path + "/" + TimeNow + "/" + HD;
  // String DBaddT = Temperatur_Path + "/" + TimeNow + "/" + TD;
  String DBaddH = Humidity_Path + "/" + TimeNow;
  String DBaddT = Temperatur_Path + "/" + TimeNow;
  
  Firebase.setString(firebaseData,DBaddH,strHum);
  Firebase.setString(firebaseData,DBaddT,strTem);

    if (Firebase.getString(firebaseData, "/Switch_Data/Switch_1")){
    Serial.println(firebaseData.stringData());
    if (firebaseData.stringData() == "false") {
    digitalWrite(Relay1, HIGH);
    Serial.print("Relay 1 ON");
    }
    else if (firebaseData.stringData() == "true"){
    digitalWrite(Relay1, LOW);
    Serial.print("Relay 1 OFF");
    }
    }

    if (Firebase.getString(firebaseData, "/Switch_Data/Switch_2")){
    Serial.println(firebaseData.stringData());
    if (firebaseData.stringData() == "false") {
    digitalWrite(Relay2, HIGH);
    Serial.print("Relay 2 ON");
    }
    else if (firebaseData.stringData() == "true"){
    digitalWrite(Relay2, LOW);
    Serial.print("Relay 2 OFF");
    }
    }

    if (Firebase.getString(firebaseData, "/Switch_Data/Switch_3")){
    Serial.println(firebaseData.stringData());
    if (firebaseData.stringData() == "false") {
    digitalWrite(Relay3, HIGH);
    Serial.print("Relay 3 ON");
    }
    else if (firebaseData.stringData() == "true"){
    digitalWrite(Relay3, LOW);
    Serial.print("Relay 3 OFF");
    }
    }

    if (Firebase.getString(firebaseData, "/Switch_Data/Switch_4")){
    Serial.println(firebaseData.stringData());
    if (firebaseData.stringData() == "false") {
    digitalWrite(Relay4, HIGH);
    Serial.print("Relay 4 ON");
    }
    else if (firebaseData.stringData() == "true"){
    digitalWrite(Relay4, LOW);
    Serial.print("Relay 4 OFF");
    }
    }

    if (Firebase.getString(firebaseData, "/Switch_Data/Switch_5")){
    Serial.println(firebaseData.stringData());
    if (firebaseData.stringData() == "false") {
    digitalWrite(Relay5, HIGH);
    Serial.print("Relay 5 ON");
    }
    else if (firebaseData.stringData() == "true"){
    digitalWrite(Relay5, LOW);
    Serial.print("Relay 5 OFF");
    }
    }

    if (Firebase.getString(firebaseData, "/Switch_Data/Switch_6")){
    Serial.println(firebaseData.stringData());
    if (firebaseData.stringData() == "false") {
    digitalWrite(Relay6, HIGH);
    Serial.print("Relay 6 ON");
    }
    else if (firebaseData.stringData() == "true"){
    digitalWrite(Relay6, LOW);
    Serial.print("Relay 6 OFF");
    }
    }
delay(2000);
}
