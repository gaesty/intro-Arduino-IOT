/*  ___   ___  ___  _   _  ___   ___   ____ ___  ____  
 * / _ \ /___)/ _ \| | | |/ _ \ / _ \ / ___) _ \|    \ 
 *| |_| |___ | |_| | |_| | |_| | |_| ( (__| |_| | | | |
 * \___/(___/ \___/ \__  |\___/ \___(_)____)___/|_|_|_|
 *                  (____/ 
 * Osoyoo Wifi Arduino Shield Testing Project
 * Exchange UDP  data with two Arduino device through two Osoyoo Mega-IoT boards
 * tutorial url: http://osoyoo.com/?p=29139
 */

#include "WiFiEsp.h"
#include "WiFiEspUdp.h"
#include "ArduinoJson.h"
#include "SoftwareSerial.h"
#include "LiquidCrystal_I2C.h"

LiquidCrystal_I2C lcd(0x3f,16,2);  // set the LCD address from 0x27 to 0x3f if i2c address is 0x3f
SoftwareSerial softserial(A9, A8); // A9 to ESP_TX, A8 to ESP_RX by default
  StaticJsonDocument<200> doc;
#define LED_PIN 11
#define BUZZER 5 
char ssid[] = "wifihome";            // your network SSID (name)
char pass[] = "sinissy1";        // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status

char remote_server[] = "192.168.133.162";  // remote device IP
unsigned int local_port = 2390;        // local port to listen for UDP packets
unsigned int remote_port = 2390;        // remote port to listen for UDP packets

const int UDP_PACKET_SIZE = 255;  // UDP timestamp is in the first 48 bytes of the message
const int UDP_TIMEOUT =500;    // timeout in miliseconds to wait for an UDP packet to arrive

char packetBuffer[255];

// A UDP instance to let us send and receive packets over UDP
WiFiEspUDP Udp;

void setup()
{
  pinMode(BUZZER,OUTPUT);
  pinMode(LED_PIN,OUTPUT);
  lcd.init();
  lcd.backlight();

  Serial.begin(9600);   // initialize serial for debugging
  softserial.begin(115200);
  softserial.write("AT+CIOBAUD=9600\r\n");
  softserial.write("AT+RST\r\n");
  softserial.begin(9600);    // initialize serial for ESP module
  WiFi.init(&softserial);    // initialize ESP module

  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }

  // attempt to connect to WiFi network
  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }

  // you're connected now, so print out the data
  Serial.println("You're connected to the network");
   printWifiStatus();

  Serial.println("\nStarting connection to server...");
  // if you get a connection, report back via serial:
  Udp.begin(local_port);
  
  Serial.print("Listening on port ");
  Serial.println(local_port);

  
}

void loop()
{
  //sendUDPpacket(remote_server); // send an UDP packet to a time server
 //    Udp.beginPacket(remote_server,remote_port);
  //  Udp.write("What's your name?"); 
  // wait for a reply for UDP_TIMEOUT miliseconds
unsigned long startMs = millis();
 while (!Udp.available() && (millis() - startMs) < UDP_TIMEOUT) {}

  DeserializationError error = deserializeJson(doc, Udp);

  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }
   
  double temp = doc["T"];
  double humid = doc["H"];
  int led_status=doc["L"];
  int gas_status=doc["G"];
  if (led_status==0) digitalWrite(LED_PIN,HIGH);
   else digitalWrite(LED_PIN,LOW);
 if (gas_status==0) digitalWrite(BUZZER,HIGH);
   else digitalWrite(BUZZER,LOW);
   
    char*  cardID;
  cardID=  doc["R"];
   Serial.print("RFID:");
 Serial.println(cardID);
  lcd.clear();
   lcd.backlight();
   lcd.setCursor(0,0);
    
    lcd.print("T:");
    lcd.print(temp);
    lcd.print(" H:");
    lcd.print(humid);
    lcd.print('%');
    lcd.setCursor(0,1);
    lcd.print("RFID ");
    if (strlen(cardID)>0);
    lcd.print(cardID);

  // wait ten seconds before asking for the time again
 delay(500);
}
void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
 
