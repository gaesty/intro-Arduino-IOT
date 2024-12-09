/*  ___   ___  ___  _   _  ___   ___   ____ ___  ____  
 * / _ \ /___)/ _ \| | | |/ _ \ / _ \ / ___) _ \|    \ 
 *| |_| |___ | |_| | |_| | |_| | |_| ( (__| |_| | | | |
 * \___/(___/ \___/ \__  |\___/ \___(_)____)___/|_|_|_|
 *                  (____/ 
 * Use browser and OSOYOO MEGA-IoT extension shield to detect remote noise
 * Tutorial URL http://osoyoo.com/?p=28843
 * CopyRight www.osoyoo.com
 */

#include "WiFiEsp.h"
//#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial softserial(A9, A8); // A9 to ESP_TX, A8 to ESP_RX by default
//#endif
#define redLED 11
#define greenLED 12
#define sound_sensor A2
#define buzzer 5
int SoundStatus=0;
String StatusStr;
char ssid[] = "wifihome";            // your network SSID (name)
char pass[] = "sinissy1";        // your network password
int status = WL_IDLE_STATUS;


WiFiEspServer server(80);
// use a ring buffer to increase speed and reduce memory allocation
RingBuffer buf(8);

void setup()
{ 
  pinMode(redLED, OUTPUT);	// initialize digital pin Red LED as an output.
  pinMode(greenLED, OUTPUT);  // initialize digital pin Red LED as an output.
    pinMode(buzzer, OUTPUT);  // initialize digital pin buzzer as an output.
	    digitalWrite(buzzer,HIGH);  //make buzzer alarm to give test sound source
  pinMode(sound_sensor, INPUT);  // initialize gas sensor pin input.
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
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network
    status = WiFi.begin(ssid, pass);
  }

  Serial.println("You're connected to the network");
  printWifiStatus();
  
  // start the web server on port 80
  server.begin();
}


void loop()
{


  SoundStatus=digitalRead(sound_sensor);

  if (SoundStatus==1) {
    digitalWrite(redLED,LOW);
    digitalWrite(greenLED,HIGH);   
    Serial.println("no noise");
    StatusStr="<font color=GREEN><b>No Noise!</b></font>";
  } else
  {
 Serial.println("sound detected");
    digitalWrite(redLED,HIGH);
    digitalWrite(greenLED,LOW);   
        StatusStr="<font color=RED><b>Noise Detected!</b></font>";
  }
  WiFiEspClient client = server.available();  // listen for incoming clients

  if (client) {                               // if you get a client,
    Serial.println("New client");             // print a message out the serial port
    buf.init();                               // initialize the circular buffer
    while (client.connected()) {              // loop while the client's connected
      if (client.available()) {               // if there's bytes to read from the client,
        char c = client.read();               // read a byte, then
        buf.push(c);                          // push it to the ring buffer

        // printing the stream to the serial monitor will slow down
        // the receiving of data from the ESP filling the serial buffer
        //Serial.write(c);
        
        // you got two newline characters in a row
        // that's the end of the HTTP request, so send a response
        if (buf.endsWith("\r\n\r\n")) {
          sendHttpResponse(client);
     
          break;
        }

    
      }
    }
    
    // close the connection
    client.stop();
    Serial.println("Client disconnected");

  }
}


void sendHttpResponse(WiFiEspClient client)
{
  // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
  // and a content-type so the client knows what's coming, then a blank line:
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println();
  
  // the content of the HTTP response follows the header:
  client.print("<meta http-equiv=\"refresh\" content=\"5\"/>Sound Sensor Status: ");
    client.print(StatusStr);

}

void printWifiStatus()
{
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print where to go in the browser
  Serial.println();
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
  Serial.println();
}
