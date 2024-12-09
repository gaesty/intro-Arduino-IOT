/*
 OSOYOO Smarthome example: Remote Control Servo
 
 A simple web server that lets you control a servor via a web page.
 This sketch will print the IP address of your ESP8266 module (once connected)
 to the Serial monitor. From there, you can open that address in a web browser
 to control the servo on pin D9, when the servo turn to 180 degree,the green led
 on D4 will be lit.

 For more details see: http://osoyoo.com/?p=28938
*/
#include "WiFiEsp.h"
#include "WiFiEspUdp.h"
WiFiEspUDP Udp;
unsigned int localPort = 8888;  // local port to listen on
#include "Keypad.h"
char packetBuffer[5];  
const byte ROWS = 4; //four rows
const byte COLS = 4; //four columns
//define the cymbols on the buttons of the keypads

char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {33, 35, 37,39}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {41, 43, 45, 47}; //connect to the column pinouts of the keypad
 
//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 


#include <Servo.h>
Servo myservo;//create servo object to control a servo
#define greenled 12 //connect greenled to digital pin4
// Emulate softserial on pins A9/A8 if not present
//#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial softserial(A9, A8); // RX, TX
//#endif

char ssid[] = "wifihome";            // your network SSID (name)
char pass[] = "sinissy1";        // your network password
int status = WL_IDLE_STATUS;

int ledStatus = LOW;

WiFiEspServer server(80);

// use a ring buffer to increase speed and reduce memory allocation
RingBuffer buf(8);

void setup()
{
 // myservo.attach(3);//attachs the servo on pin D3 to servo object
//  myservo.write(0);//back to 0 degrees 
 //delay(15);//wait for a second
  pinMode(greenled, OUTPUT);	// initialize digital pin greenled as an output.
   pinMode(3, OUTPUT);
  Serial.begin(9600);   // initialize serial for debugging
//  myservo.attach(3);
  //   myservo.write(0);//servo goes to 90 degrees
 digitalWrite(3,LOW);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("start");
  // set the data rate for the SoftwareSerial port
  softserial.begin(115200);
  softserial.println("AT+UART_DEF=9600,8,1,0,0");
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
  Udp.begin(localPort);
  
  Serial.print(" target port ");
  Serial.println(localPort);
}


void loop()
{
  
  char customKey = customKeypad.getKey();
  
  if (customKey=='*'){
    Serial.println(" Close THE DOOR!");
    digitalWrite(greenled, LOW);   // turn the led OFF 
    myservo.attach(3);
    delay(300);
    myservo.write(0);//servo goes to 0 degrees 
    delay(400);
    myservo.detach();  // save current of servo
    digitalWrite(3,LOW);
  }
  if (customKey=='#'){
    Serial.println("Open THE DOOR!");
    digitalWrite(greenled, HIGH);   // turn the led on 
    myservo.attach(3);
    delay(300);
    myservo.write(180);//servo goes to 180 degrees 
    delay(400);
    myservo.detach();  // save current of servo
    digitalWrite(3,LOW);
  }
   if (customKey=='0'){
    Serial.println("Half Close THE DOOR!");
    digitalWrite(greenled, LOW);   // turn the led off 
    myservo.attach(3);
    delay(300);
    myservo.write(90);//servo goes to 90 degrees 
    delay(400);
    myservo.detach();  // save current of servo
    digitalWrite(3,LOW);
  }
   int packetSize = Udp.parsePacket();
  if (packetSize) {                               // if you get a client,
     Serial.print("Received packet of size ");
    Serial.println(packetSize);
    int len = Udp.read(packetBuffer, 255);
    if (len > 0) {
      packetBuffer[len] = 0;
    }
      char c=packetBuffer[0];
            switch (c)    //serial control instructions
            {   
  
               case 'L':
                Serial.println("Close THE DOOR!");
            
                digitalWrite(greenled, LOW);   // turn the led off 
                myservo.attach(3);
                delay(300);
                myservo.write(0);//servo goes to 180 degrees 
                delay(400);
                myservo.detach(); // save current of servo
                digitalWrite(3,LOW);
                break;

               case 'A':
                Serial.println("Half Close THE DOOR!");
                digitalWrite(greenled, LOW);   // turn the led off 
                myservo.attach(3);
                delay(300);
                myservo.write(90);//servo goes to 90 degrees 
                delay(400);
                myservo.detach();  // save current of servo
                digitalWrite(3,LOW);
                break;
               case 'R':
                Serial.println("Open THE DOOR!");
                ledStatus = HIGH;
                digitalWrite(greenled, HIGH);   // turn the led on  
                myservo.attach(3);  
                delay(300);
                myservo.write(180);//servo goes to 180 degrees 
                delay(400);
                myservo.detach(); // save current of servo
                digitalWrite(3,LOW);
                break;
         
              } //END OF ACTION SWITCH
        }//end of packetsize test
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
  Serial.print("please set your UDP APP target IP to: ");
  Serial.println(ip);
  Serial.println();
}
