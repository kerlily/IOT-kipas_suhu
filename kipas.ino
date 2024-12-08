#define BLYNK_TEMPLATE_ID "TMPL6S4W3YSBz"
#define BLYNK_TEMPLATE_NAME "KIPAS PINTAR"
#define BLYNK_AUTH_TOKEN "UanKN2ER4PgUL5PXG6UClIdfwx2eSsft"

#define BLYNK_PRINT Serial // Menggunakan serial Blynk

#include <WiFi.h> // Library WiFi
#include <WiFiClient.h> // Library WiFiClient
#include <BlynkSimpleEsp32.h> // Library Blynk ESP32

char ssid[] = "Wokwi-GUEST"; // Nama WiFi
char pass[] = ""; // Password WiFi

BlynkTimer timer; // Timer untuk mengirim data

// SENSOR DHT
#include <DHTesp.h> // Library sensor DHT
const int DHT_PIN = 15; // Pin sinyal DHT
byte relayPin = 4; // Pin relay pada ESP32
DHTesp dhtSensor; // Membuat objek sensor DHT
float suhu, kelembaban; // Variabel untuk suhu dan kelembaban

bool manualControl = false; // Status kontrol manual
bool buttonLocked = false; // Status penguncian tombol
int relayState = LOW; // Status awal relay

void setup()
{
  Serial.begin(115200); // Inisialisasi serial monitor

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass); // Inisialisasi Blynk

  dhtSensor.setup(DHT_PIN, DHTesp::DHT22); // Mendefinisikan sensor DHT
  timer.setInterval(1000L, myTimerEvent); // Mengirim data setiap detik

  pinMode(relayPin, OUTPUT); // Mengatur pin relay sebagai output
  digitalWrite(relayPin, relayState); // Mengatur status awal relay
}

void myTimerEvent()
{
  TempAndHumidity data = dhtSensor.getTempAndHumidity(); // Mendapatkan data
  suhu = data.temperature; // Mendefinisikan variabel suhu
  kelembaban = data.humidity; // Mendefinisikan variabel kelembaban
  Serial.println("Suhu: " + String(suhu, 1) + "C");
  Serial.println("Kelembaban: " + String(kelembaban, 1) + "%");

  // NOTIFIKASI KODE
  if (suhu > 35) // Jika suhu melebihi 35
  {
    Serial.println("Suhu Naik");
    Blynk.logEvent("high_temp"); // Blynk mengirim notifikasi ke Gmail
  }

  // KONTROL RELAY BERDASARKAN SUHU
  if (suhu >= 30) // Jika suhu 30 atau lebih
  {
    relayState = HIGH; // Menyalakan relay
    manualControl = false; // Menonaktifkan kontrol manual saat suhu naik
    Blynk.virtualWrite(V2, 1); // Mengubah saklar Blynk ke posisi ON
  }
  else if (suhu < 30) // Jika suhu di bawah 30
  {
    if (!manualControl) // Hanya matikan relay jika kontrol manual tidak aktif
    {
      relayState = LOW; // Mematikan relay
      Blynk.virtualWrite(V2, 0); // Mengubah saklar Blynk ke posisi OFF
    }
  }

  // Memperbarui status relay
  digitalWrite(relayPin, relayState); 
  // Mengirim data ke Blynk
  Blynk.virtualWrite(V0, suhu); // Mengirim data suhu ke pin V0 di Blynk
  Blynk.virtualWrite(V1, kelembaban); // Mengirim data kelembaban ke pin V1
}

BLYNK_WRITE(V2) // V2 untuk kontrol manual dari Blynk
{
  int pinValue = param.asInt(); // Membaca nilai Blynk

  // Logika kontrol manual
  if (pinValue == 1 && suhu < 30) // Jika suhu < 30 dan saklar diaktifkan secara manual
  {
    relayState = HIGH; // Menyalakan relay
    manualControl = true; // Mengaktifkan kontrol manual
  }
  else if (pinValue == 0 && suhu < 30) // Jika suhu < 30 dan saklar dimatikan secara manual
  {
    relayState = LOW; // Mematikan relay
    manualControl = false; // Menonaktifkan kontrol manual
  }

  digitalWrite(relayPin, relayState); // Memperbarui status relay
  Serial.print("Nilai Switch V2 adalah: ");
  Serial.println(pinValue);
}

void loop()
{
  Blynk.run(); // Menjalankan Blynk
  timer.run(); // Menjalankan timer
}
