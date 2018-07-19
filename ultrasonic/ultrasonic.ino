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
const int trigPin1 = D8;
const int echoPin1 = D7;  
const int trigPin2 = D6;
const int echoPin2 = D5;
/*********************** Global Variables***************************/
long duration1;
long duration2;
int distance1;
int distance2;

int movementThreshold = 10;

int baseline;
int baselineSize = 7;
int prevHeight;
int newHeight;
int heightCheckSize = 11;
int chunkSize = 7;
RunningMedian recordedHeights = RunningMedian(chunkSize);



unsigned long connect_time;
unsigned long last_update;

int delayval = 100; 

const char* ssid = "181BayCRETech";
const char* password = "LetsGoRaptors!";
const char* mqttServer = "192.168.0.11";
const int mqttPort = 1883;

//EDIT THESE 3 VALUES
const char* clientName = "DeskNode2";
const char* topic_pub = "Desks/DeskNode2";    //write to this topic
const char* topic_request_pub = "Desks/DeskNode2req";    //write to this topic
const char* topic_sub = "Desks/DeskNode2/sub";  //listen to this topic

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

void makeBaseline() {
  int total = 0;
  RunningMedian measurements = RunningMedian(baselineSize);
  for (int i = 0; i < baselineSize; i++) {
    measurements.add(getHeight());
  }
  baseline = measurements.getMedian();
  Serial.println("Baseline: ");
  Serial.println(baseline); 
}

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
  client.publish(topic_request_pub, JSONmessageBuffer, false);
}


void loop() {
  if (!client.connected()) {
    reconnect();
    client.subscribe(topic_sub);
  }

  checkHeight();
  client.loop();
  delay(10);
}



void checkHeight() {
  if (!prevHeight) {
    prevHeight = baseline;
  }
  for(int i = 0; i<chunkSize;i++){
    recordedHeights.add(getHeight());
  }
  int newHeight = recordedHeights.getMedian();
    if (abs(newHeight - prevHeight) >= movementThreshold) {
      Serial.println("**********Sending*********");
      Serial.println("old: ");
      Serial.println(prevHeight);
      Serial.println("new: ");
      Serial.println(newHeight);
      
      sendHeight(prevHeight, newHeight);
      prevHeight = newHeight;
      }

  
}


int getHeight() {
  int realDistance;

  RunningMedian measurements = RunningMedian(heightCheckSize);

  // Clears the trigPin
  for (int i = 0; i < heightCheckSize; i++) {
    
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

   
  delay(150);
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
  char JSONmessageBuffer[200];
  JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));

  
 client.publish(topic_pub, JSONmessageBuffer, false);
}


/*****************MQTT Listener******************************************************/
void callback(char* topic, byte* payload, unsigned int length2){
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
    JSONencoder["previousRecordedHeight"] = prevHeight;
    char JSONmessageBuffer[100];
    JSONencoder.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
    client.publish(topic_pub, JSONmessageBuffer);
  }
         
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
        client.publish(topic_request_pub, JSONmessageBuffer);
        Serial.println(JSONmessageBuffer);
      }
  }       
}

