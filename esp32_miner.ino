/* * ============================================
 * LightWave Blockchain (LWC) - ESP32 Miner
 * Version 2.5 - HTTPS & Dashboard Optimized
 * ============================================
 */

#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "mbedtls/sha256.h"

// ==========================================
// CONFIGURATION
// ==========================================
const char* WIFI_SSID       = "wifi";
const char* WIFI_PASSWORD   = "pass";

const char* SERVER_HOST     = "miner.greennovachain.site"; 
const int   SERVER_PORT     = 443; 

// Your wallet address
const char* WALLET_ADDRESS  = "LWC_eaa13dcbd3fa10d6480e764cf105327f0652b47d";
// ==========================================

// Global Variables
String deviceId = "";
int workBlockIndex = 0;
String workPreviousHash = "";
long workTimestamp = 0;
int workDifficulty = 2;
float workReward = 0;
String workTransactions = "[]";

unsigned long totalHashes = 0;
unsigned long hashRate = 0;
unsigned long hashesThisSecond = 0;
unsigned long lastHashRateTime = 0;
unsigned long blocksFound = 0;
float totalRewards = 0;
unsigned long lastHeartbeat = 0;
bool hasWork = false;

// SHA-256 Function
String sha256(String data) {
    uint8_t hash[32];
    mbedtls_sha256_context ctx;
    mbedtls_sha256_init(&ctx);
    mbedtls_sha256_starts(&ctx, 0);
    mbedtls_sha256_update(&ctx, (const unsigned char*)data.c_str(), data.length());
    mbedtls_sha256_finish(&ctx, hash);
    mbedtls_sha256_free(&ctx);

    char hexStr[65];
    for (int i = 0; i < 32; i++) {
        sprintf(hexStr + (i * 2), "%02x", hash[i]);
    }
    hexStr[64] = '\0';
    return String(hexStr);
}

// Calculate Block Hash
String calculateBlockHash(int idx, long ts, String prevHash, String txs, unsigned long nonce, int diff) {
    String data = String(idx) + String(ts) + prevHash + txs + String(nonce) + String(diff);
    return sha256(data);
}

bool meetsTarget(String hash, int difficulty) {
    for (int i = 0; i < difficulty; i++) {
        if (hash.charAt(i) != '0') return false;
    }
    return true;
}

// HTTPS Helper
String buildURL(String path) {
    return "https://" + String(SERVER_HOST) + path;
}

// WiFi Connection
void connectWiFi() {
    Serial.printf("\n[WIFI] Connecting to: %s\n", WIFI_SSID);
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\n[WIFI] Connected! IP: " + WiFi.localIP().toString());
}

// Get Mining Job
bool getWork() {
    WiFiClientSecure client;
    client.setInsecure(); // Required for Cloudflare
    HTTPClient http;
    
    String url = buildURL("/api/mining/getwork?wallet=" + String(WALLET_ADDRESS) + "&deviceId=" + deviceId);
    
    http.begin(client, url);
    int code = http.GET();

    if (code == 200) {
        String response = http.getString();
        DynamicJsonDocument doc(8192);
        deserializeJson(doc, response);

        if (doc["success"]) {
            JsonObject data = doc["data"];
            workBlockIndex = data["blockIndex"];
            workPreviousHash = data["previousHash"].as<String>();
            workTimestamp = data["timestamp"];
            workDifficulty = data["difficulty"];
            workReward = data["reward"];
            
            // Handle Transactions
            if (data.containsKey("transactions")) {
                serializeJson(data["transactions"], workTransactions);
            } else {
                workTransactions = "[]";
            }
            
            http.end();
            return true;
        }
    }
    http.end();
    return false;
}

// Submit Block
bool submitBlock(unsigned long nonce, String hash) {
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    
    String url = buildURL("/api/mining/submit");
    http.begin(client, url);
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<1024> doc;
    doc["walletAddress"] = WALLET_ADDRESS;
    doc["nonce"] = nonce;
    doc["hash"] = hash;
    doc["deviceId"] = deviceId;
    doc["hashRate"] = hashRate;

    String body;
    serializeJson(doc, body);

    int code = http.POST(body);
    if (code == 200) {
        String res = http.getString();
        Serial.println("[SUCCESS] Block Accepted: " + res);
        blocksFound++;
        http.end();
        return true;
    }
    
    Serial.printf("[FAILED] Submission Error: %d\n", code);
    http.end();
    return false;
}

// Heartbeat to Dashboard
void sendHeartbeat() {
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    
    String url = buildURL("/api/mining/heartbeat");
    http.begin(client, url);
    http.addHeader("Content-Type", "application/json");

    StaticJsonDocument<512> doc;
    doc["deviceId"] = deviceId;
    doc["hashRate"] = hashRate;
    doc["status"] = "online";
    doc["totalHashes"] = totalHashes;

    String body;
    serializeJson(doc, body);
    http.POST(body);
    http.end();
    
    Serial.printf("[HB] %lu H/s | Found: %lu\n", hashRate, blocksFound);
}

void setup() {
    Serial.begin(115200);
    
    // Generate Unique ID
    uint64_t chipid = ESP.getEfuseMac();
    deviceId = "ESP32_" + String((uint32_t)(chipid >> 32), HEX) + String((uint32_t)chipid, HEX);
    
    connectWiFi();
    
    Serial.println("LWC Miner v2.5 Started. ID: " + deviceId);
}

void loop() {
    if (WiFi.status() != WL_CONNECTED) connectWiFi();

    // Heartbeat every 30 seconds for Admin Dashboard
    if (millis() - lastHeartbeat >= 30000) {
        sendHeartbeat();
        lastHeartbeat = millis();
    }

    // Get Work and Mine
    if (getWork()) {
        Serial.printf("[MINE] Block #%d | Diff: %d\n", workBlockIndex, workDifficulty);
        
        unsigned long startTime = millis();
        hashesThisSecond = 0;
        
        // Mine for 5 seconds then update stats
        while (millis() - startTime < 5000) {
            unsigned long nonce = esp_random();
            String hash = calculateBlockHash(workBlockIndex, workTimestamp, workPreviousHash, workTransactions, nonce, workDifficulty);
            
            totalHashes++;
            hashesThisSecond++;
            
            if (meetsTarget(hash, workDifficulty)) {
                submitBlock(nonce, hash);
                break; 
            }
            
            if (totalHashes % 100 == 0) yield();
        }
        
        hashRate = hashesThisSecond / 5; // Average over 5 seconds
    } else {
        delay(5000); // Wait if server busy
    }
}
