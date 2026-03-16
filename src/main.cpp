#include <Adafruit_MAX31855.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include "config.h"

// --- WiFi credentials ---
const char* ssid     = WIFI_SSID;
const char* password = WIFI_PASSWORD;

// --- MQTT Broker ---
const char* mqtt_server = "192.168.1.100";
const int   mqtt_port   = 1883;
const char* mqtt_topic  = "sensors/esp32";

// ----------------------------
// MAX31855 Pins (shared SPI)
// ----------------------------
int thermoDO   = 19;   // MISO
int thermoCLK  = 18;   // SCK
int thermoMO  = 23;  // MOSI (unused by MAX31855 but required by SPI.begin())

// Individual chip-select pins
int thermoCS1  = 5;
int thermoCS2  = 17;
int thermoCS3  = 16;
int thermoCS4  = 4;

// 4 MAX31855 sensors (hardware SPI)
Adafruit_MAX31855 tc1(thermoCS1);
Adafruit_MAX31855 tc2(thermoCS2);
Adafruit_MAX31855 tc3(thermoCS3);
Adafruit_MAX31855 tc4(thermoCS4);

// ----------------------------
// ESP32 Internal Temperature
// ----------------------------
extern "C" {
  uint8_t temprature_sens_read();
}

float readEsp32InternalTempC() {
  return (temprature_sens_read() - 32) / 1.8;
}

// ----------------------------
// WiFi + MQTT Clients
// ----------------------------
WiFiClient espClient;
PubSubClient client(espClient);

// ----------------------------
// Connect to WiFi
// ----------------------------
void setup_wifi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("\nWiFi connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

// ----------------------------
// Reconnect to MQTT
// ----------------------------
void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT... ");

    if (client.connect("ESP32_TempClient")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" retrying in 3 seconds");
      delay(3000);
    }
  }
}

// ----------------------------
// Convert fault bits to text
// ----------------------------
String faultToString(int fault) {
  if (fault == 0) return "none";

  String s = "";
  if (fault & 0x01) s += "open_thermocouple ";
  if (fault & 0x02) s += "short_to_gnd ";
  if (fault & 0x04) s += "short_to_vcc ";
  return s;
}

// ----------------------------
// Setup
// ----------------------------
void setup() {
  Serial.begin(115200);
  delay(1000);

  // SPI for MAX31855 (hardware SPI)
  SPI.begin(thermoCLK, thermoDO, thermoMO);

  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);

  Serial.println("MAX31855 sensors ready (Adafruit, hardware SPI)");
}


// ----------------------------
// Main Loop
// ----------------------------
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Read 4 thermocouples
  double t1_ext = tc1.readCelsius();
  double t1_cj  = tc1.readInternal();
  int    t1_err = tc1.readError();

  double t2_ext = tc2.readCelsius();
  double t2_cj  = tc2.readInternal();
  int    t2_err = tc2.readError();

  double t3_ext = tc3.readCelsius();
  double t3_cj  = tc3.readInternal();
  int    t3_err = tc3.readError();

  double t4_ext = tc4.readCelsius();
  double t4_cj  = tc4.readInternal();
  int    t4_err = tc4.readError();

  // ESP32 internal temperature
  float espC = readEsp32InternalTempC();

  // Build JSON payload
  char payload[1000];
  snprintf(payload, sizeof(payload),
    "{"
      "\"t1_external\": %.2f, \"t1_cold_junction\": %.2f, \"t1_fault\": \"%s\", "
      "\"t2_external\": %.2f, \"t2_cold_junction\": %.2f, \"t2_fault\": \"%s\", "
      "\"t3_external\": %.2f, \"t3_cold_junction\": %.2f, \"t3_fault\": \"%s\", "
      "\"t4_external\": %.2f, \"t4_cold_junction\": %.2f, \"t4_fault\": \"%s\", "
      "\"esp32_internal\": %.2f"
    "}",
    t1_ext, t1_cj, faultToString(t1_err).c_str(),
    t2_ext, t2_cj, faultToString(t2_err).c_str(),
    t3_ext, t3_cj, faultToString(t3_err).c_str(),
    t4_ext, t4_cj, faultToString(t4_err).c_str(),
    espC
  );

  bool ok = client.publish(mqtt_topic, payload);
  if (!ok) {
    Serial.println("MQTT Publish FAILED");
  }

  Serial.print("MQTT Payload: ");
  Serial.println(payload);


  delay(2000);
}
