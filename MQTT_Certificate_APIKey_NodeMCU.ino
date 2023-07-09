#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <time.h>
#include <TZ.h>
#include <FS.h>
#include <LittleFS.h>
#include <CertStoreBearSSL.h>
#include <ThingSpeak.h>


const char* ssid = "****";
const char* password = "****";
const char* mqtt_server = "****.s1.eu.hivemq.cloud";

//Declaration of Pins for Temperature Sensor
const int DHT_PIN = D2;  
const int DHT_TYPE = DHT11;

// A single, global CertStore which can be used by all connections.
// Needs to stay live the entire time any of the WiFiClientBearSSLs
// are present.
BearSSL::CertStore certStore;

WiFiClientSecure espClient;
PubSubClient * client;
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (500)
char msg[MSG_BUFFER_SIZE];
int value = 0;

float temperature = 0.0;
DHT dht(DHT_PIN, DHT_TYPE);

const int raindropPin = A0;  // Analog input pin for raindrop sensor

WiFiClient wifiClient;

const char* server = "api.thingspeak.com";
const char* apiKey = "****";
const unsigned long channelID = ****;  
void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
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
  dht.begin();

}


void setDateTime() {
  // Only the date is needed for validating the certificates.
  configTime(TZ_Europe_Berlin, "pool.ntp.org", "time.nist.gov");

  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(100);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println();

  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.printf("%s %s", tzname[0], asctime(&timeinfo));
}


void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if the first character is present
  if ((char)payload[0] != NULL) {
    digitalWrite(LED_BUILTIN, LOW); // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is active low on the ESP-01)
    delay(500);
    digitalWrite(LED_BUILTIN, HIGH); // Turn the LED off by making the voltage HIGH
  } else {
    digitalWrite(LED_BUILTIN, HIGH); // Turn the LED off by making the voltage HIGH
  }
}


void reconnect() {
  // Loop until we’re reconnected
  while (!client->connected()) {
    Serial.print("Attempting MQTT connection…");
    String clientId = "ESP8266Client - MyClient";
    // Attempt to connect
    // Insert your password
    if (client->connect(clientId.c_str(), "****", "****")) {
      Serial.println("connected");
      // Once connected, publish an announcement…
      client->publish("testTopic", "hello world");
      // … and resubscribe
      client->subscribe("testTopic");
    } else {
      Serial.print("failed, rc = ");
      Serial.print(client->state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void setup() {

  //delay(500);
  // When opening the Serial Monitor, select 9600 Baud
  Serial.begin(9600);
 // delay(500);

  LittleFS.begin();
  setup_wifi();
  setDateTime();

  pinMode(LED_BUILTIN, OUTPUT); // Initialize the LED_BUILTIN pin as an output

  // you can use the insecure mode, when you want to avoid the certificates
  //espclient->setInsecure();

  int numCerts = certStore.initCertStore(LittleFS, PSTR("/certs.idx"), PSTR("/certs.ar"));
  Serial.printf("Number of CA certs read: %d\n", numCerts);
  if (numCerts == 0) {
    Serial.printf("No certs found. Did you run certs-from-mozilla.py and upload the LittleFS directory before running?\n");
    return; // Can't connect to anything w/o certs!
  }

  BearSSL::WiFiClientSecure *bear = new BearSSL::WiFiClientSecure();
  // Integrate the cert store with this connection
  bear->setCertStore(&certStore);

  client = new PubSubClient(*bear);

  client->setServer(mqtt_server, 8883);
  client->setCallback(callback);
    dht.begin();

  ThingSpeak.begin(wifiClient);
  delay(20000);


}

void loop() {
  if (!client->connected()) {
    reconnect();
  }
  client->loop();

  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    float humidity = dht.readHumidity();

    float temperature = dht.readTemperature();

    if (isnan(humidity) || isnan(temperature)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }
   
    snprintf(msg, MSG_BUFFER_SIZE, "Temp: %.2f°C, Humidity: %.2f%%", temperature, humidity);

    Serial.print("Publish message: ");
    Serial.println(msg);
    client->publish("temp", msg);
    
    int raindropValue = analogRead(raindropPin);
    Serial.print("Raindrop Value: ");
    Serial.println(raindropValue);

    ThingSpeak.setField(1, raindropValue);
    int response = ThingSpeak.writeFields(channelID, apiKey);

     Serial.print("response: ");
    Serial.println(response);
    if (response == 200) {
      Serial.println("Data uploaded to ThingSpeak");
    } else {
      Serial.println("Error uploading data to ThingSpeak");
    }
  }
    delay(20000);

}