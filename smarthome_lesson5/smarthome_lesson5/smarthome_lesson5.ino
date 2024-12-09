/*
 OSOYOO Smarthome example: remote control RGB
 
 A simple web server that lets you control the RGB color via a web page.
 This sketch will print the IP address of your ESP8266 module (once connected)
 to the Serial monitor. From there, you can open that address in a web browser
 to control the RGB on the RGB interface.

 For more details see: http://osoyoo.com/?p=28918
*/

#include "WiFiEsp.h"

// Emulate softserial on pins A9/A8 if not present
//#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial softserial(A9, A8); // RX, TX
//#endif

char ssid[] = "wifihome";            // your network SSID (name)
char pass[] = "sinissy1";        // your network password
int status = WL_IDLE_STATUS;

const int redPin = 24; // R connected to digital pin 24 
const int greenPin = 23; // G to digital pin 23
const int bluePin = 22; // B connected to digital pin 22 

WiFiEspServer server(80);

// use a ring buffer to increase speed and reduce memory allocation
RingBuffer buf(8);

void setup()
{
  pinMode(redPin, OUTPUT); // set the redPin to be an output 
  pinMode(greenPin, OUTPUT); // set the greenPin to be an output 
  pinMode(bluePin, OUTPUT); // set the bluePin to be an output 
//  pinMode(LED_BUILTIN, OUTPUT);	// initialize digital pin LED_BUILTIN as an output.
  Serial.begin(9600);   // initialize serial for debugging

  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("start");
  // set the data rate for the SoftwareSerial port
  softserial.begin(115200);
  softserial.println("AT+UART_DEF=9600,8,1,0,0\r\n");
  softserial.write("AT+RST\r\n");
  delay(100);
  softserial.begin(9600);
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


///////////////////////////////// Check to see if the client request was "GET /R" or "GET /G"or "GET /B":
        else if (buf.endsWith("GET /R")) {
          Serial.println("Turn the RGB LED to red");
          color(255, 0, 0);//Turn the RGB LED to red
  
        }
        else if (buf.endsWith("GET /G")) {
          Serial.println("Turn the RGB LED to green");
          color(0,255, 0); //Turn the RGB LED to blue
          
      }
        else if (buf.endsWith("GET /B")) {
          Serial.println("Turn the RGB LED to blue");

          color(0, 0, 255);//Turn the RGB LED to green
          }
      }
    }
    
    // close the connection
    client.stop();
    Serial.println("Client disconnected");
  }
}

void color (unsigned char red, unsigned char green, unsigned char blue) // the color generating function 
{ 
analogWrite(redPin, red); 
analogWrite(greenPin, green); 
analogWrite(bluePin, blue); 
}
void sendHttpResponse(WiFiEspClient client)
{
  // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
  // and a content-type so the client knows what's coming, then a blank line:
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println();
  
  // The HTTP response ends with another blank line:
  client.println();
    // the content of the HTTP response follows the header:
  client.println("<br>");
  client.println("<br>");
  
  client.println("Click <a href=\"/R\">here</a> turn the RGB to Red<br>");
  client.println("Click <a href=\"/G\">here</a> turn the RGB to Green<br>");
  client.println("Click <a href=\"/B\">here</a> turn the RGB to Blue<br>");
  
  // The HTTP response ends with another blank line:
  client.println();
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
