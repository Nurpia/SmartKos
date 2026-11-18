
#include <WiFi.h>
#include <DHT.h>
#include <ESP32Servo.h>

// ================= WIFI =================
const char* ssid = "MR-FK-01";
const char* password = "adzanmasjid";

// ================= PIN ESP32 =================
#define DHT_PIN 15
#define DHT_TYPE DHT22

#define LDR_PIN 34
#define LED_PIN 2
#define SERVO_PIN 18

// ================= PENGATURAN =================
#define LIGHT_THRESHOLD 2000

#define HUMIDITY_OPEN 70.0
#define HUMIDITY_CLOSE 65.0

#define SERVO_OPEN 90
#define SERVO_CLOSE 0

// ================= OBJECT =================
DHT dht(DHT_PIN, DHT_TYPE);
Servo windowServo;

// ================= VARIABLE =================
int lightValue = 0;

float temperature = 0;
float humidity = 0;

bool windowIsOpen = false;
bool lampIsOn = false;

unsigned long lastRead = 0;
unsigned long lastWiFiCheck = 0;

const unsigned long READ_INTERVAL = 2500;
const unsigned long WIFI_CHECK_INTERVAL = 5000;

// ================= WIFI =================
void connectWiFi() {

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  Serial.print("Menghubungkan WiFi");

  unsigned long startTime = millis();

  while (WiFi.status() != WL_CONNECTED &&
         millis() - startTime < 15000) {

    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {

    Serial.println("\nWiFi berhasil terhubung!");

    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

  } else {

    Serial.println("\nWiFi gagal terhubung.");
    Serial.println("Sistem tetap berjalan secara lokal.");
  }
}

// ================= RECONNECT WIFI =================
void reconnectWiFi() {

  if (WiFi.status() != WL_CONNECTED) {

    Serial.println("WiFi terputus!");
    Serial.println("Mencoba menghubungkan kembali...");

    WiFi.disconnect();
    WiFi.begin(ssid, password);

  } else {

    // WiFi masih terhubung
  }
}

// ================= KONTROL LAMPU =================
void controlLamp(int value) {

  // LDR rendah = terang
  // LDR tinggi = gelap

  if (value > LIGHT_THRESHOLD) {

    digitalWrite(LED_PIN, HIGH);
    lampIsOn = true;

  } else {

    digitalWrite(LED_PIN, LOW);
    lampIsOn = false;
  }
}

// ================= KONTROL JENDELA =================
void controlWindow(float hum) {

  if (!windowIsOpen && hum >= HUMIDITY_OPEN) {

    windowServo.write(SERVO_OPEN);
    windowIsOpen = true;

    Serial.println("JENDELA TERBUKA");

  } else if (windowIsOpen && hum <= HUMIDITY_CLOSE) {

    windowServo.write(SERVO_CLOSE);
    windowIsOpen = false;

    Serial.println("JENDELA TERTUTUP");
  }
}

// ================= SETUP =================
void setup() {

  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  analogReadResolution(12);

  dht.begin();

  windowServo.setPeriodHertz(50);
  windowServo.attach(SERVO_PIN, 500, 2400);

  windowServo.write(SERVO_CLOSE);

  Serial.println("\n===== SMART KOS SYSTEM =====");

  connectWiFi();

  Serial.println("Sistem siap bekerja!");
}

// ================= LOOP =================
void loop() {

  // Memeriksa koneksi WiFi
  if (millis() - lastWiFiCheck >= WIFI_CHECK_INTERVAL) {

    lastWiFiCheck = millis();

    reconnectWiFi();
  }

  // Pembacaan sensor
  if (millis() - lastRead >= READ_INTERVAL) {

    lastRead = millis();

    // Membaca sensor LDR
    lightValue = analogRead(LDR_PIN);

    // Membaca sensor DHT22
    temperature = dht.readTemperature();
    humidity = dht.readHumidity();

    // Kontrol lampu
    controlLamp(lightValue);

    Serial.println("\n===== SMART KOS SYSTEM =====");

    // Informasi WiFi
    Serial.print("Status WiFi   : ");

    if (WiFi.status() == WL_CONNECTED) {

      Serial.println("TERHUBUNG");

      Serial.print("IP Address    : ");
      Serial.println(WiFi.localIP());

    } else {

      Serial.println("TERPUTUS");
    }

    // Informasi LDR
    Serial.print("Nilai LDR     : ");
    Serial.println(lightValue);

    Serial.print("Status Lampu  : ");
    Serial.println(lampIsOn ? "MENYALA" : "MATI");

    // Pemeriksaan DHT22
    if (isnan(temperature) || isnan(humidity)) {

      Serial.println("DHT22 gagal dibaca!");

    } else {

      Serial.print("Suhu          : ");
      Serial.print(temperature);
      Serial.println(" C");

      Serial.print("Kelembapan    : ");
      Serial.print(humidity);
      Serial.println(" %");

      // Kontrol jendela
      controlWindow(humidity);

      Serial.print("Status Jendela: ");

      Serial.println(
        windowIsOpen ? "TERBUKA" : "TERTUTUP"
      );
    }

    Serial.println("============================");
  }
}
