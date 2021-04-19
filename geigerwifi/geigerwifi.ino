
// Load Wi-Fi library
#include <Vector.h>
#include <WiFi.h>
#include <string.h>
#include "ESP32TimerInterrupt.h"
// Network credentials
const char* ssid = "ssid";
const char* password = "password";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;
uint32_t tablica[1000];
Vector<uint32_t> wektor(tablica);
ESP32Timer ITimer1(1);
struct tube {
  const uint8_t PIN;
  uint32_t clicks;
  bool tubeon;
  float cpm;
};
tube tube1 = {34, 0, false, 0};
long czas;
double wspczasu;
void IRAM_ATTR isr() {
  tube1.clicks += 1;
  tube1.tubeon = true;
  wspczasu = czas/60000.0;
  tube1.cpm = tube1.clicks/wspczasu;
}
void IRAM_ATTR pobranie_probki() {
  wektor.push_back(tube1.clicks);
}
// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
  ITimer1.attachInterruptInterval(10000 * 1000, pobranie_probki);
  Serial.begin(115200);
  // Initialize the output variables as outputs
  pinMode(tube1.PIN, INPUT);
  attachInterrupt(tube1.PIN, isr, FALLING);
  czas = millis();


  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop(){
    czas = millis();
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<meta http-equiv=\"refresh\" content=\"1\" />");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>ESP32 Web Server</h1>");
            
            // Display current state 
            client.println("<p>klik = " + String(tube1.clicks) + " cpm = " + String(tube1.cpm) + " wspczasu = " + String(wspczasu) + "</p>");
            client.println("<p/>");
            client.println("<p/>");
            client.println("<p>");
            for (auto&& probka : wektor){
            client.println(String(probka) + "<br>");
            }
            client.println("</p>");
          
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
