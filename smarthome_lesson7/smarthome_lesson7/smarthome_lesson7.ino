/*
 OSOYOO smarthome example: remote accsse dht11 data 

 A simple web server that shows the data of the DHT11 sensor via a web page using an ESP8266 module.
 This sketch will print the IP address of your ESP8266 module (once connected)
 to the Serial monitor. From there, you can open that address in a web browser
 to display the web page.
 The web page will be automatically refreshed each 20 seconds.

 For more details see: http://osoyoo.com/?p=28934
*/

#include "WiFiEsp.h"
#include "dht.h"
dht DHT;
// if you require to change the pin number, Edit the pin with your arduino pin.
#define DHT11_PIN 2
// Emulate softserial on pins A9/A8 if not present
//#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial softserial(A9, A8); // RX, TX
//#endif

char ssid[] = "wifihome";            // your network SSID (name)
char pass[] = "sinissy1";        // your network password
int status = WL_IDLE_STATUS;
int reqCount = 0;                // number of requests received

WiFiEspServer server(80);


void setup()
{
  // initialize serial for debugging
  Serial.begin(9600);

  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("start");
  // set the data rate for the SoftwareSerial port
  softserial.begin(9600);
  softserial.println("AT+UART_DEF=9600,8,1,0,0\r\n");
  softserial.write("AT+RST\r\n");
  delay(100);
  softserial.begin(9600);
  // initialize ESP module
  WiFi.init(&softserial);

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
  
  // start the web server on port 80
  server.begin();
}


void loop()
{
  //get the dht11 data
  int chk = DHT.read11(DHT11_PIN);
  Serial.print(" Humidity: " );
  Serial.print(DHT.humidity, 1);
  Serial.println('%');
  Serial.print(" Temparature ");
  Serial.print(DHT.temperature, 1);
  Serial.println('C');
 // delay(2000);
  // listen for incoming clients
  WiFiEspClient client = server.available();
  if (client) {
    Serial.println("New client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          Serial.println("Sending response");
          
          // send a standard http response header
          // use \r\n instead of many println statements to speedup data send
          client.print(
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Connection: close\r\n"  // the connection will be closed after completion of the response
            "Refresh: 20\r\n"        // refresh the page automatically every 20 sec
            "\r\n");
          client.print("<!DOCTYPE HTML>\r\n");
          client.print("<html>\r\n");
          client.print("<h1>Hello OSOYOO!</h1>\r\n");
          client.print("Requests received: ");
          client.print(++reqCount);
          client.print("<br>\r\n");
          client.print("Humidity:  ");
          client.print(DHT.humidity, 1);
          client.print('%');
          client.print("<br>\r\n");
          client.print("Temperature:  ");
          client.print(DHT.temperature, 1);
          client.print('C');
          client.print("<br>\r\n");
          client.print("</html>\r\n");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(10);

    // close the connection:
    client.stop();
    Serial.println("Client disconnected");
  }
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
