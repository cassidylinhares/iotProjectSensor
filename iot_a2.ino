#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>

#define SENSORPIN A0

//Wifi 
const char* ssid = "Birbo Lan";
const char* password = "birbosHumbleAbod3";

//Domain name with URL path or IP address with path
const char *serverName = "https://plantpal-whv3b.ondigitalocean.app/insertMoistureLevel/";
const char *host = "plantpal-whv3b.ondigitalocean.app";

const char fingerprint[] PROGMEM = "B0:DC:04:57:60:5E:1C:1B:4C:95:63:C3:59:69:96:5F:0C:07:BC:E5";

//Timer
unsigned long lastTime = 0;
unsigned long timerDelay = 60000;

//Moisture Sensor
const int air = 720;
const int water = 400;
int moistureValue = 0;
int moisturePct = 0;

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
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
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

int read_sensor() {
  //read and map values
  moistureValue = analogRead(SENSORPIN);
  moisturePct = map(moistureValue, air, water, 0, 100);

  //check edge cases
  if(moisturePct >= 100) {
    moisturePct = 100;
  } 
  
  if(moisturePct <= 0) {
    moisturePct = 0;
  }

  return moisturePct;
}

void post_sensor() {
  unsigned long now = millis();
  if (now- lastTime >= timerDelay) {
    lastTime = millis();
    WiFiClient client;
    WiFiClientSecure https;
    HTTPClient http;    //Declare object of class HTTPClient

    https.setFingerprint(fingerprint);
    Serial.printf("Using fingerprint '%s'\n", fingerprint);
    
    Serial.println("HTTPS Connecting");
    int r=0; //retry counter
    while((!https.connect(host, 443)) && (r < 30)){
        delay(100);
        Serial.print(".");
        r++;
    }
    if(r==30) {
      Serial.println("Connection failed");
    }
    else {
      Serial.println("Connected to web");
    }
  
    char postData[100];
    char plantId[10] = "plant1";
    int meter = read_sensor();
  
    //Post Data
    snprintf (postData, 100, "{\"level\": %d, \"plant_id\": \"%s\"}", meter, plantId);
    Serial.print("sending: ");
    Serial.println(postData); 
    http.begin(https, serverName);              //Specify request destination
    http.addHeader("Content-Type", "application/json");    //Specify content-type header
  
    int resHttpCode = http.POST(postData);   //Send the request
    String resData = http.getString();    //Get the response payload
  
    Serial.print("Response Status: ");
    Serial.println(resHttpCode);   //Print HTTP return code
    Serial.print("Response data: ");
    Serial.println(resData);    //Print request response payload
  
    http.end();  //Close connection
  }
}

void setup() {
  Serial.begin(115200);
  setup_wifi();
}

void loop() {
  post_sensor();
}
