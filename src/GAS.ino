// Konfigurasi Blynk dan WiFi
#define BLYNK_TEMPLATE_ID "TMPL60Pg5kZUs"
#define BLYNK_TEMPLATE_NAME "GAS level monitoring system"
#define BLYNK_AUTH_TOKEN "EYozH08_PCE92H1AUrjjCv0fQ6BJ-tDl"

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// Informasi WiFi
char ssid[] = "Zaky";     // Nama WiFi Anda
char pass[] = "70240900"; // Password WiFi Anda

// Definisi Virtual Pin Blynk
#define VPIN_GAS_PERCENTAGE V0    // Untuk menampilkan persentase gas
#define VPIN_GAS_STATUS V1        // Untuk menampilkan status (AMAN/WASPADA/BAHAYA)
#define VPIN_FAN_STATUS V2        // Untuk menampilkan status kipas
#define VPIN_BUZZER_STATUS V3     // Virtual Pin untuk menampilkan status buzzer

// Definisi pin komponen
const int MQ2_PIN = 34;      // Analog pin untuk sensor MQ2
const int BUZZER_PIN = 25;   // Pin untuk buzzer
const int FAN_PIN = 26;      // Pin untuk kipas
const int LED_PIN = 2;       // Built-in LED

// Threshold untuk tingkat bahaya gas LPG
const int AMAN = 20;         // Di bawah 20% dianggap aman
const int WASPADA = 40;      // 20-40% perlu waspada
const int BAHAYA = 60;       // Di atas 40% dianggap bahaya

// Kalibrasi sensor
const int GAS_MIN = 0;      // Nilai minimum pembacaan sensor
const int GAS_MAX = 4095;   // Nilai maksimum pembacaan sensor (12-bit ADC ESP32)

// Timer untuk Blynk
BlynkTimer timer;

// Variabel
int gasValue;
int gasPercentage;
String statusGas = "";

// Fungsi membaca data gas
void readGasData() {
  gasValue = analogRead(MQ2_PIN);
  gasPercentage = map(gasValue, GAS_MIN, GAS_MAX, 0, 100);
  if (gasPercentage < AMAN) {
    statusGas = "AMAN";
    digitalWrite(FAN_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
  } else if (gasPercentage < WASPADA) {
    statusGas = "WASPADA";
    digitalWrite(FAN_PIN, LOW);
    digitalWrite(BUZZER_PIN, LOW);
  } else {
    statusGas = "BAHAYA";
    digitalWrite(FAN_PIN, HIGH);
    digitalWrite(BUZZER_PIN, HIGH);
  }
}

// Fungsi mengirim data ke Blynk
void sendSensorData() {
  Blynk.virtualWrite(VPIN_GAS_PERCENTAGE, gasPercentage);
  Blynk.virtualWrite(VPIN_GAS_STATUS, statusGas);
}

// Setup awal
void setup() {
  Serial.begin(115200);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  pinMode(MQ2_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);

  timer.setInterval(1000L, readGasData);
  timer.setInterval(2000L, sendSensorData);
}

// Loop utama
void loop() {
  Blynk.run();
  timer.run();
}
