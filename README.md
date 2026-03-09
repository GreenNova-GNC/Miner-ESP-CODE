# LightWave Blockchain ESP32 Miner

LightWave Blockchain (LWC) is a lightweight blockchain designed for IoT devices.

This project provides a **fully functional ESP32 miner** capable of performing SHA-256 proof-of-work mining and communicating with the LightWave mining server.

---

## Features

• ESP32 based mining  
• SHA256 Proof-of-Work  
• Secure HTTPS mining API  
• Automatic block submission  
• Dashboard heartbeat reporting  
• Unique device ID generation  
• Low-power mining

---

## Blockchain Specifications

| Parameter | Value |
|----------|------|
| Block Time | 1 minute |
| Algorithm | SHA256 |
| Total Supply | 15,000,000 LWC |
| Mining Hardware | ESP32 |
| Network | HTTPS API |

---

## Hardware Requirements

Supported boards:

- ESP32 DevKit V1
- ESP32 WROOM
- ESP32 NodeMCU

Required:

• ESP32 board  
• USB cable  
• WiFi internet connection  

---

## Software Requirements

Install:

- Arduino IDE
- ESP32 Board Package

Libraries:

```
WiFi.h
HTTPClient.h
WiFiClientSecure.h
ArduinoJson
mbedtls
```

Install **ArduinoJson** using the Arduino Library Manager.

---

## Miner Configuration

Edit the following variables in the code.

```cpp
const char* WIFI_SSID       = "YOUR_WIFI";
const char* WIFI_PASSWORD   = "YOUR_PASSWORD";

const char* SERVER_HOST     = "miner.greennovachain.site";

const char* WALLET_ADDRESS  = "YOUR_WALLET";
```

---

## Uploading the Miner

1. Open Arduino IDE
2. Select board **ESP32 Dev Module**
3. Connect ESP32 via USB
4. Upload `esp32-miner.ino`

After upload the miner will automatically start.

---

## Mining Workflow

The miner performs the following steps:

1. Connect to WiFi
2. Request mining work from server
3. Calculate SHA256 hashes
4. Check difficulty target
5. Submit valid block
6. Send heartbeat every 30 seconds

---

## Mining API

### Get Work

```
GET /api/mining/getwork
```

Example:

```
/api/mining/getwork?wallet=WALLET&deviceId=DEVICE
```

---

### Submit Block

```
POST /api/mining/submit
```

Example:

```json
{
 "walletAddress":"LWC_wallet",
 "nonce":123456,
 "hash":"000abc...",
 "deviceId":"ESP32_ABC"
}
```

---

### Heartbeat

```
POST /api/mining/heartbeat
```

Sent every **30 seconds**.

---

## Device ID

Each miner generates a unique ID based on the ESP32 chip MAC.

Example:

```
ESP32_4a3bc12f9a
```

---

## Performance

Typical performance:

| Difficulty | Hashrate |
|------------|-----------|
| 1 | ~600 H/s |
| 2 | ~400 H/s |
| 3 | ~250 H/s |

---

## Power Usage

Approximate power usage:

```
~0.6W
```

This allows large **distributed IoT mining networks**.

---

## Documentation

Full documentation is available in the `/docs` folder.

---

## License

MIT License

---

## Maintainer

Project maintained by:

**GreenNova Energy Chain**

---

## Website

https://miner.greennovachain.site
