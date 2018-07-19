/*************************Headers***************************/
#include <Time.h>
#include <TimeLib.h>
#include <RunningMedian.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>



#define MQTT_KEEPALIVE 60


/************************Define Pins***************************/
// defines pins numbers
const int trigPin1 = D4;
const int echoPin1 = D3;  
const int trigPin2 = D6;
const int echoPin2 = D5;
/*********************** Global Variables***************************/
long duration1;
long duration2;
int distance1;
int distance2;
<<<<<<< HEAD:ultrasonic/ultrasonic.ino

int threshold = 10;
=======
int threshold = 3;    //deadband to ignore changes
>>>>>>> NJ-Test:ultrasonic/ultrasonic.ino
int baseline;
int lastHeight;
int prevLastHeight;
int prevNewHeight;
unsigned long connect_time;
unsigned long last_update;
unsigned long lastMeasure;
unsigned long newMeasure;
int delayval = 100; 
const char* ssid = "CNN";
const char* password = "Co!!eenandNei!";
const char* mqttServer = "192.168.1.13";
const int mqttPort = 1883;
<<<<<<< HEAD:ultrasonic/ultrasonic.ino

//EDIT THESE 3 VALUES
const char* clientName = "DeskNode2";
const char* subscribeTopic = "Status/DeskNode2";
const char* publishTopic = "Desks/DeskNode2";
=======
const char* clientName = "DeskNode8";
const char* topic_pub = "Desks/DeskNode8";    //write to this topic
const char* topic_pub2 = "Desks/DeskNode8req";    //write to this topic
const char* topic_sub = "Desks/DeskNode8/sub";  //listen to this topic
>>>>>>> NJ-Test:ultrasonic/ultrasonic.ino

WiFiClient espClient;         //wifi client
PubSubClient client(espClient); //MQTT client requires wifi client








/****************setup wifi************************************/
void setup_wifi() {

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
  long rssi = WiFi.RSSI();
  Serial.print("RSSI:");
  Serial.println(rssi);
}
/*****************Connect to MQTT Broker**********************************/
void ConnectBroker(PubSubClient client, const char* clientName)
{
    while (!client.connected())
    {
        Serial.print("Connecting to MQTT: ");
        Serial.println(clientName);
        if(client.connect(clientName))      //command to connect to MQTT broker with the unique client name
        {
          Serial.println("Connected");
        }
        else
        {
          Serial.print("Failed with state ");
          Serial.println(client.state());
          delay(200);
        }
    }
} 
/*****************reconnect to MQTT Broker if it goes down**********************************/
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect, just a name to identify the client
    if (client.connect(clientName)) {
      Serial.println("connected");
      
      
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

/*****************MQTT Listener******************************************************/
void callback(char* topic, byte* payload, unsigned int length2){
<<<<<<< HEAD:ultrasonic/ultrasonic.ino
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);

   
  Serial.print("Message: ");
  for(int i = 0; i<length2;i++){
    Serial.print((char)payload[i]);
  }

  Serial.println();
  Serial.println("-------------");

   

  payload[length2] = 0;

    StaticJsonBuffer<300> JSONbuffer; 
    String inData = String((char*)payload);
    JsonObject& root = JSONbuffer.parseObject(inData);
  
  String request = root["details"];

  if(request == "height"){
  
    JsonObject& JSONencoder = JSONbuffer.createObject();
    JSONencoder["currentHeight"] = getHeight();
    JSONencoder["previousRecordedHeight"] = lastHeight;
    char JSONmessageBuffer[100];
    JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
    client.publish(publishTopic, JSONmessageBuffer);
  }
         
=======
  if (strcmp(topic,"Desks/DeskNode8/sub")==0)
  {
      Serial.print("Message arrived in topic: ");
      Serial.println(topic);
      Serial.print("Message: ");
      
      for(int i = 0; i<length2;i++){
      Serial.print((char)payload[i]);
      }
      Serial.println ("");
    
       payload[length2] = 0;
    
        StaticJsonBuffer<300> JSONbuffer; 
        String inData = String((char*)payload);
        JsonObject& root = JSONbuffer.parseObject(inData);
      
      String request = root["system"];
    
      if(request == "diagnostics"){
        Serial.println("-----Getting Diagnostic Data--------");
        JsonObject& JSONencoder = JSONbuffer.createObject();
        JSONencoder["ID"] = clientName;
        JSONencoder["Connected"] = connect_time;
        JSONencoder["LastUpdate"] = last_update;
        JSONencoder["WiFiSig"] = WiFi.RSSI();
        JSONencoder["Height"] = getHeight();
        char JSONmessageBuffer[300];
        JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
        client.publish(topic_pub2, JSONmessageBuffer);
        Serial.println(JSONmessageBuffer);
      }
  }       
>>>>>>> NJ-Test:ultrasonic/ultrasonic.ino
}


