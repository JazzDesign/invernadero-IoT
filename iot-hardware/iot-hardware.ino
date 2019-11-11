

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include <BME280I2C.h>
#include <Wire.h>

// Network constants
#define TEAM_NAME "node"
int PUBLISH_PERIOD = 1000;

char json[] = "{\"id\":\"id01\",\"type\":\"input\",\"sensor\":\"323\"}"; 


//IDS Generators
long randomId;
char* randomChar[40] = {"a", "b", "c", "d", "e", "f","g", "h", "i", "j", "k", "l", "m", "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z"};

String idDispositivo = "id00";
String idOutput1 = "id02";
String idOutput2 = "id03";
String idInput = "id01";

//Spected Message

//String example = "{\"id\":\"12y\",\"type\":\"input\",\"sensor\":\"400\"}";
// id01,ON,true;
// 1i,0,1000;

//==============

const char* ssid = "Nexxt_36F1B0";
const char* password = "";
const char* mqtt_server = "192.168.0.111";

// I/O constants
#define TEMP_PIN A0
#define LED_PIN D5
#define BOMB_PIN D6


const char openB = '{';
const char closeB = '}';

char *token,*ok1,*ok2;

int red=0;
int blue = 0;
int green = 0;

int redPin = D6;
int greenPin = D8;
int bluePin = D7;

//Button
int buttonPin = D1;

// Variables will change:
int buttonPushCounter = 0;   // counter for the number of button presses
int buttonState = 0;         // current state of the button
int lastButtonState = 0;  //

// data variables
int lastReadingTime = 0;
double temp = 0;

char msg[150];
char cnt[50];
char msg_r[100];
char topic_name[250];

// network variables
WiFiClient espClient;
PubSubClient mqtt_client(espClient);

  
// temp
BME280I2C::Settings mysettings; 
//(
//  BME280I2C::OSR_X1,
//  BME280I2C::OSR_X1,
//  BME280I2C::OSR_X1,
//  BME280I2C::Mode_Forced,
//  BME280I2C::StandbyTime_1000ms,
//  BME280I2C::Filter_Off,
//  BME280I2C::SpiEnable_False,
//  0x77
//);

BME280I2C bme(mysettings);    // Default : forced mode, standby time = 1000 ms
                  // Oversampling = pressure ×1, temperature ×1, humidity ×1, filter off,



void generateIdDispositivo(){
  String str(random(10, 90));
  String str2(randomChar[random(0, 25)]);
  
  idDispositivo = str + str2;
  Serial.print("ID DISPOSITIVO: ");
  Serial.println(idDispositivo);
}


void generateIdOutput1(){
  String str(random(10, 90));
  String str2(randomChar[random(0, 25)]);
  
  idOutput1 = str + str2;
  Serial.print("ID Output1: ");
  Serial.println(idOutput1);
}

void generateIdOutput2(){
  String str(random(10, 90));
  String str2(randomChar[random(0, 25)]);
  
  idOutput2 = str + str2;
  Serial.print("ID Output2: ");
  Serial.println(idOutput2);
}

void generateIdInput(){
  String str(random(10, 90));
  String str2(randomChar[random(0, 25)]);
  
  idInput = str + str2;
  Serial.print("ID Input: ");
  Serial.println(idInput);
}


void sendFirstMessage(){
  String data = "{\"iDis\":\"";
    data += idDispositivo;
    data += "\",\"iOut-1\":\"";
    data += idOutput1;
    data += "\",\"desOut-1\":\"Luces\"";
    data += ",\"iOut-2\":\"";
    data += idOutput2;
    data += "\",\"desOut-2\":\"Agua\"";
    data += ",\"iIn-1\":\"";
    data += idInput;
    data += "\",\"desIn-1\":\"Temp\"";
    data += "}";

    data.toCharArray(msg, 150);
    mqtt_client.publish(getTopic("response"), msg);  
}

