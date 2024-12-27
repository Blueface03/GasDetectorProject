#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <WebServer.h>

/* Konfigurasi WiFi */
const char* ssid = "Zaky";          // Nama WiFi
const char* password = "70240900";  // Password WiFi

/* Definisi pin komponen */
const int MQ2_PIN = 34;      // Pin sensor gas
const int BUZZER_PIN = 25;   // Pin buzzer
const int LED_PIN = 2;       // Pin LED

/* Threshold untuk status gas */
const int AMAN = 20;         // Di bawah 20% dianggap aman
const int WASPADA = 40;      // 20-40% perlu waspada
const int BAHAYA = 60;       // Di atas 40% dianggap bahaya

/* Kalibrasi sensor */
const int GAS_MIN = 0;       // Nilai minimum sensor
const int GAS_MAX = 4095;    // Nilai maksimum sensor

/* Objek untuk LCD dan Web Server */
LiquidCrystal_I2C lcd(0x27, 16, 2);
WebServer server(80);

/* Variabel */
int gasValue = 0;
int gasPercentage = 0;
String statusGas = "AMAN";
bool isBuzzerOn = false;

/* Fungsi untuk menghitung persentase gas */
int calculateGasPercentage(int rawValue) {
  return map(rawValue, GAS_MIN, GAS_MAX, 0, 100);
}

/* Fungsi untuk menentukan status gas */
String getGasStatus(int percentage) {
  if (percentage <= AMAN) return "AMAN";
  if (percentage <= WASPADA) return "WASPADA";
  return "BAHAYA";
}

/* Fungsi untuk menangani halaman web utama */
void handle_OnConnect() {
  String html = "<!DOCTYPE html><html>";
  html += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">";
  html += "<title>Gas Detector</title>";
  html += "<style>html { font-family: Helvetica; text-align: center;}";
  html += ".button {padding: 16px; margin: 10px; font-size: 20px; cursor: pointer;}";
  html += ".button-off {background-color: red; color: white;}</style></head>";
  html += "<body>";
  html += "<h1>Gas Detector</h1>";
  html += "<p>Gas Level: <span id=\"gasLevel\">Loading...</span>%</p>";
  html += "<p>Status: <span id=\"gasStatus\">Loading...</span></p>";
  html += "<p><a href=\"/buzzer/off\"><button class=\"button button-off\">Turn Buzzer Off</button></a></p>";
  html += "<script>";
  html += "setInterval(() => {";
  html += "  fetch('/update').then(response => response.json()).then(data => {";
  html += "    document.getElementById('gasLevel').innerText = data.gasLevel;";
  html += "    document.getElementById('gasStatus').innerText = data.gasStatus;";
  html += "  });";
  html += "}, 1000);";  // Update setiap 1 detik
  html += "</script>";
  html += "</body></html>";
  server.send(200, "text/html", html);
}

/* Fungsi untuk menangani data real-time */
void handle_Update() {
  String json = "{\"gasLevel\": \"" + String(gasPercentage) + "\", \"gasStatus\": \"" + statusGas + "\"}";
  server.send(200, "application/json", json);
}

/* Fungsi untuk menangani kontrol buzzer */
void handleBuzzerOff() {
  isBuzzerOn = false;
  digitalWrite(BUZZER_PIN, LOW);
  server.sendHeader("Location", "/");
  server.send(303);
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n\nESP32 Gas Detector Starting...");
  Serial.println("=================================");

  /* Inisialisasi pin */
  pinMode(MQ2_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);

  /* Inisialisasi LCD */
  lcd.init();
  lcd.backlight();
  lcd.print("Gas Detector");

  /* Menampilkan status WiFi */
  Serial.println("Connecting to WiFi...");
  Serial.print("SSID: ");
  Serial.println(ssid);

  /* Koneksi WiFi dengan DHCP */
  WiFi.begin(ssid, password);

  /* Looping sampai terhubung */
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n\nWiFi Connected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP()); // Menampilkan IP Address
  Serial.println("=================================");
  
  /* Konfigurasi Web Server */
  server.on("/", handle_OnConnect);
  server.on("/update", handle_Update);
  server.on("/buzzer/off", handleBuzzerOff);
  server.begin();
  Serial.println("Web Server Started!");
  Serial.println("Access the server at: http://" + WiFi.localIP().toString());
}

void loop() {
  server.handleClient();

  /* Pembacaan sensor gas */
  gasValue = analogRead(MQ2_PIN);
  gasPercentage = calculateGasPercentage(gasValue);
  statusGas = getGasStatus(gasPercentage);

  /* Tampilkan di LCD */
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("LPG: " + String(gasPercentage) + "%");
  lcd.setCursor(0, 1);
  lcd.print("Status: " + statusGas);

  /* Kondisi alarm */
  if (statusGas == "WASPADA" || statusGas == "BAHAYA") {
    digitalWrite(BUZZER_PIN, HIGH);
    digitalWrite(LED_PIN, HIGH);
    isBuzzerOn = true;
  } else {
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
    isBuzzerOn = false;
  }
}
