#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define SERVICE_UUID "0000ffe0-0000-1000-8000-00805f9b34fb"
#define CHARACTERISTIC_UUID "0000ffe1-0000-1000-8000-00805f9b34fb"

const int RELAY_CTRL_PIN = 26; // Relay control
const int MOISTURE_PIN = 36;   // Soil sensor

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

class ServerCallback : public BLEServerCallbacks
{
  void onConnect(BLEServer *pServer)
  {
    deviceConnected = true;
    Serial.println("Device connected");
  }

  void onDisconnect(BLEServer *pServer)
  {
    deviceConnected = false;
    Serial.println("Device disconnected");
  }
};

void setup()
{
  pinMode(RELAY_CTRL_PIN, OUTPUT);
  Serial.begin(115200);
  Serial.println("Starting motor and moisture readings...");

  BLEDevice::init("ESP32-BLE");

  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new ServerCallback());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
      CHARACTERISTIC_UUID,
      BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_NOTIFY);

  pCharacteristic->setValue("Initial Hello from ESP32");
  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->start();

  Serial.println("BLE advertising started...");
}

void loop()
{
  // Turn ON the pump
  digitalWrite(RELAY_CTRL_PIN, HIGH); // Transistor ON → Relay ON → Pump ON
  Serial.println("Pump ON");

  // Read moisture
  int moistureValue = analogRead(MOISTURE_PIN); // 0 to 4095
  Serial.print("Moisture analog value: ");
  Serial.println(moistureValue);

  delay(2000); // Wait 2 seconds

  // Turn OFF the pump
  digitalWrite(RELAY_CTRL_PIN, LOW); // Transistor OFF → Relay OFF → Pump OFF
  Serial.println("Pump OFF");

  // Read moisture again
  moistureValue = analogRead(MOISTURE_PIN);
  Serial.print("Moisture analog value: ");
  Serial.println(moistureValue);

  if (deviceConnected)
  {
    static unsigned long lastSent = 0;
    if (millis() - lastSent > 2000)
    {
      String message = "Hello at " + String(millis() / 1000) + "s";
      pCharacteristic->setValue(message.c_str());
      pCharacteristic->notify();
      Serial.println("Sent: " + message);
      lastSent = millis();
    }
  }

  delay(2000); // Wait 2 seconds
}
