#include <WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <BluetoothSerial.h>
#include "time.h"

#define SS_PIN 5
#define RST_PIN 22
#define LED_BT_STATUS 2  // LED indicador de status BT

const char* ssid = "OFF";
const char* password = "foidebase";

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -3 * 3600;
const int daylightOffset_sec = 0;

const char *deviceName = "ESP32-RFID";

const byte uidRondyson[] = {0x63, 0x80, 0xEA, 0x2C};
const byte uidSete[]     = {0x92, 0x1D, 0xB5, 0x02};

BluetoothSerial SerialBT;
MFRC522 rfid(SS_PIN, RST_PIN);

bool btConnected = false;
unsigned long lastBTCheck = 0;
const unsigned long BT_CHECK_INTERVAL = 2000;

bool compareUID(byte *uid, const byte *knownUID, byte size) {
  for (byte i = 0; i < size; i++) {
    if (uid[i] != knownUID[i]) return false;
  }
  return true;
}

String getTimestamp() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return "1970-01-01 00:00:00";
  char buf[20];
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(buf);
}

void verificarConexaoBluetooth() {
  bool conectado = SerialBT.hasClient();
  if (conectado != btConnected) {
    btConnected = conectado;
    digitalWrite(LED_BT_STATUS, btConnected ? HIGH : LOW);
    Serial.println(btConnected ? "✅ Cliente Bluetooth CONECTADO" : "❌ Cliente Bluetooth DESCONECTADO");
  }
}

void sincronizarDataHora() {
  Serial.print("🌐 Conectando ao WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  unsigned long startAttemptTime = millis();

  // Aguarda conexão WiFi (timeout 10s)
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    delay(50);
    Serial.print(".");
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\n❌ Falha ao conectar no WiFi para sincronizar horário");
    return;
  }
  Serial.println("\n✅ WiFi conectado!");

  // Sincroniza NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    Serial.printf("🕒 Data/hora sincronizada: %02d/%02d/%04d %02d:%02d:%02d\n",
      timeinfo.tm_mday, timeinfo.tm_mon+1, timeinfo.tm_year+1900,
      timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
  } else {
    Serial.println("⚠ Falha ao obter hora via NTP");
  }
  WiFi.disconnect(true); // Desconecta do WiFi para economizar energia
}

void setup() {
  Serial.begin(115200);
  SPI.begin();
  rfid.PCD_Init();
  pinMode(LED_BT_STATUS, OUTPUT);
  digitalWrite(LED_BT_STATUS, LOW);

  sincronizarDataHora();         // 1. WiFi e NTP
  SerialBT.begin(deviceName);    // 2. Bluetooth só depois do NTP
  Serial.println("🔵 Bluetooth iniciado como \"" + String(deviceName) + "\".");
}

void loop() {
  if (millis() - lastBTCheck > BT_CHECK_INTERVAL) {
    verificarConexaoBluetooth();
    lastBTCheck = millis();
  }

  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;

  String uidStr = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) uidStr += "0";
    uidStr += String(rfid.uid.uidByte[i], HEX);
  }
  uidStr.toUpperCase();

  Serial.print("🔍 Cartão detectado: ");
  Serial.println(uidStr);

  String nome = "Intruso";
  String status = "negado";

  if (compareUID(rfid.uid.uidByte, uidRondyson, 4)) {
    nome = "Rondyson";
    status = "autorizado";
  } else if (compareUID(rfid.uid.uidByte, uidSete, 4)) {
    nome = "Sete";
    status = "autorizado";
  }

  String timestamp = getTimestamp();

  String msg = "{";
  msg += "\"uid\":\"" + uidStr + "\",";
  msg += "\"nome\":\"" + nome + "\",";
  msg += "\"status\":\"" + status + "\",";
  msg += "\"hora\":\"" + timestamp + "\"";
  msg += "}";

  Serial.print("📤 Enviando via Bluetooth: ");
  Serial.println(msg);

  if (SerialBT.hasClient()) {
    SerialBT.println(msg);
    Serial.println("✅ Mensagem enviada com sucesso!");
  } else {
    Serial.println("⚠ Nenhum cliente conectado.");
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  delay(100);
}
