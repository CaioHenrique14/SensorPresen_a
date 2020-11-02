
// --- WIFI ---
#include <WiFi.h>
const char* ssid     = "Maria mae de Deus_2.4G";
const char* password = "mater.navita";
WiFiClient esp32Client;

// --- MQTT ---
//Adicionar dados para a conexão
#include <PubSubClient.h>
PubSubClient client(esp32Client);
const char* mqtt_Broker = "";
const char* topico = "vaga";
const char* mqtt_ClientID = "mqtt-explorer-04b20d03";
const char* mqtt_user = "root";
const char* mqtt_pass = "key";

// --- Bluetooth ---
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
int scanTime = 5; 
int nivelRSSI = -60; 
String dispositivosAutorizados = "c2:6b:f6:62:fa:3f";
bool dispositivoPresente = false;
unsigned long ultimoTempoMedido = 0;
const long intervaloPublicacao = 20000;


// --- Setup ---
void setup() {
  Serial.begin(115200);
  conectarWifi();
  client.setServer(mqtt_Broker, 1883);
  Serial.println("Scanning...");
  BLEDevice::init("");
  
}

// --- Loop ---
void loop() {
  if (!client.connected()) {
    conectarMQTT();
  }
  scanBLE();
  publicarStatusnoTopico();
  delay(2000);
}

// -- Funções Auxiliares ---
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      String dispositivosEncontrados = advertisedDevice.getAddress().toString().c_str();
      if ( dispositivosEncontrados == dispositivosAutorizados  && advertisedDevice.getRSSI() > nivelRSSI) {
        Serial.println("Identificador DETECTADO!");
        Serial.print("RSSI: ");
        Serial.println(advertisedDevice.getRSSI());
        dispositivoPresente = true;
        ultimoTempoMedido = millis();
      }
      else{
        dispositivoPresente = false;
      }
    }
};

// --- Scan Bluetooth LE ---
void scanBLE(){
  BLEScan* pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  BLEScanResults foundDevices = pBLEScan->start(scanTime);
}

// --- Conecta ao WIFI ---
void conectarWifi(){
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

// --- Conecta ao MQTT ---
void conectarMQTT() {
  while (!client.connected()) {
    client.connect(mqtt_ClientID, mqtt_user, mqtt_pass);
  }
}

void publicarStatusnoTopico(){
  if(dispositivoPresente == 1){
    client.publish(topico, String("on").c_str(), true);
    Serial.println("Power ON");
  }
  else {
    if (millis() - ultimoTempoMedido > intervaloPublicacao) {
      client.publish(topico, String("off").c_str(), true);
      Serial.println("Power OFF");
    }
  }
}
