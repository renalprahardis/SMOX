#include <HX711.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#define DOUT D2
#define CLK D3
#define TRIGGER_PIN  D4
#define ECHO_PIN     D5 
#define vibPin D6
#define buzzPin D7
#define selenoidPin D1
HX711 scale(DOUT,CLK);
const char* ssid = "Esty 1008";
const char* password = "margonda4";
const char* mqtt_server = "ngehubx.online";
const char* topic = "inTopic";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[400];
int value = 0;
char message_buff[100];
double berat, beratkurang;
int val;
int nilaiBerat = 0;
String pesan = "";
String msgs="";

void setup_wifi() {

  delay(10);
  Serial.print("Connecting to ");
  Serial.println(ssid);
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

void callback(char* topic, byte* payload, unsigned int length) {
    int i = 0;
  for(i=0; i<length; i++) {
    message_buff[i] = payload[i];
  }
  message_buff[i] = '\0';
  
  String msgString = String(message_buff);
  pesan = msgString;
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.publish("outTopic", "hello world");
      client.subscribe("coba/kel7");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);    
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  pinMode(TRIGGER_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(vibPin, INPUT);
  pinMode(buzzPin,OUTPUT);
  pinMode(selenoidPin, OUTPUT);
  scale.set_scale(2280.f);  
  scale.tare();
}
long vibVel = 0;
int check = 0;

void loop() {
 if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
    long now = millis();
    double duration, distance;
   digitalWrite(TRIGGER_PIN, LOW);
  delayMicroseconds(2); 
  digitalWrite(TRIGGER_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGGER_PIN, LOW);
  duration = pulseIn(ECHO_PIN, HIGH);
  distance = (duration/2) / 29.1;
  Serial.print(distance);
  Serial.println(" cm");
    String mss = pesan;
    if(mss=="0"){
      digitalWrite(selenoidPin,HIGH);
    } else if(mss=="1"){
      digitalWrite(selenoidPin,LOW);
      check = 1;
    } else if(mss=="9"){
      berat = scale.get_units() * 1 ;
      nilaiBerat = berat-4;
      scale.power_down();  
      delay(50);
      scale.power_up();
      msgs = " Berat sebesar : " ;
      msgs=msgs+nilaiBerat;
    }
    delay(10);
    long vibVal=pulseIn (vibPin, HIGH);
    Serial.println(vibVal +" cm");
    if((vibVal>10000 || distance > 15) && check == 1){
      digitalWrite(buzzPin,HIGH);
      delay(500);
      digitalWrite(buzzPin,LOW);
      msgs = "Alert";
    } 
     char message[80];
     msgs.toCharArray(message,80);
     Serial.println(message);
     //publish sensor data to MQTT broker
    client.publish("inTopic", message);
}
