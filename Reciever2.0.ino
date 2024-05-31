#include <SPI.h>
#include <LoRa.h>
#include <WiFiNINA.h>
#include <ArduinoHttpClient.h>

// WiFi configuration
const char* ssid = "Galaxy Note10+678e";
const char* password = "jlpj9353";

// Your server's URL
const char* serverName = "192.168.56.22";
const int serverPort = 3000;

WiFiClient wifiClient;
HttpClient client = HttpClient(wifiClient, serverName, serverPort);

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  // Initialize Serial Monitor
  Serial.begin(9600);
  while (!Serial); // Wait for the serial port to connect
  Serial.println("LoRa Receiver");

  setup_wifi();

  // Initialize LoRa module
  if (!LoRa.begin(433E6)) { // or 915E6, depending on your module
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  Serial.println("Ready to receive packets.");
}

void loop() {
  // Try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // Received a packet
    Serial.print("Received packet: ");
    
    // Read packet
    uint8_t receivedVals[19];
    for (int i = 0; i < 19; i++) {
      receivedVals[i] = LoRa.read();
    }

    // Extract data from the received packet
    int gpsYear = (receivedVals[0] << 8) | receivedVals[1];
    int gpsMonth = receivedVals[2];
    int gpsDay = receivedVals[3];
    int gpsHour = receivedVals[4];
    int gpsMin = receivedVals[5];
    int gpsSec = receivedVals[6];
    float gpsLat;
    float gpsLong;
    memcpy(&gpsLat, &receivedVals[7], sizeof(gpsLat));
    memcpy(&gpsLong, &receivedVals[11], sizeof(gpsLong));
    int temperature = receivedVals[15];
    int humidity = receivedVals[16];
    int smoke_val = (receivedVals[17] << 8) | receivedVals[18];

    // Print the received values
    Serial.print("GPS Date: ");
    Serial.print(gpsYear);
    Serial.print("-");
    Serial.print(gpsMonth);
    Serial.print("-");
    Serial.println(gpsDay);

    Serial.print("GPS Time: ");
    Serial.print(gpsHour);
    Serial.print(":");
    Serial.print(gpsMin);
    Serial.print(":");
    Serial.println(gpsSec);

    Serial.print("GPS Latitude: ");
    Serial.println(gpsLat, 6);

    Serial.print("GPS Longitude: ");
    Serial.println(gpsLong, 6);

    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");

    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");

    Serial.print("Smoke Detector Reading: ");
    Serial.println(smoke_val);
    Serial.println("-----");

    // Create JSON payload
    String payload = "{";
    payload += "\"gpsYear\":" + String(gpsYear) + ",";
    payload += "\"gpsMonth\":" + String(gpsMonth) + ",";
    payload += "\"gpsDay\":" + String(gpsDay) + ",";
    payload += "\"gpsHour\":" + String(gpsHour) + ",";
    payload += "\"gpsMin\":" + String(gpsMin) + ",";
    payload += "\"gpsSec\":" + String(gpsSec) + ",";
    payload += "\"gpsLat\":" + String(gpsLat, 6) + ",";
    payload += "\"gpsLong\":" + String(gpsLong, 6) + ",";
    payload += "\"temperature\":" + String(temperature) + ",";
    payload += "\"humidity\":" + String(humidity) + ",";
    payload += "\"smoke_val\":" + String(smoke_val);
    payload += "}";

    // Send data to server
    if (WiFi.status() == WL_CONNECTED) {
      client.beginRequest();
      client.post("/api/data");
      client.sendHeader("Content-Type", "application/json");
      client.sendHeader("Content-Length", payload.length());
      client.beginBody();
      client.print(payload);
      client.endRequest();

      int statusCode = client.responseStatusCode();
      String response = client.responseBody();
      Serial.print("HTTP Response code: ");
      Serial.println(statusCode);
      Serial.println(response);
    } else {
      Serial.println("WiFi Disconnected");
    }
  }
  delay(1000); // Wait a bit before next reception
}
