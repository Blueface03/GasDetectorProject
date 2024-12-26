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
#define VPIN_BUZZER_STATUS V3    // Virtual Pin untuk menampilkan status buzzer


// Definisi pin I2C untuk ESP32
#define I2C_SDA 21
#define I2C_SCL 22

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
const int GAS_MAX = 4095;   // Nilai maximum pembacaan sensor (12-bit ADC ESP32)

// Inisialisasi LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Variables
int gasValue;
int gasPercentage;
bool isWarning = false;
String statusGas = "";
bool manualFanControl = false;
bool manualBuzzerControl = false;

// Timer untuk Blynk
BlynkTimer timer;

// Fungsi untuk mengirim data ke Blynk
void sendSensorData() {
  Blynk.virtualWrite(VPIN_GAS_PERCENTAGE, gasPercentage);
  Blynk.virtualWrite(VPIN_GAS_STATUS, statusGas);
  Blynk.virtualWrite(VPIN_FAN_STATUS, digitalRead(FAN_PIN) ? "ON" : "OFF");
  Blynk.virtualWrite(VPIN_BUZZER_STATUS, digitalRead(BUZZER_PIN) ? "ON" : "OFF");
}


void setup() {
  Serial.begin(115200);
  Serial.println("Sistem Mulai...");
  
  // Inisialisasi pin
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(MQ2_PIN, INPUT);
  
  // Koneksi ke Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  
  // Setup timer untuk mengirim data ke Blynk setiap 1 detik
  timer.setInterval(1000L, sendSensorData);
  
  // Inisialisasi I2C dan LCD
  Wire.begin(I2C_SDA, I2C_SCL);
  lcd.init();
  lcd.backlight();
    
  // Tampilan awal LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Detektor Gas LPG");
  lcd.setCursor(0, 1);
  lcd.print("Sistem Siap!");
    
  testComponents();
  delay(2000);
}

void testComponents() {
  Serial.println("Tes Komponen...");
    
  Serial.println("Tes Buzzer...");
  digitalWrite(BUZZER_PIN, HIGH);
  delay(500);
  digitalWrite(BUZZER_PIN, LOW);
    
  Serial.println("Tes Kipas...");
  digitalWrite(FAN_PIN, HIGH);
  delay(1000);
  digitalWrite(FAN_PIN, LOW);
    
  Serial.println("Tes LED...");
  digitalWrite(LED_PIN, HIGH);
  delay(500);
  digitalWrite(LED_PIN, LOW);
}

int calculateGasPercentage(int rawValue) {
  return map(rawValue, GAS_MIN, GAS_MAX, 0, 100);
}

String getStatusMessage(int percentage) {
  if (percentage <= AMAN) {
    return "AMAN";
  } else if (percentage <= WASPADA) {
    return "WASPADA";
  } else {
    return "BAHAYA";
  }
}

void handleGasWarning(int percentage) {
  String status = getStatusMessage(percentage);
  
  if (!manualFanControl && !manualBuzzerControl) {  // Hanya jalankan otomatis jika tidak dalam mode manual
    if (status == "BAHAYA") {
      digitalWrite(BUZZER_PIN, HIGH);
      delay(1000);
      digitalWrite(BUZZER_PIN, LOW);
      
      digitalWrite(FAN_PIN, HIGH);
      digitalWrite(LED_PIN, HIGH);
      
      // Kirim notifikasi ke Blynk
      Blynk.logEvent("gas_alert", "BAHAYA! Kadar Gas LPG Tinggi!");
      
    } else if (status == "WASPADA") {
      for(int i = 0; i < 2; i++) {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(200);
        digitalWrite(BUZZER_PIN, LOW);
        delay(200);
      }
      
      digitalWrite(FAN_PIN, HIGH);
      digitalWrite(LED_PIN, !digitalRead(LED_PIN));
      
      // Kirim notifikasi ke Blynk
      Blynk.logEvent("gas_warning", "WASPADA! Kadar Gas LPG Terdeteksi");
      
    } else {
      normalCondition();
    }
  }
}

void normalCondition() {
  if (!manualFanControl) {
    digitalWrite(FAN_PIN, LOW);
  }
  if (!manualBuzzerControl) {
    digitalWrite(BUZZER_PIN, LOW);
  }
  digitalWrite(LED_PIN, LOW);
}

void updateDisplay(int percentage, String status) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("LPG: " + String(percentage) + "%");
  lcd.setCursor(0, 1);
  lcd.print("Status: " + status);
}

void loop() {
  Blynk.run();
  timer.run();
  
  gasValue = analogRead(MQ2_PIN);
  gasPercentage = calculateGasPercentage(gasValue);
  statusGas = getStatusMessage(gasPercentage);
  
  updateDisplay(gasPercentage, statusGas);
  handleGasWarning(gasPercentage);
  
  delay(100);  // Delay dikurangi karena sudah ada timer Blynk
}    
