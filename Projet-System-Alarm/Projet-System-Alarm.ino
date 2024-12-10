#include <SPI.h>
#include <RFID.h>
#include "WiFiEsp.h"
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>

// Configuration du keypad
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
    {'1','2','3','A'},
    {'4','5','6','B'},
    {'7','8','9','C'},
    {'*','0','#','D'}
};
byte rowPins[ROWS] = {33, 35, 37, 39}; // Lignes
byte colPins[COLS] = {41, 43, 45, 47}; // Colonnes
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Code d'activation
char code[] = "1234";
char enteredCode[5];
byte currentPosition = 0;
bool systemArmed = false;

#define RED_LED 12
#define MOTION_SENSOR 4

RFID rfid(48, 49);
LiquidCrystal_I2C lcd(0x3f, 16, 2);

// Configuration WiFi
char ssid[] = "wifihome";
char pass[] = "sinissy1";
int status = WL_IDLE_STATUS;

unsigned char authorized_rfid[] = {0xEE, 0x2B, 0x8F, 0x26};

#include "SoftwareSerial.h"
SoftwareSerial softserial(A9, A8);
WiFiEspServer server(80);

void setup() {
    pinMode(RED_LED, OUTPUT);
    pinMode(MOTION_SENSOR, INPUT);
    
    SPI.begin();
    rfid.init();
    lcd.init();
    lcd.backlight();
    
    setupWiFi();
    
    lcd.clear();
    lcd.print("System Ready");
}

void loop() {
    char key = keypad.getKey();
    
    if (key) {
        if (!systemArmed) {
            if (key >= '0' && key <= '9') {
                if (currentPosition < 4) {
                    enteredCode[currentPosition] = key;
                    currentPosition++;
                    
                    lcd.clear();
                    lcd.print("Code: ");
                    for(byte i = 0; i < currentPosition; i++) {
                        lcd.print("*");
                    }
                }
            }
            else if (key == 'D') {
                if (currentPosition == 4) {
                    if (compareCode()) {
                        systemArmed = true;
                        lcd.clear();
                        lcd.print("System Armed");
                    } else {
                        lcd.clear();
                        lcd.print("Wrong Code");
                    }
                }
                currentPosition = 0;
                delay(2000);
                lcd.clear();
            }
            else if (key == '*') {
                currentPosition = 0;
                lcd.clear();
                lcd.print("Code Reset");
                delay(1000);
                lcd.clear();
            }
        }
    }
    
    if (systemArmed) {
        checkMotion();
    }
    checkRFID();
    handleWiFiClient();
}

void checkMotion() {
    if (digitalRead(MOTION_SENSOR) == HIGH) {
        digitalWrite(RED_LED, HIGH);
        lcd.clear();
        lcd.print("Intrusion!");
    } else {
        digitalWrite(RED_LED, LOW);
    }
}

void checkRFID() {
    if (rfid.isCard() && rfid.readCardSerial()) {
        if (compare_rfid(rfid.serNum, authorized_rfid)) {
            systemArmed = false;
            digitalWrite(RED_LED, LOW);
            lcd.clear();
            lcd.print("System Disarmed");
        } else {
            lcd.clear();
            lcd.print("Access Denied");
        }
        rfid.halt();
        delay(2000);
        lcd.clear();
    }
}

boolean compareCode() {
    for (byte i = 0; i < 4; i++) {
        if (code[i] != enteredCode[i]) {
            return false;
        }
    }
    return true;
}

void setupWiFi() {
    Serial.begin(9600);
    softserial.begin(115200);
    softserial.write("AT+CIOBAUD=9600\r\n");
    softserial.write("AT+RST\r\n");
    softserial.begin(9600);
    WiFi.init(&softserial);

    if (WiFi.status() == WL_NO_SHIELD) {
        lcd.clear();
        lcd.print("WiFi Error");
        while (true);
    }

    while (status != WL_CONNECTED) {
        lcd.clear();
        lcd.print("Connecting WiFi");
        status = WiFi.begin(ssid, pass);
    }

    lcd.clear();
    lcd.print("WiFi Connected");
}

// Fonction de gestion du client WiFi
void handleWiFiClient() {
    WiFiEspClient client = server.available();
    if (client) {
        boolean currentLineIsBlank = true;
        while (client.connected()) {
            if (client.available()) {
                char c = client.read();
                if (c == '\n' && currentLineIsBlank) {
                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-Type: text/html");
                    client.println();
                    break;
                }
                if (c == '\n') {
                    currentLineIsBlank = true;
                } else if (c != '\r') {
                    currentLineIsBlank = false;
                }
            }
        }
        client.stop();
    }
}

// Fonction de comparaison RFID
boolean compare_rfid(unsigned char x[], unsigned char y[]) {
    for (int i = 0; i < 4; i++) {
        if (x[i] != y[i]) return false;
    }
    return true;
}