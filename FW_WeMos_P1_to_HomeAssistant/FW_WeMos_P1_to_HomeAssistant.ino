#define RST_PIN D2    // Wemos D2 (GPIO4)
#define RGB_R_PIN 12  // Wemos D6 (GPIO12)
#define RGB_G_PIN 5   // Wemos D1 (GPIO5)
#define RGB_B_PIN 14  // Wemos D5 (GPIO14)

#define P1_SERIAL_PORT Serial

#include <Arduino.h>      //Inculded with esp8266 package
#include "update.h"
#include <ESP8266WiFi.h>  // Included with esp8266 package
#include <WiFiManager.h>  // https://github.com/tzapu/WiFiManager
#include <ESP8266mDNS.h>  // Included with esp8266 package

//Kies een Poort nummer, in dit geval 23
WiFiServer server(23);
WiFiClient client;

#define IDEAL_PACKET_SIZE 1024

//Funcion Prototyps
void pulseColor(int pin, uint16_t delayTime = 0);

void setup() {
  WiFiManager wm;

  pinMode(RST_PIN, INPUT_PULLUP);
  pinMode(RGB_R_PIN, OUTPUT);  // Red RGB led
  pinMode(RGB_G_PIN, OUTPUT);  // Green RGB led
  pinMode(RGB_B_PIN, OUTPUT);  // Blue RGB led

  digitalWrite(RGB_R_PIN, HIGH);
  digitalWrite(RGB_G_PIN, HIGH);
  digitalWrite(RGB_B_PIN, HIGH);

  //Hold the button on the PCB to reset the Wi-Fi Settings
  if (digitalRead(RST_PIN) == LOW) {
    delay(2000);
    if (digitalRead(RST_PIN) == LOW) {
      pulseColor(RGB_R_PIN);
      delay(5000);
      if (digitalRead(RST_PIN) == LOW) {
        pulseColor(RGB_R_PIN);
        pulseColor(RGB_R_PIN);
        pulseColor(RGB_R_PIN);
        wm.resetSettings();
        delay(1000);
        pulseColor(RGB_G_PIN);  //Hold the button until it pulses green
        delay(1000);
      }
    }
  }



  pulseColor(RGB_R_PIN);
  pulseColor(RGB_G_PIN);
  pulseColor(RGB_B_PIN);

  Serial.begin(115200, SERIAL_8N1);


  //Verander de receive buffer grootte
  Serial.setRxBufferSize(IDEAL_PACKET_SIZE * 8);
  Serial.setDebugOutput(false);

  //allow for multiple esp-p1-power on the network
  String hostname = "SlimmeMeter2HomeAssisant";
  P1_SERIAL_PORT.println(hostname);
  WiFi.mode(WIFI_STA);
  WiFi.hostname(hostname.c_str());


  digitalWrite(RGB_B_PIN, LOW);

  wm.autoConnect("Smart Meter Wi-Fi Setup");

  digitalWrite(RGB_B_PIN, HIGH);

  pulseColor(RGB_G_PIN, 2000);
  
    // Initialize mDNS
  if (MDNS.begin(hostname)) {
    Serial.println("mDNS responder started");
  } else {
    Serial.println("Error starting mDNS responder");
  }


  server.begin();
  server.setNoDelay(true);



  digitalWrite(RGB_R_PIN, LOW);
  digitalWrite(RGB_G_PIN, LOW);
  digitalWrite(RGB_B_PIN, LOW);

  delay(2000);

  digitalWrite(RGB_R_PIN, HIGH);
  digitalWrite(RGB_G_PIN, HIGH);
  digitalWrite(RGB_B_PIN, HIGH);

  delay(1000);

  String ipString = WiFi.localIP().toString();
  // Loop through each character in the IP string
  for (int i = 0; i < ipString.length(); i++) {
    // Get the current character
    char c = ipString.charAt(i);

    // Blink the red LED to indicate the decimal point
    if (c == '.') {
      pulseColor(RGB_R_PIN);
      delay(500);
      i++;
      continue;
    }

    // Convert the character to an integer
    int digit = c - '0';

    // Blink the blue LED for the first digit
    for (int j = 0; j < digit; j++) {
      pulseColor(RGB_B_PIN);
      delay(500);
    }

    // Blink the green LED for the second digit

    pulseColor(RGB_G_PIN);

    delay(2000);
  }

  digitalWrite(RGB_R_PIN, LOW);
  digitalWrite(RGB_G_PIN, LOW);
  digitalWrite(RGB_B_PIN, LOW);

  delay(2000);

  digitalWrite(RGB_R_PIN, HIGH);
  digitalWrite(RGB_G_PIN, HIGH);
  digitalWrite(RGB_B_PIN, HIGH);

  //After setup swap P1_SERIAL_PORT pins for reading the P1 port
  P1_SERIAL_PORT.swap();
}

unsigned long lastUpdate = 0;

void loop() {
  unsigned long now = millis();

  size_t available = P1_SERIAL_PORT.peekAvailable();
  if (available >= IDEAL_PACKET_SIZE || (available > 0 && (now - lastUpdate) > 50)) {
    const char* buffer = P1_SERIAL_PORT.peekBuffer();
    if (client && client.connected()) {
      analogWrite(RGB_B_PIN, 240);
      size_t written = client.write(buffer, available);
      P1_SERIAL_PORT.peekConsume(written);
      lastUpdate = now;
      digitalWrite(RGB_B_PIN, HIGH);
    } else {
      P1_SERIAL_PORT.peekConsume(0);
    }
  }
  if (server.hasClient()) {
    client.stop();
    client = server.available();
  }


  static unsigned long prevBlinkMillis = millis();
  if (millis() - prevBlinkMillis > 10000) {
    quickFlash(RGB_G_PIN);
    prevBlinkMillis = millis();
  }

  static unsigned long prevUpdateCheck = millis();
  //Check every day for a firmware update
  if (millis() - prevUpdateCheck > 24 * 3600 * 1000) {
    handleFWUpdate();
  }

  delay(10);
}