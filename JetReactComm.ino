
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <MQTT.h>

// HINT:  provide the following information
//
// WiFi connection: SSID and Password
const char* ssid = "XRAY";
const char* password = "1111111111";

// MQTT server params: server, port, user, password, and unique clientid
const char* mqtt_server = "m13.cloudmqtt.com";
const int mqtt_port = 18513; 
const char *mqtt_user = "pzrqufih";
const char *mqtt_pass = "Nrj1sf3Edw9E";
const char *mqtt_topic = "JetReact";            // prefix with your initials, so you don't interference with your fellow participants
String mqtt_clientid = "";

WiFiClient espClient;
PubSubClient client(espClient, mqtt_server, mqtt_port);

// your button press input
const int buttonPin = D2; 
int buttonPushCounter = 0;   // counter for the number of button presses
int buttonState = 0;         // current state of the button
int lastButtonState = 0;     // previous state of the button


SoftwareSerial mySerial(D6, D5); // RX, TX


// Connect to WiFi network
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
}

// Connect to MQTT broker
void reconnect() {
  while (!client.connected()) {
    mqtt_clientid = String(ESP.getChipId());
    Serial.print("Attempting MQTT connection...");
    Serial.print(mqtt_clientid);
    Serial.print("  ");
    Serial.print(mqtt_user);
    
    // Attempt to connect
    if (client.connect(MQTT::Connect(mqtt_clientid).set_auth(mqtt_user, mqtt_pass))) {
      Serial.println("MQTT connected");
    } 
    else {
      Serial.print("failed, rc=");
      // Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(19200);
  Serial.println("Goodnight moon!");

  // set the data rate for the SoftwareSerial port
  mySerial.begin(9600);

  setup_wifi();
}

void loop() { 
  String content = "";
  char character;

  while(mySerial.available()) {
      delay(10);
      character = mySerial.read();
      content.concat(character);
  }

  if (content != "") {
    Serial.println(content);
    // publish button press count
    client.publish(mqtt_topic, content);
    
  }

  if (!client.connected()) {
    reconnect();
  }
  
  client.loop();

  
  
}


