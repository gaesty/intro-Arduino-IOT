/*  ___   ___  ___  _   _  ___   ___   ____ ___  ____  
 * / _ \ /___)/ _ \| | | |/ _ \ / _ \ / ___) _ \|    \ 
 *| |_| |___ | |_| | |_| | |_| | |_| ( (__| |_| | | | |
 * \___/(___/ \___/ \__  |\___/ \___(_)____)___/|_|_|_|
 *                  (____/ 
 * Osoyoo Wifi Arduino Shield Testing Project
 * Exchange UDP  data with two Arduino device through two Osoyoo Mega-IoT boards
 * tutorial url: http://osoyoo.com/?p=29139
 */
 
#include <SPI.h>
#include "WiFiEsp.h"
#include "WiFiEspUdp.h"
#include "ArduinoJson.h"
#include "SoftwareSerial.h"
#include <RFID.h>
RFID rfid(48,49);
#include "dht.h"
dht DHT;
#define DHT11_PIN 2
#define Button 7

#define Gas_Sensor A3
int  led_status=0;

SoftwareSerial softserial(A9, A8); // A9 to ESP_TX, A8 to ESP_RX by default

char ssid[] = "wifihome";            // your network SSID (name)
char pass[] = "sinissy1";        // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status

char remote_server[] = "192.168.133.150";  // remote device IP of Device B
unsigned int local_port = 2390;        // local port to listen for UDP packets
unsigned int remote_port = 2390;        // remote port to listen for UDP packets

const int UDP_PACKET_SIZE = 255;  // UDP timestamp is in the first 48 bytes of the message
const int UDP_TIMEOUT =500;    // timeout in miliseconds to wait for an UDP packet to arrive

char packetBuffer[255];
  String RFID_STR="";
// A UDP instance to let us send and receive packets over UDP
WiFiEspUDP Udp;
String RFID_Str="";
void setup()
{
   pinMode(Button,INPUT);
   pinMode(Gas_Sensor,INPUT);
   
    SPI.begin();
  rfid.init();
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

  Serial.println("You're connected to the network");
   printWifiStatus();

  Serial.println("\nStarting connection to server...");
  // if you get a connection, report back via serial:
  Udp.begin(local_port);
  
  Serial.print("Listening on port ");
  Serial.println(local_port);
}

void loop()
{ int gas_status=digitalRead(Gas_Sensor);
  led_status=digitalRead(Button);
 
  if (rfid.isCard()) {
    Serial.println("Find the card!");
   //read serial number
    if (rfid.readCardSerial()) {
      RFID_STR="";
      RFID_STR +=String(rfid.serNum[0],HEX);
      RFID_STR +=String(rfid.serNum[1],HEX);
      RFID_STR +=String(rfid.serNum[2],HEX);
      RFID_STR +=String(rfid.serNum[3],HEX);
      RFID_STR +=String(rfid.serNum[4],HEX);
      Serial.print("RFID:");
      Serial.println(RFID_STR);
    }
    rfid.selectTag(rfid.serNum);
  }
  
  rfid.halt();
  
  StaticJsonDocument<255> doc;
 
int chk = DHT.read11(DHT11_PIN);
    doc["T"] = DHT.temperature;
    doc["H"] = DHT.humidity;
    doc["R"] = RFID_STR;
    doc["L"] = led_status;
    doc["G"] = gas_status;

    
    Udp.beginPacket(remote_server,remote_port);
 serializeJson(doc,Udp);
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
 boolean compare_rfid(unsigned char x[],unsigned char y[])
{
  for (int i=0;i<5;i++)
  {
    if(x[i]!=y[i]) return false;
  }
  return true;
}