void receiveJson(String str){
  String receiveId = getValue(str,',',0);

  if(receiveId == idOutput1){
//    Serial.println("ES OUTPUT1");
    String receivedText = getValue(str, ',', 2);
//    Serial.println(receivedText);
    String receivedStatus = getValue(str, ',', 3);
//    Serial.println(receivedStatus);
    if(receivedText == "ON"){
      digitalWrite(LED_PIN, HIGH);
//      Serial.println("SE ENCENDIO");
    }else if(receivedText == "OFF"){
      digitalWrite(LED_PIN, LOW);
//      Serial.println("SE APAGO");
    }
  }else if(receiveId == idOutput2){
//    Serial.println("ES OUTPUT2");
    String receivedText = getValue(str, ',', 2);
//    Serial.println(receivedText);
    String receivedStatus = getValue(str, ',', 3);
//    Serial.println(receivedStatus);
    if(receivedText == "ON"){
      digitalWrite(BOMB_PIN, HIGH);
//      Serial.println("SE ENCENDIO");
    }else if(receivedText == "OFF"){
      digitalWrite(BOMB_PIN, LOW);
//      Serial.println("SE APAGO");
    }
  }else if(receiveId == idInput){
//    Serial.println("ES INPUT");
    String receivedFreq = getValue(str, ',', 3);
    PUBLISH_PERIOD = receivedFreq.toInt();
//    Serial.print("FRECUENCIA: ");
//    Serial.println(PUBLISH_PERIOD);

  }
}

//Obtenemos valor puro
String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void setupTempSensor() {
  Serial.println("Setting up BME280 Sensor");
  mysettings.bme280Addr = 0x77;
  bme = BME280I2C(mysettings);
  Wire.begin(D4, D3); // S
  while(!bme.begin())
  //while(!bme.begin(0x77))
  {
    Serial.println("Could not find BME280 sensor!");
    delay(1000);
  }

  switch(bme.chipModel())
  {
     case BME280::ChipModel_BME280:
       Serial.println("Found BME280 sensor! Success.");
       break;
     case BME280::ChipModel_BMP280:
       Serial.println("Found BMP280 sensor! No Humidity available.");
       break;
     default:
       Serial.println("Found UNKNOWN sensor! Error!");
  }
  
}
void setupMQTT() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  mqtt_client.setServer(mqtt_server, 1883);
  mqtt_client.setCallback(callback);
}

void setup() {
  Serial.begin(115200);
  setupMQTT();
  setupTempSensor();
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  pinMode(BOMB_PIN, OUTPUT);
  digitalWrite(BOMB_PIN, LOW);

  //Generate Unique Ids
//  generateIdDispositivo();
//  generateIdOutput1();
//  generateIdOutput2();
//  generateIdInput();

}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    msg_r[i] = (char)payload[i];
  }
  msg_r[length] = 0;
  Serial.print("'");
  Serial.print(msg_r);
  Serial.println("'");
  receiveJson(String(msg_r));
//  if (strcmp("ON", msg_r) == 0) {
//    Serial.println("LED ON");
//    digitalWrite(LED_PIN, HIGH);
//  } else if(strcmp("OFF", msg_r) == 0) {
//    Serial.println("LED OFF");
//    digitalWrite(LED_PIN, LOW);
//  }else{
//    Serial.print("RECIBIDO: ");
//    Serial.println(msg_r);
//    PUBLISH_PERIOD = atoi(msg_r);
//  }
}



void reconnect() {
  // Loop until we're reconnected
  while (!mqtt_client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqtt_client.connect(TEAM_NAME)) {
      Serial.println("connected");
      mqtt_client.subscribe(getTopic("response"));
//      sendFirstMessage();
    }else {
      Serial.print("failed, rc=");
      Serial.print(mqtt_client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void publish(char* topic, char* payload) {
  Serial.println(topic_name);
  mqtt_client.publish(topic_name, payload);
}

char* getTopic(char* topic) {
  sprintf(topic_name, "/%s", topic);
  return topic_name;
}

void loop() {
  if (!mqtt_client.connected()) {
    reconnect();
  }
  mqtt_client.loop();

  int actualTime = millis();
  if (actualTime - lastReadingTime > PUBLISH_PERIOD) {
    lastReadingTime = actualTime;

    float temp(NAN), hum(NAN), pres(NAN);
  
     BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
     BME280::PresUnit presUnit(BME280::PresUnit_Pa);
  
     bme.read(pres, temp, hum, tempUnit, presUnit);
    
//    Serial.println(temp);
    String str(temp);
//    str.toCharArray(msg, 50);

    String data = "{\"id\":\"id01\",\"type\":\"input\",\"sensor\":\"";
    data += str;
    data += "\"}";

    data.toCharArray(msg, 150);
    mqtt_client.publish(getTopic("devices"), msg);     
    delay(5000);
  }
}
