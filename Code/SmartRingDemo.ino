#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <MAX30105.h>
#include "SC7A20.h"  // Include the SC7A20 library

#include "html.h"   // Separate file with webpage

// Define I2C pins
#define I2C_SCL 35
#define I2C_SDA 36

// WiFi credentials
const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

// Create server and WebSocket objects
WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

// Create sensor objects
MAX30105 particleSensor;
SC7A20_Class accelerometer;

bool checkForBeat(int32_t sample);
int16_t averageDCEstimator(int32_t *p, uint16_t x);
int16_t lowPassFIRFilter(int16_t din);
int32_t mul16(int16_t x, int16_t y);

const byte RATE_SIZE = 4; // Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; // Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; // Time at which the last beat occurred

float beatsPerMinute;
int beatAvg;
int stepCount = 0;

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  // Initialize I2C
  Wire.begin(I2C_SDA, I2C_SCL);
  // Initialize sensors
  particleSensor.begin();
  particleSensor.setup(); // Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A); // Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); // Turn off Green LED

  // Initialize accelerometer
  if (!accelerometer.begin(0x18, Wire)) {
    Serial.println("SC7A20 not detected. Check wiring.");
    while (1);
  }

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  // Start server and WebSocket
  server.on("/", webpage);
  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
  // Print the server IP address
  Serial.print("Server IP Address: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Handle client requests
  server.handleClient();
  webSocket.loop();

  // Read Sensor data and send it to the server
  long irValue = particleSensor.getIR();

  if (checkForBeat(irValue) == true)
  {
    // We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20)
    {
      rates[rateSpot++] = (byte)beatsPerMinute; // Store this reading in the array
      rateSpot %= RATE_SIZE; // Wrap variable

      // Take average of readings
      beatAvg = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }

    // Send heart rate data to the webpage
    String heartRateData = "{\"type\":\"heartRate\",\"value\":" + String(beatsPerMinute) + "}";
    webSocket.broadcastTXT(heartRateData);

    // Send average heart rate data to the webpage
    String avgHeartRateData = "{\"type\":\"avgHeartRate\",\"value\":" + String(beatAvg) + "}";
    webSocket.broadcastTXT(avgHeartRateData);
  }

  // Read accelerometer data
  accelerometer.measure();
  int16_t x = accelerometer.accel_X;
  int16_t y = accelerometer.accel_Y;
  int16_t z = accelerometer.accel_Z;

  // Calculate the magnitude of the acceleration vector
  float magnitude = sqrt(x * x + y * y + z * z);

  // Simple step detection algorithm
  static float lastMagnitude = 0;
  if (magnitude > 1.2 * lastMagnitude && magnitude > 1000) {
    stepCount++;
    String stepCountData = "{\"type\":\"motionData\",\"value\":" + String(stepCount) + "}";
    webSocket.broadcastTXT(stepCountData);
  }
  lastMagnitude = magnitude;
}

/*****************************************************************    
 *    webpage()
 * 
 *    This function sends the webpage to clients
 *****************************************************************/
void webpage()
{
  server.send(200, "text/html", htmlCode);
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  // Handle WebSocket events
  if (type == WStype_TEXT) {
    Serial.printf("[%u] Received text: %s\n", num, payload);
  }
}
