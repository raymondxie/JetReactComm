
#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <MQTT.h>
#include <WiFiManager.h>
#include <Ticker.h>

//WiFiManager - allow setting to a new WiFi network
WiFiManager wifiManager;
Ticker ticker;
// long button press input for resetting WiFi network


// HINT:  provide the following information if you have fixed WiFi SSID
//
// WiFi connection: SSID and Password
const char* ssid = "XRAY";
const char* password = "1111122222";

// MQTT server params: server, port, user, password, and unique clientid
const char* mqtt_server = "m13.cloudmqtt.com";
const int mqtt_port = 18513; 
const char *mqtt_user = "pzrqufih";
const char *mqtt_pass = "Nrj1sf3Edw9E";
const char *mqtt_topic = "JetReact"; 
String mqtt_clientid = "";

WiFiClient espClient;
PubSubClient client(espClient, mqtt_server, mqtt_port);
SoftwareSerial mySerial(D6, D5); // RX, TX


// for detecting quick or long press
const int CTRL_BUTTON=D0;

long buttonTimer = 0;
long longPressTime = 5000;  // how long to be registered as long-press, to reset WiFi SSID/password
boolean buttonActive = false;
boolean longPressActive = false;

// for debounce
int buttonState;
int lastButtonState = HIGH; // INPUT_PULLUP, so initially on HIGH
long lastDebounceTime = 0;  // last time output is toggle
long debounceDelay = 50;    // debounce time (filtering noise)

void tick()
{
  //toggle state
  int state = digitalRead(BUILTIN_LED);  // get the current state of GPIO1 pin
  digitalWrite(BUILTIN_LED, !state);     // set pin to the opposite state
}

//gets called when WiFiManager enters configuration mode
void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
  //entered config mode, make led toggle faster
  ticker.attach(0.2, tick);
}

// erase cached WiFi info: SSID and password, so that you can reboot and configure to a new AP
void resetWifi() {
  //reset settings - to clear the cached SSID and password
  wifiManager.resetSettings();
  delay(2000);
  ESP.restart();
  delay(1000);
}


// Connect to WiFi network, if we have hard-coded SSID/password
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

// detect button press:  short or long
void detectButtonPress() {
  // check the button press or long press
  int reading = digitalRead( CTRL_BUTTON );
  if (reading != lastButtonState) {
    // reading changed either by switch or noise, reset the debouncing timer
    lastDebounceTime = millis();
  }

  // whatever the reading, it has been there for longer than debounceTime, so it is real
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if ( reading == LOW) {    
      if (buttonActive == false) {
        // Serial.println("button presed");
        buttonActive = true;
        buttonTimer = millis();
      }
  
      if ((millis() - buttonTimer > longPressTime) && (longPressActive == false)) {
        // triggered while in pressed state
        longPressActive = true;        
        Serial.println("long press detected -- do something now");

        resetWifi();
      }
    } 
    else {
      if (buttonActive == true) {
        if (longPressActive == true) {
          // Serial.println("long press De-activated");
          longPressActive = false;
        } 
        else {
          // a button just released and it is a short press
          // triggered at release time
          Serial.println("short press happened - do something");
        }
  
        // Serial.println("button released");
        buttonActive = false;
      }
    }    
  }

  // set it for next loop
  lastButtonState = reading;

}

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(19200);
  Serial.println("Goodnight moon!");

  // set the data rate for the SoftwareSerial port
  mySerial.begin(9600);

  pinMode( BUILTIN_LED, OUTPUT);
  pinMode( CTRL_BUTTON, INPUT_PULLUP);  // press- LOW, release - HIGH

  // if fixed to a known WiFi network (SSID)
  // setup_wifi();

  ticker.attach(1.0, tick); // blink LED to indicate STATION mode
  
  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wifiManager.setAPCallback(configModeCallback);

  Serial.println("Trying to connect to WiFi network");
  //fetches ssid and password and tries to connect
  //if it does not connect it starts an access point with the specified name "AP-SmartBadge"
  //and goes into a blocking loop awaiting configuration
  if (!wifiManager.autoConnect("AP-StringPacman", "pacman")) {
    Serial.println("failed to connect to new WiFi network and hit timeout");
  }

  //if you get here you have connected to the WiFi
  Serial.print("network connected to ..."); Serial.println(WiFi.SSID());

  ticker.detach();
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

  detectButtonPress();

  if (!client.connected()) {
    reconnect();
  }
  
  client.loop();
}