/*****************checks for speed of measurements******************************************************/
unsigned long msDifference(){
  
  int timeDiff = newMeasure-lastMeasure;
  return timeDiff;
}

<<<<<<< HEAD:ultrasonic/ultrasonic.ino
void setup() {
  pinMode(trigPin1, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin1, INPUT); // Sets the echoPin as an Input
  pinMode(trigPin2, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin2, INPUT); // Sets the echoPin as an Input
  Serial.begin(115200);
  
  ConnectWifi(ssid,password);
  client.setServer(mqttServer,mqttPort);
  ConnectBroker(client, clientName);
  client.setCallback(callback);
  client.subscribe(subscribeTopic);
  // (timezone, daylight offset in seconds, server1, server2)
  // 3*3600 as setTimezone function converts seconds to hours
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  lastMeasure = millis();
  ////////////


  
   Serial.println("\nWaiting for time");
  while (!time(nullptr)) {
    Serial.print(".");
    delay(250);
  }
  Serial.println("Configured time.");

  ///
    /// Don't know why but it disconnects often without multiple client.loop() even with increased keepalive time
  ///
  client.loop();
    delay(2000);
    Serial.println("Getting current time");
    client.loop();
      sendStartupMessage();
    client.loop();
      Serial.print("Getting Baseline");
      makeBaseline();
      client.loop();
=======
>>>>>>> NJ-Test:ultrasonic/ultrasonic.ino



void sendStartupMessage(){
  
  time_t now = time(nullptr);
  connect_time = time(nullptr);
  Serial.println("Time is now: ");
  Serial.println(now);
  
  StaticJsonBuffer<100> JSONbuffer;
  JsonObject& JSONencoder = JSONbuffer.createObject();
  JSONencoder["id"] = clientName;
  JSONencoder["startuptime"] = now;
  char JSONmessageBuffer[100];
  JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
<<<<<<< HEAD:ultrasonic/ultrasonic.ino
  client.publish(publishTopic, JSONmessageBuffer, false);
=======
  client.publish(topic_pub, JSONmessageBuffer, false);
>>>>>>> NJ-Test:ultrasonic/ultrasonic.ino
}




<<<<<<< HEAD:ultrasonic/ultrasonic.ino
void loop() {
  if (!client.connected()) {
    reconnect();
    client.subscribe(subscribeTopic);
  }

  checkHeight();
  client.loop();
  delay(10);

}
=======
>>>>>>> NJ-Test:ultrasonic/ultrasonic.ino

void checkHeight() {
  if (!lastHeight) {
    lastHeight = baseline;
  }
  int newHeight = getHeight();
  if(passesSpeedCheck(newHeight,lastHeight)){
    if (abs(newHeight - lastHeight) >= threshold) {
      Serial.println("**********Sending*********");
      Serial.println("old: ");
      Serial.println(lastHeight);
      Serial.println("new: ");
      Serial.println(newHeight);
      prevLastHeight = lastHeight;
      prevNewHeight = newHeight;
      sendHeight(lastHeight, newHeight);
      lastHeight = newHeight;
      
      
      //newMeasure = millis();
    }
      lastMeasure = newMeasure;

  }
}

boolean passesSpeedCheck(int newHeight, int oldHeight){
  float distance = abs(newHeight-oldHeight);
  float timeDiff = msDifference();
  float dt = 1000*(distance/timeDiff);
    return dt<4.0;
  
}


void makeBaseline() {
  int total = 0;
  int baseSize = 3;
  RunningMedian measurements = RunningMedian(5);
  for (int i = 0; i < baseSize; i++) {
    measurements.add(getHeight());
  }
  baseline = measurements.getMedian();
  lastMeasure = millis();
  Serial.println("Baseline: ");
  Serial.println(baseline);


/*
StaticJsonBuffer<300> JSONbuffer;
  JsonObject& JSONencoder = JSONbuffer.createObject();
  JSONencoder["Baseline"] = baseline;
  char JSONmessageBuffer[200];
  JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));

  
 client.publish(topic_pub, JSONmessageBuffer, false);
*/



  
}

int getHeight() {
  int realDistance;

  RunningMedian measurements = RunningMedian(5);

  // Clears the trigPin
  for (int i = 0; i < 5; i++) {
    
    digitalWrite(trigPin1, LOW);
    digitalWrite(trigPin2, LOW);
    delayMicroseconds(2);
    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(trigPin1, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin1, LOW);
    duration1 = pulseIn(echoPin1, HIGH);

    delayMicroseconds(10);


    digitalWrite(trigPin2, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin2, LOW);
    // Reads the echoPin, returns the sound wave travel time in microseconds
    duration2 = pulseIn(echoPin2, HIGH);

    // Calculating the distance
    distance1 = duration1 * 0.034 / 2;
    distance2 = duration2 * 0.034 / 2;
    // Prints the distance on the Serial Monitor
    Serial.println("dist1");
    Serial.println(distance1);
    Serial.println("dist2");
    Serial.println(distance2);

    if (distance1 >= distance2) {
      realDistance = distance1;
    }
    else {
      realDistance = distance2;
    }
   


    measurements.add(realDistance);
  }

  realDistance = measurements.getMedian();

  
    newMeasure = millis();
  
  delay(250);
  return realDistance;
}

void sendHeight(int oldHeight, int newHeight) {
  
  last_update = time(nullptr);
  StaticJsonBuffer<300> JSONbuffer;
  JsonObject& JSONencoder = JSONbuffer.createObject();
  JSONencoder["id"] = clientName;
  JSONencoder["oldheight"] = oldHeight;
  JSONencoder["newheight"] = newHeight;
  JSONencoder["time"] = time(nullptr);
  JSONencoder["mscounter"] = newMeasure;
  char JSONmessageBuffer[200];
  JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));

  
 client.publish(topic_pub, JSONmessageBuffer, false);
}

