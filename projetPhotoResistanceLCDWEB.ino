#include "WiFiEsp.h"
#include "SoftwareSerial.h"
#include <LiquidCrystal_I2C.h>

// Configuration du matériel
LiquidCrystal_I2C lcd(0x3f, 16, 2);
SoftwareSerial softserial(A9, A8);

// Définition des pins
#define LIGHT_SENSOR A0
#define LED 13

// Variables WiFi
char ssid[] = "wifihome";
char pass[] = "sinissy1";
int status = WL_IDLE_STATUS;

// Variables globales
int lightStatus = 0;
String lightStr;
boolean autoMode = true;

WiFiEspServer server(80);
RingBuffer buf(80);

void setup() {
  // Initialisation des pins
  pinMode(LED, OUTPUT);
  pinMode(LIGHT_SENSOR, INPUT);
  digitalWrite(LED, HIGH);

  // Initialisation série et LCD
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  
  Serial.println("\n=== Démarrage du système ===");
  lcd.print("Demarrage...");

  // Configuration WiFi
  Serial.println("Configuration WiFi...");
  softserial.begin(115200);
  softserial.write("AT+CIOBAUD=9600\r\n");
  delay(1000);
  softserial.write("AT+RST\r\n");
  delay(1000);
  softserial.begin(9600);
  
  WiFi.init(&softserial);
  delay(1000);

  // Vérification module WiFi
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("ERREUR: Module WiFi non détecté!");
    lcd.clear();
    lcd.print("WiFi absent!");
    while (true) {
      digitalWrite(LED, !digitalRead(LED));
      delay(500);
    }
  }

  // Connexion WiFi
  Serial.println("\nConnexion au réseau WiFi...");
  lcd.clear();
  lcd.print("Connexion WiFi...");
  
  while (status != WL_CONNECTED) {
    Serial.print("Tentative connexion à ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    delay(1000);
  }

  // Affichage informations réseau
  Serial.println("\n=== Connexion établie ===");
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  Serial.print("Force du signal: ");
  Serial.print(WiFi.RSSI());
  Serial.println(" dBm");
  
  IPAddress ip = WiFi.localIP();
  Serial.print("Adresse IP: ");
  Serial.println(ip);
  Serial.println("\nPour accéder à l'interface web:");
  Serial.print("http://");
  Serial.println(ip);
  Serial.println("========================");

  lcd.clear();
  lcd.print("IP:");
  lcd.setCursor(0, 1);
  lcd.print(ip);
  
  server.begin();
  digitalWrite(LED, LOW);
}


void loop() {
  // Lecture capteur
  int lightValue = analogRead(LIGHT_SENSOR);
  Serial.print("Valeur luminosite: ");
  Serial.println(lightValue);
  
  // Mode automatique
  if (autoMode) {
    if (lightValue > 750) {  // Si la photorésistance est couverte (valeur haute)
      digitalWrite(LED, LOW);
      lightStr = "OFF (Auto)";
    } else {  // Si la photorésistance détecte de la lumière (valeur basse)
      digitalWrite(LED, HIGH);
      lightStr = "ON (Auto)";
    }
  }

  // Mise à jour LCD
  lcd.setCursor(0, 0);
  lcd.print("Lum: ");
  lcd.print(lightValue);
  lcd.print("   ");
  
  lcd.setCursor(0, 1);
  lcd.print("LED: ");
  lcd.print(lightStr);
  lcd.print("   ");
  // Gestion client web
  WiFiEspClient client = server.available();
  if (client) {
    Serial.println("Nouveau client");
    boolean currentLineIsBlank = true;
    String currentLine = "";
    
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        
        if (c == '\n' && currentLineIsBlank) {
          sendWebPage(client);
          break;
        }
        
        if (c == '\n') {
          if (currentLine.indexOf("GET /ON") >= 0) {
            autoMode = false;
            digitalWrite(LED, HIGH);
            lightStr = "ON (Manuel)";
          }
          if (currentLine.indexOf("GET /OFF") >= 0) {
            autoMode = false;
            digitalWrite(LED, LOW);
            lightStr = "OFF (Manuel)";
          }
          if (currentLine.indexOf("GET /AUTO") >= 0) {
            autoMode = true;
          }
          currentLine = "";
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          currentLine += c;
          currentLineIsBlank = false;
        }
      }
    }
    client.stop();
    Serial.println("Client deconnecte");
  }
}

void sendWebPage(WiFiEspClient client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
  
  // Page HTML
  client.println("<!DOCTYPE HTML>");
  client.println("<html><head>");
  client.println("<meta name='viewport' content='width=device-width, initial-scale=1'>");
  client.println("<meta http-equiv='refresh' content='5'>");
  client.println("<style>");
  client.println("body { text-align: center; font-family: Arial; }");
  client.println(".button { padding: 10px 20px; margin: 5px; }");
  client.println("</style></head>");
  client.println("<body>");
  
  client.println("<h1>Controle Eclairage</h1>");
  client.println("<p>Luminosite: " + String(analogRead(LIGHT_SENSOR)) + "</p>");
  client.println("<p>Mode: " + String(autoMode ? "Automatique" : "Manuel") + "</p>");
  client.println("<p>LED: " + lightStr + "</p>");
  
  client.println("<a href='/ON'><button class='button'>ON</button></a>");
  client.println("<a href='/OFF'><button class='button'>OFF</button></a>");
  client.println("<a href='/AUTO'><button class='button'>Mode Auto</button></a>");
  
  client.println("</body></html>");
}