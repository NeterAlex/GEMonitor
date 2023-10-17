#define BLINKER_WIFI
#define OLED_RESET -1
#define BuzzPin D2
#define RelayPin D6
#define TouchPin D7
#define GreenPin D8
#define BluePin D9
#define TrigPin D10
#define EchoPin D11
#define DHTPin D12
#define PIRPin D3
#define LdrPin A0
#define FlamePin A1
#define DHTType DHT11

#include <Blinker.h>
#include <Wire.h> 
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

// WiFi Configuration
char auth[] = "";
char ssid[] = "Wpa";
char pswd[] = "";

// Init global objects
DHT dht(DHTPin, DHTType);
WiFiUDP ntpUDP;
Adafruit_SSD1306 display(128, 64, &Wire, OLED_RESET);
NTPClient timeClient(ntpUDP, "ntp1.aliyun.com",60*60*8, 30*60*1000);
int LdrV = 0;
int touchState = 0;
double dis = 0;
int pirV = 0, pirC = 0, pirLock = 0;
String pirTime = "";
uint32_t read_time = 0;
float humi_read, temp_read;
BlinkerNumber LUMI("lumi");
BlinkerText PIRREC("pirrec");
BlinkerNumber PIRC("pirc");

BlinkerButton Button1("btn-light");
BlinkerButton Button2("btn-relay");

void button1_callback(const String & state)
{
    BLINKER_LOG("Toggle LED D2: ", state);
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}

void button2_callback(const String & state)
{
    BLINKER_LOG("Toggle Relay: ", state);
    digitalWrite(RelayPin, !digitalRead(RelayPin));
}

void dataRead(const String & data)
{
    BLINKER_LOG("Blinker readString: ", data);

    uint32_t BlinkerTime = millis();

    Blinker.vibrate();        
    Blinker.print("millis", BlinkerTime);

    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}

void dataStorage() {
    Blinker.dataStorage("tempS", temp_read);
    Blinker.dataStorage("humiS", humi_read);
}

long getTime()
{
  digitalWrite(TrigPin,HIGH);
  delayMicroseconds(10);
  digitalWrite(TrigPin,LOW);
  return pulseIn(EchoPin,HIGH);
}

void PIRSensor() {
    pirV = digitalRead(PIRPin);
    if(pirV == HIGH){
      PIRREC.print(timeClient.getFormattedTime());
      pirTime = timeClient.getFormattedTime();
      Blinker.delay(500);
      if(pirV == HIGH){
          pirLock = 1;
      }
    } 
    else if (pirLock ==1 && pirV == LOW) {
      pirLock = 0;
      pirC++;
      PIRC.print(pirC);
    }
    else{
      Serial.println("nothing detected");
    }
}

void displayOLED(){
    display.clearDisplay();
    display.setCursor(0,0);
    display.print("[Temp] ");
    display.print(temp_read);
    display.println("C");
    display.print("[Humi] ");
    display.print(humi_read);
    display.println("%");
    display.print("[Lumi] ");
    display.print(LdrV);
    display.println("lux");
    display.print("[PIRT] ");
    display.print(pirTime);
    display.println("");
    display.println("");
    display.print("Distance:");
    display.print(dis);
    display.println("cm");
    display.drawRect(0, 52, 128, 12, WHITE);
    display.fillRect(2, 54, map(dis, 2, 30, 2, 124), 8, WHITE);
    display.display();
}

void setup()
{
    // Init Serial
    Serial.begin(115200);
    BLINKER_DEBUG.stream(Serial);
    BLINKER_DEBUG.debugAll();
    // Init HC-SR04
    pinMode(TrigPin, OUTPUT);
    pinMode(EchoPin, INPUT);
    // Init PIR
    pinMode(PIRPin, INPUT);
    // Init Relay
    pinMode(RelayPin, OUTPUT);
    digitalWrite(RelayPin, HIGH);
    // Init TouchSensor
    pinMode(TouchPin, INPUT);
    // Init D2 LED
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
    // Init Blinker
    Blinker.begin(auth, ssid, pswd);
    Blinker.attachData(dataRead);
    Blinker.attachRTData(rtData);
    Blinker.attachDataStorage(dataStorage);
    Button1.attach(button1_callback);
    Button2.attach(button2_callback);
    // Init DHT11
    dht.begin();
    // Init Display
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.setTextColor(WHITE);
    display.clearDisplay();
    display.setTextSize(1);
    // Init Internet Time Server
    timeClient.begin();
}

void rtData(){
    Blinker.sendRtData("dis", dis);
    Blinker.sendRtData("lumi", LdrV);
    Blinker.sendRtData("temp", temp_read);
    Blinker.sendRtData("humi", humi_read);
    Blinker.printRtData();
}

void loop() {
    // Blinker handle
    Blinker.run();
    Blinker.delay(1000);

    // Handle NTP Time info
    timeClient.update();
    //Serial.println(timeClient.getFormattedTime());

    // Collect sensor data
    // LDR
    LdrV = analogRead(LdrPin);
    // DHT11
    humi_read = dht.readHumidity();
    temp_read = dht.readTemperature();
    // HC-SR04
    dis = getTime()/ 58.3;
    // PIR
    PIRSensor();
    // OLED
    displayOLED();
}













