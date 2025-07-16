# 📲 ESP32 + RFID + Bluetooth: Controle de Acesso com Notificação para App Android

Este projeto implementa um sistema de **controle de acesso RFID** utilizando o **ESP32**, que autentica usuários por meio de tags RFID e envia notificações com nome, UID e hora atual via **Bluetooth clássico (SPP)** para um **aplicativo Android**. A sincronização de data e hora é feita por NTP via Wi-Fi.  

O arquivo rfid.zip é referente a interface mobile.
---

## 📦 Requisitos

### Hardware
- ESP32 DevKit V1
- Leitor RFID RC522 (com conexão SPI)
- LED (indicador de conexão Bluetooth)
- Fonte USB 5V
- Jumpers e Protoboard

### Bibliotecas (Arduino IDE)
- `WiFi.h`
- `SPI.h`
- `MFRC522.h`
- `BluetoothSerial.h`
- `time.h`

---

## 🔌 Pinagem

| Componente  | ESP32 Pin |
|-------------|------------|
| SS (SDA)     | GPIO 5     |
| RST         | GPIO 22    |
| MOSI        | GPIO 23    |
| MISO        | GPIO 19    |
| SCK         | GPIO 18    |
| LED         | GPIO 2     |

---

## ⚙️ Funcionalidades

- Leitura de tags RFID
- Autenticação de UID com dois usuários:
  - Rondyson: `63 80 EA 2C`
  - Sete: `92 1D B5 02`
- Sincronização de hora via NTP
- Envio de dados via Bluetooth SPP no formato **JSON**
- LED indica status da conexão Bluetooth

---

## 📤 Formato da Mensagem Enviada

```json
{
  "uid": "6380EA2C",
  "nome": "Rondyson",
  "status": "autorizado",
  "hora": "2025-07-16 15:23:01"
}
