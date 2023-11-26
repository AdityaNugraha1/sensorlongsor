#include <WiFi.h>
#include <TinyGPS++.h>
#include <HTTPClient.h>

const char* ssid = "JTM274A";
const char* password = "wismajtm";

const int soilMoistureSensorPin = 34;
const int raindropsSensorPin = 35;

const int soilMoistureThreshold = 700;
const int raindropsThreshold = 300;

float latitude;
float longitude;

// Inisialisasi objek TinyGPS++
TinyGPSPlus gps;

unsigned long previousMillis = 0;
const long interval = 30 * 60 * 1000;  // 30 minutes in milliseconds

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600); // Inisialisasi komunikasi serial dengan modul GPS
  pinMode(soilMoistureSensorPin, INPUT);
  pinMode(raindropsSensorPin, INPUT);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(4000);
    Serial.println("Connecting to WiFi");
  }
  Serial.println("Connected to WiFi");
}

void sendPostRequestWithSensorData() {
  int soilMoisture = analogRead(soilMoistureSensorPin);
  int raindrops = analogRead(raindropsSensorPin);
  bool potentialLandslide = soilMoisture > soilMoistureThreshold && raindrops > raindropsThreshold;

  // Membaca data lokasi GPS dari modul
  latitude = gps.location.lat();
  longitude = gps.location.lng();

  // Jika WiFi terhubung dan sudah 30 menit berlalu
  if (WiFi.status() == WL_CONNECTED && millis() - previousMillis >= interval) {
    HTTPClient http;

    // Menyiapkan URL target
    http.begin("https://praktikumiot-406205.et.r.appspot.com/api/insert_sensor_data");

    // Mengatur tipe konten sebagai application/x-www-form-urlencoded
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");

    // Membuat payload data dalam format x-www-form-urlencoded
    String postData = "soilMoisture=" + String(soilMoisture) +
                      "&raindrops=" + String(raindrops) +
                      "&potentialLandslide=" + String(potentialLandslide ? "yes" : "no") +
                      "&latitude=" + String(latitude, 6) +
                      "&longitude=" + String(longitude, 6);

    // Mengirim permintaan POST dengan payload data
    int httpResponseCode = http.POST(postData);

    // Cek kode respons
    if (httpResponseCode > 0) {
      String response = http.getString();
      Serial.print("Response code: ");
      Serial.println(httpResponseCode);
      Serial.print("Response: ");
      Serial.println(response);
    } else {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
      Serial.println(soilMoisture);
      Serial.println(raindrops);
      Serial.println(potentialLandslide);
    }

    // Tutup koneksi
    http.end();

    // Reset the timer
    previousMillis = millis();
  }

  // Print serial data every 3 seconds
  Serial.println("Sensor Data:");
  Serial.print("Soil Moisture: ");
  Serial.println(soilMoisture);
  Serial.print("Raindrops: ");
  Serial.println(raindrops);
  Serial.print("Potential Landslide: ");
  Serial.println(potentialLandslide ? "yes" : "no");
  Serial.println("===================");

  delay(3000);
}

void loop() {
  while (Serial2.available() > 0) {
    if (gps.encode(Serial2.read())) {
      // Jika berhasil mendapatkan data GPS, panggil fungsi untuk mengirim data
      sendPostRequestWithSensorData();
    }
  }
}
