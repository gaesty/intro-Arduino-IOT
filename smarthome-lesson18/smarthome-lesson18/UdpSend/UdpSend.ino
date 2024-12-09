/*  ___   ___  ___  _   _  ___   ___   ____ ___  ____  
 * / _ \ /___)/ _ \| | | |/ _ \ / _ \ / ___) _ \|    \ 
 *| |_| |___ | |_| | |_| | |_| | |_| ( (__| |_| | | | |
 * \___/(___/ \___/ \__  |\___/ \___(_)____)___/|_|_|_|
 *                  (____/ 
 * Osoyoo Wifi Arduino Shield Testing Project
 * send UDP message to remote Arduino Device
 * tutorial url: http://osoyoo.com/?p=29020
 */
#include "WiFiEsp.h"
#include "WiFiEspUdp.h"

#include "SoftwareSerial.h"
SoftwareSerial softserial(A9, A8); // A9 to ESP_TX, A8 to ESP_RX by default

char ssid[] = "wifihome";            // your network SSID (name)
char pass[] = "sinissy1";        // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status

char remote_server[] = "192.168.133.150";  // remote device IP
unsigned int local_port = 2390;        // local port to listen for UDP packets
unsigned int remote_port = 2390;        // remote port to listen for UDP packets

const int UDP_PACKET_SIZE = 255;  // UDP timestamp is in the first 48 bytes of the message
const int UDP_TIMEOUT = 3000;    // timeout in miliseconds to wait for an UDP packet to arrive

char packetBuffer[255];

// A UDP instance to let us send and receive packets over UDP
WiFiEspUDP Udp;

void setup()
{
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
  
  Udp.begin(local_port);
}

void loop()
{
  //sendUDPpacket(remote_server); // send an UDP packet to a time server
     Udp.beginPacket(remote_server,remote_port);
    Udp.write("What's your name?"); 
  // wait for a reply for UDP_TIMEOUT miliseconds
  unsigned long startMs = millis();
  while (!Udp.available() && (millis() - startMs) < UDP_TIMEOUT) {}

  Serial.println(Udp.parsePacket());
  if (Udp.parsePacket()) {
    Serial.println("reply received");
    // We've received a packet, read the data from it into the buffer
    int len = Udp.read(packetBuffer, 255);
    if (len > 0) {
      packetBuffer[len] = 0;
    }
    Serial.println(packetBuffer);

  }
  // wait ten seconds before asking for the time again
  delay(10000);
}

 
