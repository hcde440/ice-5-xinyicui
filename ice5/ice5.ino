//Libraries to include
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>

//Some immutable definitions/constants
#define ssid "University of Washington"
#define pass ""
#define mqtt_server "mediatedspaces.net"
#define mqtt_name "hcdeiot"
#define mqtt_pass "esp8266"

WiFiClient espClient;
PubSubClient mqtt(espClient);

#define OLED_RESET -1
Adafruit_SSD1306 display(OLED_RESET); //creating an ssd1306 instance named display

//Global variables
char espUUID[8] = "Sunny";
String tempF;
String tempC;
String humidity;
String pressure;
DynamicJsonBuffer jsonBuffer;

//function to do something when a message arrives from mqtt server

//{"uuid": "84:F3", "tempF": "72.68", "tempC": "22.60", "humidity": "33.50", "pressure": "101.83"}
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  JsonObject& root = jsonBuffer.parse(payload);

  if (!root.success()) {
    Serial.println("parseObject() failed");  
  }

  tempF = root["tempF"].as<String>();
  tempC = root["tempC"].as<String>();
  humidity = root["humidity"].as<String>();
  pressure = root["pressure"].as<String>();

  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Temp.   : ");
  display.print(tempF);
  display.println("'F");
  display.print("Temp.   : ");
  display.print(tempC);
  display.println("'C");
  display.print("Humidity: ");
  display.print(humidity);
  display.println("%");
  display.print("Pressure: ");
  display.print(pressure);
  display.println(" kPa");
  display.display();
}

//function to reconnect if we become disconnected from the server
void reconnect() {
  // Loop until we're reconnected
  while (!espClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    // Attempt to connect
    if (mqtt.connect(espUUID, mqtt_name, mqtt_pass)) { //the connction
      Serial.println("connected");
      // Once connected, publish an announcement...
      char announce[40];
      strcat(announce, espUUID);
      strcat(announce, "is connecting. <<<<<<<<<<<");
      mqtt.publish(espUUID, announce);
      mqtt.subscribe("colinyb/weatherStation");
      
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqtt.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
//////////////////////////////////////SETUP
void setup()
{
  Serial.begin(115200);//for debugging code, comment out for production
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(); Serial.println("WiFi connected"); Serial.println();
  Serial.print("Your ESP has been assigned the internal IP address ");
  Serial.println(WiFi.localIP());

  mqtt.setServer(mqtt_server, 1883);
  mqtt.setCallback(callback);

  // set up the OLED display
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Starting up.");
  display.display();
}

////////////////////////////LOOP
void loop() {
  mqtt.loop();
  if (!mqtt.connected()) {
    reconnect();
  }

}
