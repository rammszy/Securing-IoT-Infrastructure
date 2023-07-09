#include <SPI.h>
#include <WiFiNINA.h>
#include <AES.h>

#include <AESLib.h>

#include <AES_config.h>

#include <Base64.h>

AES aes;

byte cipher[1000];

char b64[1000];


const int trigPin = 9;
const int echoPin = 10;

long duration;
int distance;


char ssid[] = "****";        
char pass[] = "****";    
int keyIndex = 0;            

int status = WL_IDLE_STATUS;



char server[] = "192.168.145.218";    




WiFiClient client;

// msg: message need to be encrypted.

// key_str: secrete key, 16 bytes

// iv_str:  initial vector, 16 bytes

void do_encrypt(String msg, String key_str, String iv_str) {



  byte iv[16];

  // copy the iv_str content to the array.

  memcpy(iv,(byte *) iv_str.c_str(), 16);



  // use base64 encoder to encode the message content. It is optional step.

  int blen=base64_encode(b64,(char *)msg.c_str(),msg.length());



  // calculate the output size:

  aes.calc_size_n_pad(blen);

  // custom padding, in this case, we use zero padding:

  int len=aes.get_size();

  byte plain_p[len];

  for(int i=0;i<blen;++i) plain_p[i]=b64[i];

  for(int i=blen;i<len;++i) plain_p[i]='\0';



  // do AES-128-CBC encryption:

  int blocks = len / 16;

  aes.set_key ((byte *)key_str.c_str(), 16) ;

  aes.cbc_encrypt (plain_p, cipher, blocks, iv);



  // use base64 encoder to encode the encrypted data:

  base64_encode(b64,(char *)cipher,len);

  Serial.println("Encrypted Data output: "+String((char *)b64));
   if (client.connect(server, 80)) {
    Serial.println("connected to server");
    // Make a HTTP request:
    client.println("GET /iot.php?enc=" +String((char *)b64) + " HTTP/1.1");
    client.println("Host: 192.168.145.218");
    client.println("Connection: close");
    client.println();
  }

  

}





void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  
  
  
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }
  Serial.println("Connected to WiFi");
  printWifiStatus();

  Serial.println("\nStarting connection to server...");
  // if you get a connection, report back via serial:
 /* if (client.connect(server, 80)) {
    Serial.println("connected to server");
    // Make a HTTP request:
    client.println("GET /iot.php?enc=arduino HTTP/1.1");
    client.println("Host: 192.168.145.218");
    client.println("Connection: close");
    client.println();
  }*/
}

void loop() {
  // if there are incoming bytes available
  // from the server, read them and print them:
   // Measure distance
  

  
  while (client.available()) {
    char c = client.read();
    Serial.write(c);
    
    
  }
  
  // put your main code here, to run repeatedly:
  // Clear the trigger pin
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  // Set the trigger pin high for 10 microseconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  // Measure the duration of the echo pulse
  duration = pulseIn(echoPin, HIGH);
 distance= duration * 0.034 / 2;

  


  String msg=String(distance);

  String key_str="aaaaaaaaaaaaaaaa";// 16 bytes

  String iv_str="aaaaaaaaaaaaaaaa"; //16 bytes

  do_encrypt(msg,key_str,iv_str);


  delay(5000);

  // if the server's disconnected, stop the client:
  if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting from server.");
    client.stop();

    // do nothing forevermore:
    while (true);
  }
}


void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}