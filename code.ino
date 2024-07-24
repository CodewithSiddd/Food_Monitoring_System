#include <DHT.h> 
#include <ESP8266WiFi.h> 
#include <WiFiClient.h>; 
#include <ThingSpeak.h>; 
#define RL 10.0 // the value of RL is 10K 
#define m -0.36848 // using formula y = mx+c and the graph in the datasheet 
#define c 1.105 // btained by before calculation 
#define R0 1.9 
 
// replace with your channel’s thingspeak API key, 
const char * myWriteAPIKey = "13ASJ73VXFVPF7ZJs"; 
unsigned long myChannelNumber = 1667361; //Replace it with your channel ID 
const char* ssid = "iBall-Baton"; 
const char* password = "pratik@1234"; 
const char* server = "api.thingspeak.com"; 
const char *host = "maker.ifttt.com"; 
const char *privateKey = "i0VOW0-bvRCAYW2VJfQm6E1wWru48lxSb6ld4x4qOgF"; 
double gas; 
 
//Motor PINs l298n 
#define ENA 16 //D0 
#define IN1 5  //D1 
#define IN2 4  //D2 
#define Gas_Pin A0 
#define DHTPIN 0 // CONNECT THE DHT11 SENSOR TO PIN D4 OF THE NODEMCU 
DHT dht(DHTPIN, DHT11); //CHANGE DHT11 TO DHT22 IF YOU ARE USING DHT22 
WiFiClient client; 
 
void stoped() 
{ 
  //Apply speed zero for stopping motors 
   
//  analogWrite(ENA, 0); 
// analogWrite(ENB, 0); 
  digitalWrite(IN1, LOW); 
  digitalWrite(IN2, LOW); 
  Serial.println("Stop"); 
} 
 
void forward() 
{ 
// analogWrite(ENA, 255); 
//  analogWrite(ENB, 255); 
  digitalWrite(IN1, HIGH); 
  digitalWrite(IN2, LOW); 
  Serial.println("forward"); 
} 
 
void setup() { 
Serial.begin(9600); 
//pinMode(fan,OUTPUT); 
delay(100); 
dht.begin(); 
ThingSpeak.begin(client); 
WiFi.begin(ssid, password); 
  pinMode(ENA, OUTPUT); 
  pinMode(IN1, OUTPUT); 
  pinMode(IN2, OUTPUT); 
Serial.println();  
Serial.println(); 
Serial.print("Connecting to "); 
Serial.println(ssid); 
while (WiFi.status() != WL_CONNECTED) { 
delay(2000); 
Serial.print("."); 
} 
Serial.println(""); 
Serial.println("WiFi connected"); 
} 
 
int ppm1() {   
  float sensor_volt; //Define variable for sensor voltage  
  float RS_gas; //Define variable for sensor resistance   
  float ratio; //Define variable for ratio 
  float sensorValue = analogRead(Gas_Pin); //Read analog values of sensor   
  sensor_volt = sensorValue*(5.0/1023.0); //Convert analog values to voltage  
  RS_gas = ((5.0*10.0)/sensor_volt)-10.0; //Get value of RS in a gas 
  ratio = RS_gas/R0;  // Get ratio RS_gas/RS_air 
  double ppm_log = (log10(ratio)-c)/m; //Get ppm value in linear scale according to the the ratio 
value   
  float ppm = pow(10, ppm_log); //Convert ppm value to log scale  
  Serial.println(ppm); 
  ThingSpeak.writeField(myChannelNumber, 3,ppm, myWriteAPIKey); 
  delay(2000); 
  return(ppm);   
} 
 
void loop() { 
 float t = dht.readTemperature(); 
 delay(2000); 
ThingSpeak.writeField(myChannelNumber, 1,t, myWriteAPIKey); 
Serial.print("Temp = "); 
Serial.println(t); 
float h = dht.readHumidity(); 
delay(2000); 
ThingSpeak.writeField(myChannelNumber, 2,h, myWriteAPIKey); 
Serial.print("Humidity = "); 
Serial.println(h); 
gas = ppm1(); 
delay(2000); 
if (t > 35){ 
  if(h > 54){ 
    forward(); 
    send_event("temp_event"); 
    Serial.println("Fan On"); 
    } 
} 
else{ 
  stoped(); 
} 
   if (isnan(h) || isnan(t)|| isnan(gas)) { 
   Serial.println("Failed to read from DHT sensor!"); 
    } 
} 
 
void send_event(const char *event) 
{ 
  Serial.print("Connecting to "); 
  Serial.println(host);   
  // Use WiFiClient class to create TCP connections 
  WiFiClient client; 
  const int httpPort = 80; 
  if (!client.connect(host, httpPort)) { 
    Serial.println("Connection failed"); 
    return; 
  }   
  // We now create a URI for the request 
  String url = "/trigger/"; 
  url += event; 
  url += "/with/key/"; 
  url += privateKey;   
  Serial.print("Requesting URL: "); 
  Serial.println(url); 
  // This will send the request to the server 
  client.print(String("GET ") + url + " HTTP/1.1\r\n" + 
               "Host: " + host + "\r\n" +  
               "Connection: close\r\n\r\n"); 
  while(client.connected()) 
  { 
    if(client.available()) 
    { 
      String line = client.readStringUntil('\r'); 
      Serial.print(line); 
    } else { 
      // No data yet, wait a bit 
      delay(500); 
    }; 
  Serial.println(); 
  Serial.println("closing connection"); 
  client.stop(); 
} 
