#include <WiFi.h>
#include <PubSubClient.h>

#include <Arduino.h>
#include <DHT11.h>
#include <ZMPT101B.h>

#define SENSITIVITY 500.0f
ZMPT101B voltageSensor(26, 50.0);

DHT11 dht11(27);


const char* ssid = "o";
const char* password = "246813579";
const char* mqtt_server = "";//url del broker
const char* mqtt_user = "julio"; 
const char* mqtt_password = "12345678"; 

WiFiClient espClient;
PubSubClient client(espClient);

//conexion a la red wifi
void setup_wifi() {
    delay(10);
    Serial.println();
    Serial.print("Conectando a ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("Conectado a la red WiFi");
    Serial.println("Dirección IP: ");
    Serial.println(WiFi.localIP());
}

//conexion del broker
void reconnect() {
    while (!client.connected()) {
        Serial.print("Intentando conexión MQTT...");
        if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
            Serial.println("Conectado");
        } else {
            Serial.print("falló, rc=");
            Serial.print(client.state());
            Serial.println(" Intentando de nuevo en 5 segundos");
            delay(5000);
        }
    }
}
//funcion para la publicacion de los datos a broker
void publishData(float voltage, float Irms, int temperature) {

    if (client.connected()) {
        Serial.print("Voltage: ");
        Serial.println(voltage);
        Serial.print("V, Corriente: ");
        Serial.print(Irms, 3);
        Serial.print("A, Temperatura: ");
        Serial.println(temperature);
        Serial.print("C ");

        char temp[8];
        char corr[8];
        char volt[8];
//conversion a valores que se puedan enviar 
        itoa(temperature, temp, 10); 
        itoa(Irms, corr, 10); 
        itoa(voltage, volt, 10);
//poblucacion a las colas del broker
        client.publish("casa/temperatura", temp);
        client.publish("casa/corriente", corr);
        client.publish("casa/voltaje", volt);
    }
}

void setup() {
    Serial.begin(115200);
    setup_wifi();
    client.setServer(mqtt_server, 1883);

    voltageSensor.setSensitivity(SENSITIVITY);
}

void loop() {
  float voltage, Irms;
  int temperature = dht11.readTemperature();
 voltage = voltageSensor.getRmsVoltage();
  

  Irms = get_corriente();
 
    if (!client.connected()) {
        reconnect();
    }
    client.loop();
    publishData(voltage, Irms, temperature);

    delay(10000);
}

//funcion para obtener la corriente
float get_corriente() {
  float voltajeSensor;
  float corriente = 0;
  float Sumatoria = 0;
  long tiempo = millis();
  int N = 0;
  while (millis() - tiempo < 500) { 
    voltajeSensor = analogRead(14) * (1.1 / 4096.0); 
    corriente = voltajeSensor * 30.0; 
    Sumatoria = Sumatoria + sq(corriente); 
    N = N + 1;
    delay(1);
  }
  Sumatoria = Sumatoria * 6.9; 
  corriente = sqrt((Sumatoria) / N); 
  return(corriente);
}

