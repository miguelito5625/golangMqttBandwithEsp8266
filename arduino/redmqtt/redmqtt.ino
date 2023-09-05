#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Declara las variables para el WiFi y MQTT
const char* ssid = "claroExbinario";
const char* password = "a1a2a3a4a5";
const char* mqtt_server = "broker.emqx.io";
const int mqtt_port = 1883;
const char* mqtt_topic = "mike5625/#";
const char* mqtt_client_id = "Esp8266Client";

WiFiClient espclient;
PubSubClient client(mqtt_server, mqtt_port, espclient);

// Variables globales para almacenar los valores de RX y TX
float rxValue = 0.0;
float txValue = 0.0;

// Configuración para la pantalla LCD I2C
LiquidCrystal_I2C lcd(0x27, 16, 2); // Dirección I2C: 0x27, 16 columnas, 2 filas

void callback(char* topic, byte* payload, unsigned int length);

void reconnect() {
  while (!client.connected()) {
    Serial.println("Conectando al MQTT Broker...");
    if (client.connect(mqtt_client_id)) {
      Serial.println("Conectado al MQTT Broker");
      client.subscribe(mqtt_topic);
    } else {
      Serial.print("Fallo en la conexion MQTT, rc=");
      Serial.print(client.state());
      Serial.println(" Intentando nuevamente en 5 segundos...");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Inicializar la pantalla LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Inicializando...");

  // Conectarse al WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Conectando a");
    lcd.setCursor(0, 1);
    lcd.print("a WiFi...");
    Serial.println("Conectando a WiFi...");
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Conectado a");
  lcd.setCursor(0, 1);
  lcd.print("WiFi");
  Serial.println("Conectado a WiFi");

  client.setCallback(callback);

  // Conectarse al MQTT Broker
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Conectado a");
  lcd.setCursor(0, 1);
  lcd.print("MQTT broker");
  Serial.println("Conectando al MQTT Broker...");
  reconnect();
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensaje recibido en el topic: ");
  Serial.println(topic);
  Serial.print("Contenido del mensaje: ");

  // Decodificar el mensaje del payload
  String message = "";
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }

   // Comparar el topic recibido
  if (strcmp(topic, "mike5625/redcasa") == 0) {

    // Extraer los valores de RX y TX del mensaje
    int rxIndex = message.indexOf("rx:");
    int txIndex = message.indexOf("tx:");
    if (rxIndex != -1 && txIndex != -1) {
      String rxString = message.substring(rxIndex + 3, txIndex - 1);
      String txString = message.substring(txIndex + 3);

      // Buscar el índice del espacio en "rx" y "tx" para separar el valor de la unidad
      int rxSpaceIndex = rxString.indexOf(" ");
      int txSpaceIndex = txString.indexOf(" ");
      if (rxSpaceIndex != -1 && txSpaceIndex != -1) {
        rxValue = rxString.substring(0, rxSpaceIndex).toFloat();
        txValue = txString.substring(0, txSpaceIndex).toFloat();

        // Actualizar la pantalla LCD
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("TX:");
        lcd.print(rxValue);
        lcd.print(rxString.substring(rxSpaceIndex));
        lcd.setCursor(0, 1);
        lcd.print("RX:");
        lcd.print(txValue);
        lcd.print(txString.substring(txSpaceIndex));

        // Imprimir en el puerto serie
        Serial.print("TX: ");
        Serial.print(rxValue);
        Serial.println(rxString.substring(rxSpaceIndex));
        Serial.print("RX: ");
        Serial.print(txValue);
        Serial.println(txString.substring(txSpaceIndex));
      }
    }
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Top Ip: ");
    lcd.setCursor(0, 1);
    lcd.print(message);

    Serial.print("Top Ip: ");
    Serial.println(message);
  }
  
}