/***************Setup Routine******************************************************/
void setup() {
  pinMode(trigPin1, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin1, INPUT); // Sets the echoPin as an Input
  pinMode(trigPin2, OUTPUT); // Sets the trigPin as an Output
  pinMode(echoPin2, INPUT); // Sets the echoPin as an Input
  Serial.begin(115200);
  
  setup_wifi();
  client.setServer(mqttServer,mqttPort);
  ConnectBroker(client, clientName);    //connect to MQTT borker
  client.setCallback(callback);
  client.subscribe(topic_sub);   

  
  // (timezone, daylight offset in seconds, server1, server2)
  // 3*3600 as setTimezone function converts seconds to hours
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  lastMeasure = millis();
  ////////////


  
   Serial.println("\nWaiting for time");
  while (!time(nullptr)) {
    Serial.print(".");
    delay(250);
  }
  Serial.println("Configured time.");

  ///
    /// Don't know why but it disconnects often without multiple client.loop() even with increased keepalive time
  ///
  client.loop();
    delay(2000);
    Serial.println("Getting current time");
    client.loop();
      sendStartupMessage();
      
    client.loop();
      Serial.print("Getting Baseline");
      makeBaseline();
      client.loop();
 


  
}

void loop() {
  if (!client.connected()) {
    reconnect();
    client.subscribe(topic_sub);
  }

  checkHeight();
  client.loop();
  delay(10);

<<<<<<< HEAD:ultrasonic/ultrasonic.ino
  client.publish(publishTopic, JSONmessageBuffer, false);
=======
>>>>>>> NJ-Test:ultrasonic/ultrasonic.ino
}

