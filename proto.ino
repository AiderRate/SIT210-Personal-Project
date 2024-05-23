#include <DHT11.h>
#include <TinyGPS++.h>
#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>

// Create an instance of the DHT11 class.
DHT11 dht11(2);

// Create an instance of the TinyGPS++ object.
TinyGPSPlus gps;

void setup() {
  // Initialize USB serial for debugging
  Serial.begin(9600);
  while (!Serial); // Wait for serial port to connect
  Serial.println("LoRa Transmitter");

  // SX1278 setup
  if (!LoRa.begin(433E6)) { // or 915E6, the MHz speed of your module
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  // Print debug message
  Serial.println("Initializing...");

  // Initialize Serial1 for GPS communication
  Serial1.begin(9600);
  delay(1000); // Wait for the GPS module to initialize

  Serial.println("Ready.");
}

void loop() {
  // Values for Transmission
  int gpsYear = 0;
  int gpsMonth = 0;
  int gpsDay = 0;
  int gpsHour = 0;
  int gpsMin = 0;
  int gpsSec = 0;
  float gpsLat = 0.0;
  float gpsLong = 0.0;

  // Attempt to read data from the DHT11 sensor
  int temperature = 0;
  int humidity = 0;
  int result = dht11.readTemperatureHumidity(temperature, humidity);
  int smoke_val = analogRead(0); // Read Gas value from analog 0

  // Print temperature and humidity readings
  if (result == 0) {
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.print(" °C\tHumidity: ");
    Serial.print(humidity);
    Serial.println(" %");
  } else {
    Serial.println("Failed to read DHT11 sensor.");
  }

  // Print smoke detector reading
  Serial.print("Smoke Detector Reading: ");
  Serial.println(smoke_val);

  // Parse GPS data
  while (Serial1.available() > 0) {
    gps.encode(Serial1.read());
  }

  // Display GPS information
  if (gps.location.isValid()) {
    gpsLat = gps.location.lat();
    gpsLong = gps.location.lng();
    Serial.print("Latitude: ");
    Serial.print(gpsLat, 6);
    Serial.print(" Longitude: ");
    Serial.println(gpsLong, 6);
  } else {
    Serial.println("Location: Not Available");
  }

  if (gps.date.isValid()) {
    gpsMonth = gps.date.month();
    gpsDay = gps.date.day();
    gpsYear = gps.date.year();
    Serial.print("Date: ");
    Serial.print(gpsMonth);
    Serial.print("/");
    Serial.print(gpsDay);
    Serial.print("/");
    Serial.println(gpsYear);
  } else {
    Serial.println("Date: Not Available");
  }

  if (gps.time.isValid()) {
    gpsHour = gps.time.hour();
    gpsMin = gps.time.minute();
    gpsSec = gps.time.second();
    Serial.print("Time: ");
    if (gps.time.hour() < 10) Serial.print("0");
    Serial.print(gps.time.hour());
    Serial.print(":");
    if (gps.time.minute() < 10) Serial.print("0");
    Serial.print(gps.time.minute());
    Serial.print(":");
    if (gps.time.second() < 10) Serial.print("0");
    Serial.println(gps.time.second());
  } else {
    Serial.println("Time: Not Available");
  }

  // Initialize array to be transmitted
  uint8_t payload[19];
  payload[0] = (gpsYear >> 8) & 0xFF;
  payload[1] = gpsYear & 0xFF;
  payload[2] = gpsMonth;
  payload[3] = gpsDay;
  payload[4] = gpsHour;
  payload[5] = gpsMin;
  payload[6] = gpsSec;
  memcpy(&payload[7], &gpsLat, sizeof(gpsLat));
  memcpy(&payload[11], &gpsLong, sizeof(gpsLong));
  payload[15] = temperature;
  payload[16] = humidity;
  payload[17] = (smoke_val >> 8) & 0xFF;
  payload[18] = smoke_val & 0xFF;

  // LoRa Transmission
  LoRa.beginPacket();
  LoRa.write(payload, sizeof(payload));
  LoRa.endPacket();

  // Print data for Serial Plotter
  Serial.print("GPS Lat:");
  Serial.print(gpsLat, 6);
  Serial.print(" GPS Long:");
  Serial.println(gpsLong, 6);

  // Clear GPS data for the next iteration
  clearGPSData();
  Serial.println("Sent");
  // Delay before next loop iteration
  delay(1000);
}

void clearGPSData() {
  // Reset GPS object to clear data
  gps.location.lat();
  gps.location.lng();
  gps.date.month();
  gps.date.day();
  gps.date.year();
  gps.time.hour();
  gps.time.minute();
  gps.time.second();
}
