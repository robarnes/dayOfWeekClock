#include <ESP8266WiFi.h>  //For ESP8266
#include <PubSubClient.h> //For MQTT
#include <ESP8266mDNS.h>  //For OTA
#include <WiFiUdp.h>      //For OTA
#include <ArduinoOTA.h>   //For OTA
#include <Servo.h>        //For servo control

//WIFI and OTA configuration
#ifndef STASSID
#define STASSID "mySSID"
#define STAPSK  "myPASSWORD"
#define SENSORNAME  "dayOfWeekClock"
#endif
const char* ssid = STASSID;
const char* password = STAPSK;
const char* hostname = SENSORNAME;

//MQTT configuration
#define mqtt_server "mqtt.widgetninja.net"
#define mqtt_user "myUSER"
#define mqtt_password "myPASSWORD"
#define sensorName "dayOfWeekClock"

//MQTT Topic configuration
#define mqttTopic "/dayOfWeek"

//MQTT client
WiFiClient espClient;

// MQTT Callback function header
void callback(char* topic, byte* payload, unsigned int length);
PubSubClient mqtt_client(espClient);

//Necesary to make Arduino Software autodetect OTA device
WiFiServer TelnetServer(8266);

//Setup Servo
Servo myservo;    // create servo object to control a servo

//Role-Specific definitions
int firstBoot = 1;
const long interval = 300000;
unsigned long lastActionMillis;
int servoPosition = 0;
int sweepDelay = 50; //in milliseconds
int monPos = 10;
int tuePos = 48;
int wedPos = 67;
int thuPos = 90;
int friPos = 113;
int satPos = 133;
int sunPos = 155;

void setup() {
  Serial.begin(115200);
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  ArduinoOTA.setHostname(hostname);
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("OK");
  Serial.println("Configuring MQTT server...");
  mqtt_client.setServer(mqtt_server, 1883);
  mqtt_client.setCallback(callback);
  Serial.printf("   Server IP: %s\r\n", mqtt_server);
  Serial.printf("   Username:  %s\r\n", mqtt_user);
  Serial.printf("   Client Id: %s\r\n", sensorName);
  Serial.println("   MQTT configured!");
  Serial.println("Setup completed! Running app...");
  //myservo.attach(2);  //pin D4
}

// For when message show up in MQTT for subscribed topic
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  if (strcmp(topic, mqttTopic) == 0) { //this actually means we matched!
    Serial.println("Matched topic for clock.");
    if (firstBoot == 1) { //first boot and first message
      firstBoot = 0;
      Serial.println("First Boot");
      if ((char)payload[0] == '1') { //day of week
        myservo.write(sunPos); //preload servo location to yesterday (most likely last position)
      } else if ((char)payload[0] == '2') { //day of week
        myservo.write(monPos); //preload servo location to yesterday (most likely last position)
      } else if ((char)payload[0] == '3') { //day of week
        myservo.write(tuePos); //preload servo location to yesterday (most likely last position)
      } else if ((char)payload[0] == '4') { //day of week
        myservo.write(wedPos); //preload servo location to yesterday (most likely last position)
      } else if ((char)payload[0] == '5') { //day of week
        myservo.write(thuPos); //preload servo location to yesterday (most likely last position)
      } else if ((char)payload[0] == '6') { //day of week
        myservo.write(friPos); //preload servo location to yesterday (most likely last position)
      } else if ((char)payload[0] == '7') { //day of week
        myservo.write(satPos); //preload servo location to yesterday (most likely last position)
      }
    }
    if ((char)payload[0] == '1') { //day of week
      moveServo(monPos); //move servo
    } else if ((char)payload[0] == '2') { //day of week
      moveServo(tuePos); //move servo
    } else if ((char)payload[0] == '3') { //day of week
      moveServo(wedPos); //move servo
    } else if ((char)payload[0] == '4') { //day of week
      moveServo(thuPos); //move servo
    } else if ((char)payload[0] == '5') { //day of week
      moveServo(friPos); //move servo
    } else if ((char)payload[0] == '6') { //day of week
      moveServo(satPos); //move servo
    } else if ((char)payload[0] == '7') { //day of week
      moveServo(sunPos); //move servo
    } else if ((char)payload[0] == '9') { //reset request
      Serial.println("Resetting");
      ESP.restart();
    } else {
      Serial.println("We have an error on matching what day it is.");
    }
  }
}

void mqtt_reconnect() {
  // Loop until we're reconnected
  while (!mqtt_client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (mqtt_client.connect(sensorName, mqtt_user, mqtt_password)) {
      Serial.println("connected");
      mqtt_client.subscribe(mqttTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqtt_client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

int moveServo(int position) {
  Serial.println("Attaching Servo...");
  int servoPos = myservo.read();
  Serial.print("Current position: ");
  Serial.println(servoPos);
  Serial.print("Moving to: ");
  Serial.println(position);
  if (servoPos < position) {
    myservo.attach(4);  //pin D2
    delay(2000);
    for (int i = servoPos; i <= position; i++) {
      Serial.print(".");
      myservo.write(i);
      delay(sweepDelay);
    }
    Serial.println(" ");
    Serial.println("Detaching Servo...");
    delay(2000);
    myservo.detach();
  } else if (servoPos > position) {
    myservo.attach(4);  //pin D2
    delay(2000);
    for (int i = servoPos; i >= position; i--) {
      Serial.print(".");
      myservo.write(i);
      delay(sweepDelay);
    }
    Serial.println(" ");
    Serial.println("Detaching Servo...");
    delay(2000);
    myservo.detach();
  }
}

void loop() {
  ArduinoOTA.handle();
  if (!mqtt_client.connected()) {
    mqtt_reconnect();
  }
  mqtt_client.loop();
}
