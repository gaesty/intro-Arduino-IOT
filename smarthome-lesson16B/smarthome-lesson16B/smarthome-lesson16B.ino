/*  ___   ___  ___  _   _  ___   ___   ____ ___  ____  
 * / _ \ /___)/ _ \| | | |/ _ \ / _ \ / ___) _ \|    \ 
 *| |_| |___ | |_| | |_| | |_| | |_| ( (__| |_| | | | |
 * \___/(___/ \___/ \__  |\___/ \___(_)____)___/|_|_|_|
 *                  (____/ 
 * Make RFIC module and MEGA-IoT shield to make a remote control security door system
 * Tutorial URL  http://osoyoo.com/?p=29001
 * CopyRight www.osoyoo.com
 */
#include <Servo.h>
Servo head;
#define SERVO_PIN 3
#define RED_LED 11
#define GREEN_LED 12
#define BUZZER 5

#include <SPI.h>
#include <RFID.h>
unsigned char my_rfid[] = {247,101,189,96,79 }; // read http://osoyoo.com/?p=28943 to get your RFID value and replace this line
RFID rfid(48,49);   //D48--RFID module SDA pin、D49 RFID module RST pin
#include "WiFiEsp.h"
#include "WiFiEspUdp.h"
WiFiEspUDP Udp;
unsigned int localPort = 8888;  // local port to listen on

//#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial softserial(A9, A8); // A9 to ESP_TX, A8 to ESP_RX by default
//#endif

char ssid[] = "wifihome";            // your network SSID (name)
char pass[] = "sinissy1";        // your network password
int status = WL_IDLE_STATUS;

int DoorStatus = LOW;

char packetBuffer[5]; 

void setup()
{ 
  pinMode(RED_LED,OUTPUT);
  pinMode(GREEN_LED,OUTPUT);
  pinMode(BUZZER,OUTPUT);
  pinMode(SERVO_PIN,OUTPUT);

  SPI.begin();
  rfid.init();
  pinMode(LED_BUILTIN, OUTPUT);	// initialize digital pin LED_BUILTIN as an output.
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
  
  Udp.begin(localPort);
  
  Serial.print(" target port ");
  Serial.println(localPort);
}


void loop()
{
   //search card
  if (rfid.isCard()) {
    Serial.println("Find the card!");
   //read serial number
    if (rfid.readCardSerial()) {
      Serial.println("id=");
        for (int i=0;i<5;i++)
        {
            Serial.print(rfid.serNum[i]);
            Serial.print(' ');
             
        }
        Serial.println();
      if (compare_rfid(rfid.serNum,my_rfid))
      {
      Serial.println("match");
       open_door();
       digitalWrite(BUZZER,LOW);
    }
      else
      { 
             Serial.println("not match");
        close_door();
       digitalWrite(BUZZER,HIGH);
       
      }
    }
    rfid.selectTag(rfid.serNum);
  }
  
  rfid.halt();
  
  int packetSize = Udp.parsePacket();
  if (packetSize) {                              
     Serial.print("Received packet of size ");
    Serial.println(packetSize);
    int len = Udp.read(packetBuffer, 255);
    if (len > 0) {
      packetBuffer[len] = 0;
    }
      char c=packetBuffer[0];
            switch (c)    //serial control instructions
            {   
  
               case 'L':  // ◄ KEY PRESSED
                Serial.println("Close Door");
                close_door();

               case 'A': // ▲ KEY PRESSED
                Serial.println("Half Close THE DOOR!");
                half_open();
                break;
                
               case 'R': //► KEY PRESSED
                Serial.println("Open door");
                open_door();
                break;
                
               case 'B': //▼ KEY PRESSED
                break;
               
               case 'E': // || KEY PRESSED
                    digitalWrite(BUZZER,LOW);
                break;
                
               case 'F': //F1 KEY PRESSED
                break;
                
               case 'G': //F2 KEY PRESSED
                break;
                             
               case 'H': //F3 KEY PRESSED
                break;

               case 'I': //F4 KEY PRESSED
                break;
                
               case 'J': //F5 KEY PRESSED
                break;
                
               case 'K': //F6 KEY PRESSED
                break;
                
               case 'O': //Obstacle button PRESSED
                break;
                
                case 'T': //Tracking button PRESSED
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
boolean compare_rfid(unsigned char x[],unsigned char y[])
{
 
  for (int i=0;i<5;i++)
  {
 
    if(x[i]!=y[i]) return false;
  }
 
  return true;
}
void open_door()
{
    head.attach(SERVO_PIN); 
    delay(300);
    head.write(180);//servo goes to 0 degrees 
     delay(400);
    head.detach(); // save current of servo
    digitalWrite(SERVO_PIN,LOW);
    digitalWrite(GREEN_LED,HIGH);
    digitalWrite(RED_LED,LOW);
 
}
void half_open()
{
    head.attach(SERVO_PIN); 
    delay(300);
    head.write(90);//servo goes to 0 degrees 
      delay(400);
    head.detach(); // save current of servo
    digitalWrite(SERVO_PIN,LOW);
    digitalWrite(GREEN_LED,LOW);
    digitalWrite(RED_LED,LOW);
}
void close_door()
{
    head.attach(SERVO_PIN); 
    delay(300);
    head.write(0);//servo goes to 0 degrees 
     delay(400);
    head.detach(); // save current of servo
    digitalWrite(SERVO_PIN,LOW);
    digitalWrite(GREEN_LED,LOW);
    digitalWrite(RED_LED,HIGH);
 
}
