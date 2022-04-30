// code based on the "HelloServer" code from the ESP32
// Arduino Wifi examples, with some NeoPixels mixed in
// requires Adafruit NeoPixel library and the ESP32 libraries

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Adafruit_NeoPixel.h>

const char* ssid = "DAL-EKS";    // Wifi network
const char* password = "girlinchleadleaf"; // Wifi password

const String mqttColours[12] = {
  "red"    , "green"  , "blue"     ,
  "cyan"   , "white"  , "purple"   ,
  "magenta", "yellow" , "orange"   ,
  "pink"   , "oldlace", "warmwhite"
};

// Keep the order the same as mqttColours[]
const int colourArray[12][3] = {
  {255, 0, 0}, {0, 255, 0}, {0, 0, 255},
  {0, 255, 255}, {255, 255, 255}, {128, 0, 128},
  {255, 0, 255}, {255, 255, 0}, {255, 165, 0},
  {255, 192, 203}, {253, 245, 230}, {253, 245, 230}
};

WebServer server(80);   // port for web server (80, default for HTTP)

const int led = 10;     // pin for the status LED
const int np = 8;       // pin for Neopixels
const int npCount = 25; // number of pixels

Adafruit_NeoPixel strip = Adafruit_NeoPixel(npCount, np, NEO_GRB + NEO_KHZ800);

// build a simple HTML page, as a string
// buttons to turn status LED on and off, or run a rainbow cycle
char html[] = "<html><head><title>ESP32-C3FH4-RGB</title>"
              "<meta name=\"viewport\" content=\"width=device-width initial-scale=1\">"
              "<style>"
              "html{font-family: Helvetica; display:inline-block; margin: 0px auto; text-align: center;}"
              "h1{color: #0F3376;}p{font-size: 1.3rem;}"
              ".button{display: inline-block; border: none; color: white; padding: 8px 30px; text-decoration: none; font-size: 20px; cursor: pointer;}"
              ".buttonR{background-color: #FF9999;}"
              ".button0{background-color: #BF0000;}"
              ".button1{background-color: #00BF00;}"
              ".button2{background-color: #0000BF;}"
              ".button3{background-color: #00BFBF;}"
              ".button4{background-color: #BFBFBF;}"
              ".button5{background-color: #580058;}"
              ".button6{background-color: #BF00BF;}"
              ".button7{background-color: #BFBF00;}"
              ".button8{background-color: #FFA500;}"
              ".button9{background-color: #BF98A3}"
              ".button10{background-color: #BDCDBE}"
              ".button11{background-color: #BDCDBE}"
              "</style>"
              "</head>"
              "<body><h1>ESP32-C3FH4-RGB Demo</h1>"
              "<p><a href=\"/R\"><button class=\"button buttonR\">rainbow</button></a></p>\n"
              "<p><a href=\"/C?c=0\"><button class=\"button button0\">red</button></a>\n"
              "<a href=\"/C?c=1\"><button class=\"button button1\">green</button></a>\n"
              "<a href=\"/C?c=2\"><button class=\"button button2\">blue</button></a></p>\n"
              "<p><a href=\"/C?c=3\"><button class=\"button button3\">cyan</button></a>\n"
              "<a href=\"/C?c=4\"><button class=\"button button4\">white</button></a>\n"
              "<a href=\"/C?c=5\"><button class=\"button button5\">purple</button></a></p>\n"
              "<p><a href=\"/C?c=6\"><button class=\"button button6\">magenta</button></a>\n"
              "<a href=\"/C?c=7\"><button class=\"button button7\">yellow</button></a>\n"
              "<a href=\"/C?c=8\"><button class=\"button button8\">orange</button></a></p>\n"
              "<p><a href=\"/C?c=9\"><button class=\"button button9\">pink</button></a>\n"
              "<a href=\"/C?c=10\"><button class=\"button button10\">oldlace</button></a>\n"
              "<a href=\"/C?c=11\"><button class=\"button button11\">warmwhite</button></a></p>\n"
              "</body></html>";

// NeoPixel colour functions

void colourWipe(uint32_t colour, int wait) {
  for (int i = 0; i < strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, colour);
    strip.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}

void rainbowCycle(uint8_t wait) {
  uint16_t i, j, x;

  for (j = 0; j < 256 * 3; j++) { // 3 cycles
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }

  // clear all pixels
  for (x = 0; x < strip.numPixels(); x++) {
    strip.setPixelColor(x, 0, 0, 0, 0);
  }
  strip.show();

}

uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if (WheelPos < 85) {
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
    WheelPos -= 170;
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}

// web server path handlers
void handleRoot() {
  server.sendHeader("Server", "Demo (ESP32-C3FH4-RGB)");
  server.sendHeader("Cache-Control", "no-cache");
  String message = "\nArguments: ";
  server.send(200, "text/html", html);
}

void handleRainbow() {
  rainbowCycle(3);
  handleRoot();
}

void handleColour() {
  String argC = server.arg("c");
  int arrIndex = argC.toInt();
  colourWipe(strip.Color(colourArray[arrIndex][0], colourArray[arrIndex][1], colourArray[arrIndex][2]), 50);
  handleRoot();
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.sendHeader("Server", "Demo (ESP32-C3FH4-RGB)");
  server.send(404, "text/plain", message);
}

// function to rapidly blink the status LED
void blinkLed() {
  for (int i = 0; i <= 5; i++) {
    digitalWrite(led, HIGH);
    delay(100);
    digitalWrite(led, LOW);
    delay(100);
  }
}

// init code, do all the basics
void setup(void) {
  pinMode(led, OUTPUT);

  strip.begin();           // INITIALIZE NeoPixel strip object
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(255);

  Serial.begin(115200);
  WiFi.useStaticBuffers(true);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // if the board has issues connecting, try uncommenting this
  WiFi.setTxPower(WIFI_POWER_5dBm);  // required for ESP32-C3FH4-RGB

  Serial.println("Cheer1 alive");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // set hostname and advertise on the network (http://esp32rgb.local)
  if (MDNS.begin("esp32rgb")) {
    Serial.println("mDNS responder started");
  }

  // specify how to handle different URL paths
  server.on("/", handleRoot);
  server.on("/R", handleRainbow);
  server.on("/C", handleColour);

  server.onNotFound(handleNotFound);

  server.begin();
  blinkLed(); // quickly blink - we have an IP address, now ready
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  delay(2); // allow the cpu to switch to other tasks
}
