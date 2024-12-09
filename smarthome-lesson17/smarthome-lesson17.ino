/*  ___   ___  ___  _   _  ___   ___   ____ ___  ____  
 * / _ \ /___)/ _ \| | | |/ _ \ / _ \ / ___) _ \|    \ 
 *| |_| |___ | |_| | |_| | |_| | |_| ( (__| |_| | | | |
 * \___/(___/ \___/ \__  |\___/ \___(_)____)___/|_|_|_|
 *                  (____/ 
 * Use browser and OSOYOO MEGA-IoT  shield and Ultrasonic Sensor to trace human movement
 * Tutorial URL http://osoyoo.com/?p=29072
 * CopyRight www.osoyoo.com
 */

#include "WiFiEsp.h"
//#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial softserial(A9, A8); // A9 to ESP_TX, A8 to ESP_RX by default
//#endif
#define whiteLED 9
#define yellowLED 10
#define redLED 11
#define greenLED 12
#define Trig_PIN 25
#define Echo_PIN 26

char ssid[] = "wifihome";            // your network SSID (name)
char pass[] = "sinissy1";        // your network password
int status = WL_IDLE_STATUS;

int distance = 0;
String status_str;
WiFiEspServer server(80);
// use a ring buffer to increase speed and reduce memory allocation
RingBuffer buf(8);

void setup()
{  
  pinMode(redLED, OUTPUT);	// initialize digital pin Red LED as an output.
  pinMode(greenLED, OUTPUT);  // initialize digital pin Red LED as an output.
  pinMode(redLED, OUTPUT);	// initialize digital pin Red LED as an output.
  pinMode(greenLED, OUTPUT);  // initialize digital pin Red LED as an output.
  pinMode(Trig_PIN, OUTPUT);  // initialize digital pin Trig Pin as an output.
  pinMode(Echo_PIN, INPUT);  // initialize digital pin Echo Pin as an input.
 
   
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
 

  distance=watch();
  if (distance>40) {
    digitalWrite(whiteLED,LOW);  
    digitalWrite(yellowLED,LOW); 
    digitalWrite(redLED,LOW); 	
    digitalWrite(greenLED,LOW);    
    Serial.println("No object in range");
    status_str="<font color=000000><b>No object in range</b></font>";
  } else  if (distance>20)
  {
//    Serial.println("Intruder detected");
    digitalWrite(whiteLED,LOW);  
    digitalWrite(yellowLED,LOW); 
    digitalWrite(redLED,HIGH); 	
    digitalWrite(greenLED,LOW);   
    status_str="<font color=red><b>Object is Very Far</b></font>";
  }
  else  if (distance>10)
  {
  //  Serial.println("Object is far!");
    digitalWrite(whiteLED,LOW);  
    digitalWrite(yellowLED,LOW); 
    digitalWrite(redLED,LOW);   
    digitalWrite(greenLED,HIGH);   
    status_str="<font color=green><b>Object is far</b></font>";
  }   else  if (distance>5)
  {
  //  Serial.println("Object is close!");
    digitalWrite(whiteLED,LOW);  
    digitalWrite(yellowLED,HIGH); 
    digitalWrite(redLED,LOW);   
    digitalWrite(greenLED,LOW);   
    status_str="<font color=yellow><b>Object is  close</b></font>";
  } else 
   {
//    Serial.println("Object is very close!");
    digitalWrite(whiteLED,HIGH);  
    digitalWrite(yellowLED,LOW); 
    digitalWrite(redLED,LOW);   
    digitalWrite(greenLED,LOW);   
    status_str="<font color=ffffff><b>Object is Very Close!</b></font>";
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
  client.print("<meta http-equiv=\"refresh\" content=\"5\"/>Status: ");
    client.print(status_str);

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
int watch(){
  long echo_distance;
  digitalWrite(Trig_PIN,LOW);
  delayMicroseconds(5);                                                                              
  digitalWrite(Trig_PIN,HIGH);
  delayMicroseconds(15);
  digitalWrite(Trig_PIN,LOW);
  echo_distance=pulseIn(Echo_PIN,HIGH);
  echo_distance=echo_distance*0.01657; //how far away is the object in cm
  //Serial.println((int)echo_distance);
  return round(echo_distance);
}
