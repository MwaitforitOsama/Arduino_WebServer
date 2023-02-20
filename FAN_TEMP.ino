#include <ESP8266WiFi.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#define DHTPIN 14
#define DHTTYPE DHT11

const int greenButton = 7; // green button pin
const int redButton = 13;    // red button pin
const int fan = 14;          // fan pin

unsigned long greenButtonTime = 0;
unsigned long redButtonTime = 0;

bool greenButtonState = false;
bool redButtonState = false;

DHT dht(DHTPIN, DHTTYPE);
WiFiServer server(80);

// debouncing function
bool debounce(bool last, int pin) {
  bool current = digitalRead(pin);
  if (last != current) {
    delay(5);
    current = digitalRead(pin);
  }
  return current;
}

void setup() {
  pinMode(greenButton, INPUT_PULLUP);
  pinMode(redButton, INPUT_PULLUP);
  pinMode(fan, OUTPUT);
  Serial.begin(115200);
  dht.begin();
  



  WiFi.begin("SSID", "PASSWORD");

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");
  server.begin();
}

void loop() {
  float REFTEMP = dht.readTemperature();
  WiFiClient client = server.available();

  greenButtonState = debounce(greenButtonState, greenButton);
  redButtonState = debounce(redButtonState, redButton);

  float temperature = dht.readTemperature();

  if (client) {
    String request = client.readStringUntil('\r');

    if (request.indexOf("/green") != -1) {
      digitalWrite(fan, HIGH);
      greenButtonTime = millis();
      while (millis() - greenButtonTime < 2000) {
        // wait for 2 seconds
      }
      digitalWrite(fan, LOW);
      greenButtonTime = millis();
      while (millis() - greenButtonTime < 4000) {
        // wait for 4 seconds
      }
    }

    if (request.indexOf("/red") != -1) {
      redButtonTime = millis();
      while (millis() - redButtonTime < 4000) {
        // wait for 4 seconds
      }
      digitalWrite(fan, LOW);
    }

    if (request.indexOf("/setref") != -1) {
      int start = request.indexOf("value=") + 6;
      int end = request.indexOf(" HTTP");
      REFTEMP = request.substring(start, end).toFloat();
    }

    String response = "HTTP/1.1 200 OK\r\n\r\n";
    response += "<html><body><h1>ESP8266 Web Server</h1>";
    response += "<form action='/setref' method='get'>";
    response += "Reference Temperature: <input type='text' name='value' value='" + String(REFTEMP) + "'><br><br>";
    response += "Reference Temperature: <input type='text' name='value' value='" + String(REFTEMP) + "'><br><br>";
    response += "<button style='background-color: green; color: white; padding: 16px;' name='green' value='green'><a href='/green'>Green Button</a></button>&nbsp;";
    response += "<button style='background-color: red; color: white; padding: 16px;' name='red' value='red'><a href='/red'>Red Button</a></button><br><br>";
    response += "Current Temperature: " + String(temperature) + "<br><br>";
    response += "Fan is " + String((digitalRead(fan) == HIGH ? "ON" : "OFF")) + "<br><br>";
    response += "</body></html>";

    client.print(response);
    client.stop();
  }

  if (temperature > REFTEMP) {
    digitalWrite(fan, HIGH);
  } else {
    digitalWrite(fan, LOW);
  }

  if (greenButtonState == LOW) {
    digitalWrite(fan, HIGH);
    greenButtonTime = millis();
    while (millis() - greenButtonTime < 2000) {
      // wait for 2 seconds
    }
    digitalWrite(fan, LOW);
    greenButtonTime = millis();
    while (millis() - greenButtonTime < 4000) {
      // wait for 4 seconds
    }
  }

  if (redButtonState == LOW) {
    redButtonTime = millis();
    while (millis() - redButtonTime < 4000) {
      // wait for 4 seconds
    }
    digitalWrite(fan, LOW);
  }
}
