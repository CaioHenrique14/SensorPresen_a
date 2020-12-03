#include "DHT.h"
#include <WiFi.h>
const char* ssid = "Maria mae de Deus_2.4G";
const char* password = "mater.navita";
WiFiClient esp32Client;
#include <ArduinoJson.h>
#include <PubSubClient.h>
PubSubClient client(esp32Client);
const char* mqtt_Broker = "192.168.100.18";
const char* topico = "dht11";
const char* mqtt_ClientID = "mqtt-explorer-657f05f9";
const char* mqtt_user = "root";
const char* mqtt_pass = "senha";

const int pinoSensor = 35; //PINO UTILIZADO PELO SENSOR
int valorLido; //VARIÁVEL QUE ARMAZENA O PERCENTUAL DE UMIDADE DO SOLO
 
int analogSoloSeco = 400;
int analogSoloMolhado = 150;
int percSoloSeco = 0;
int percSoloMolhado = 100;
 

const int pino_dht = 19; // pino onde o sensor DHT está conectado
float temperatura; // variável para armazenar o valor de temperatura
float umidade; // variável para armazenar o valor de umidade
DHT dht(pino_dht, DHT11); // define o pino e o tipo de DHT

unsigned long ultimoTempoMedido = 0;
const long intervaloPublicacao = 20000; //Ajustar o tempo de desligamento

void setup() {
  // Inicia e configura a Serial
  Serial.begin(115200); // 9600bps
  conectarWifi();
  client.setServer("192.168.100.18", 1883);
  dht.begin(); // inicializa o sensor DHT
}

void loop() {
  // Aguarda alguns segundos entre uma leitura e outra
  delay(2000); // 2 segundos (Datasheet)
  
  if (!client.connected()) {
    conectarMQTT();
  }
  // A leitura da temperatura ou umidade pode levar 250ms
  // O atraso do sensor pode chegar a 2 segundos
  temperatura = dht.readTemperature(); // lê a temperatura em Celsius
  umidade = dht.readHumidity(); // lê a umidade
  valorLido = constrain(analogRead(pinoSensor), analogSoloMolhado, analogSoloSeco);
  valorLido = map(valorLido,analogSoloMolhado,analogSoloSeco,percSoloMolhado,percSoloSeco);

  // Se ocorreu alguma falha durante a leitura
  if (isnan(umidade) || isnan(temperatura)) {
    Serial.println("Falha na leitura do Sensor DHT!");
  }
  else { // Se não
    // Imprime o valor de temperatura
    Serial.print("Temperatura: ");
    Serial.print(temperatura);
    Serial.print(" *C ");

    Serial.print("\t"); // tabulação

    // Imprime o valor de umidade
    Serial.print("Umidade: ");
    Serial.print(umidade);
    Serial.print(" %\t");

    Serial.print("Umidade do solo: ");
    Serial.print(valorLido);
    Serial.print(" %\t");

    Serial.println(); // nova linha
  }
  publicarStatusnoTopico();
}

// --- Conecta ao WIFI ---
void conectarWifi() {
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

void publicarStatusnoTopico() {
  StaticJsonDocument<200> doc;
  String sensor = "";
  doc["umidity"] = umidade;
  doc["temperature"] = temperatura;
  doc["soilMisture"] = valorLido;
  serializeJson(doc, sensor);
  client.publish(topico, sensor.c_str(), true);
  Serial.println(sensor);
}
