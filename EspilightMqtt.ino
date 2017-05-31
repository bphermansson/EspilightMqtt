/*
 * https://github.com/bphermansson/EspilightMqtt
 * https://github.com/puuu/espilight
 * https://pilight.org/
*/

#include <ESPiLight.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>

// Wifi settings
const char* ssid = "NETGEAR83";
const char* password = "........";
const char* mqtt_server = "192.168.1.79";
const char* mqtt_user = "emonpi";
const char* mqtt_password = "emonpimqtt2016";

WiFiClient espClient;
PubSubClient client(espClient);

// Rf transmitter and receiver
#define RECEIVER_PIN 2 //any interupt able pin
#define TRANSMITTER_PIN 13
ESPiLight rf(TRANSMITTER_PIN);  //use -1 to disable transmitter

// callback function. It is called on successfully received and parsed rc signal
void rfCallback(const String &protocol, const String &message, int status, int repeats, const String &deviceID) {
  Serial.print("RF signal arrived [");
  Serial.print(protocol); //protocoll used to parse
  Serial.print("][");
  Serial.print(deviceID); //value of id key in json message
  Serial.print("] (");
  Serial.print(status);  //status of message, depending on repeat, either: 
                         // FIRST   - first message of this protocoll within the last 0.5 s
                         // INVALID - message repeat is not equal to the previous message
                         // VALID   - message is equal to the previous message
                         // KNOWN   - repeat of a already valid message
  Serial.print(") ");
  Serial.print(message); // message in json format
  Serial.println();

  // check if message is valid and process it
  if(status==3) {  
    Serial.print("Valid message: [");
    Serial.print(protocol);
    Serial.print("] ");
    Serial.print(message);
    Serial.println();

    // Send as Mqtt
    String topic = "EspilightMqtt/"+protocol;
    char ctopic[50];
    char cmessage[50];
    topic.toCharArray(ctopic, 50);
    message.toCharArray(cmessage, 50);
    bool res = client.publish(ctopic, cmessage, 50);
  }
}

void setup() {
  Serial.begin(115200);

  setup_wifi();
  client.setServer(mqtt_server, 1883);
  //client.setCallback(callback); // If a message arrives
  if (!client.connected()) {
    reconnect();
  }
  
  ota();  // Over the air programming

  //Set RF callback funktion
  rf.setCallback(rfCallback);
  //initialize receiver
  rf.initReceiver(RECEIVER_PIN);
  
}

// When a Mqtt message has arrived
// 
/*
void callback(char* topic, byte* payload, unsigned int length) {
  char message[14] ="";
  //time_t pctime = 0;
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
}
*/
void loop() {
  ArduinoOTA.handle();

  // RF receiver
  rf.loop();
  delay(10);
  
  // Mqtt
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
}
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("EspilightMqtt", mqtt_user, mqtt_password)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("EspilightMqtt", "Hello from EspilightMqtt");
      // ... and resubscribe
      //client.subscribe("time"); // Time is published on the network, we use it for time keeping
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  WiFi.hostname("EspilightMqtt");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

}
void ota(){
  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}
