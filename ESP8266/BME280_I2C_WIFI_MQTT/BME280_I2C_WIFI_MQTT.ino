#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Streaming.h>

#define SLEEP_DELAY_IN_SECONDS  60
#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme; // I2C

const char* ssid = "WIFI_SSID";
const char* password = "WIFI_PASSWORD";

const char* mqtt_server = "SERVER_MQTT_IP";
const char* mqtt_username = "USER_SERVER_MQTT";
const char* mqtt_password = "PASSWORD_MQTT";

const char* mqtt_topic_T = "BME280/temperatura";
const char* mqtt_topic_F = "BME280/temperaturaF";
const char* mqtt_topic_P = "BME280/presion";
const char* mqtt_topic_H = "BME280/humedad";
const char* mqtt_topic_A = "BME280/Altitud";

WiFiClient espClient;
PubSubClient client(espClient);

char temperatureString[6];
char temperatureFString[6];
char pressureString[6];
char humedadString[6];
char altitudString[6];

void setup() {
  // setup serial port
  Serial.begin(115200);


  bool status;

  status = bme.begin(0x76);  
  if (!status) {
    Serial.println("No se encuentra sensor BME280, comprobar conexiones!");
    while (1);
  }

  // setup WiFi
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
 
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Conectando con: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println("Direccion IP: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Esperando conexion con MQTT...");
    // Attempt to connect
    if (client.connect("ESP8266Client", mqtt_username, mqtt_password)) {
      Serial.println("conectado");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

float getTemperature() {
  Serial << "Solicitando Temperatura..." << endl;
  float temp;
  do {
    temp = bme.readTemperature();
    delay(100);
  } while (temp == 85.0 || temp == (-127.0));
  return temp;
}

float getTemperatureF() {
  Serial << "Solicitando Temperatura..." << endl;
  float tempF;
  do {
    tempF = (1.8 * bme.readTemperature() + 32);
    delay(100);
  } while (tempF == 85.0 || tempF == (-127.0));
  return tempF;
}

float getPressure() {
  Serial << "Solicitando Presión..." << endl;
  float pres;
  do {
    pres = (bme.readPressure() / 100.0F);
    delay(100);
  } while (pres == 85.0 || pres == (-127.0));
  return pres;
}

float getHumidity() {
  Serial << "Solicitando Humedad..." << endl;
  float humi;
  do {
    humi = bme.readHumidity();
    delay(100);
  } while (humi == 85.0 || humi == (-127.0));
  return humi;
}

float getAltitude() {
  Serial << "Solicitando Altitud..." << endl;
  float alti;
  do {
    alti = bme.readAltitude(SEALEVELPRESSURE_HPA);
    delay(100);
  } while (alti == 85.0 || alti == (-127.0));
  return alti;
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  float temperature = getTemperature();
  // convert temperature to a string with two digits before the comma and 2 digits for precision
  dtostrf(temperature, 2, 2, temperatureString);
  // send temperature to the serial console
  Serial << "Enviando Temperatura: " << temperatureString << "ºC" << endl;
  
  // send temperature to the MQTT topic
  client.publish(mqtt_topic_T, temperatureString);


  float temperatureF = getTemperatureF();
  // convert temperature to a string with two digits before the comma and 2 digits for precision
  dtostrf(temperatureF, 2, 2, temperatureFString);
  // send temperature to the serial console
  Serial << "Enviando Temperatura: " << temperatureFString << "ºF" << endl;
  
  // send temperature to the MQTT topic
  client.publish(mqtt_topic_F, temperatureFString);
  
  float pressure = getPressure();
  // convert temperature to a string with two digits before the comma and 2 digits for precision
  dtostrf(pressure, 2, 2, pressureString);
  // send pressure to the serial console
  Serial << "Enviando Presión: " << pressureString << "hPa" << endl;

  // send presure to the MQTT topic
  client.publish(mqtt_topic_P, pressureString);
  
  float humedad = getHumidity();
  // convert humidity to a string with two digits before the comma and 2 digits for precision
  dtostrf(humedad, 2, 2, humedadString);
  // send humidity to the serial console
  Serial << "Enviando Humedad: " << humedadString <<"%" << endl;
  
  // send humidity to the MQTT topic
    client.publish(mqtt_topic_H, humedadString);

  float altitud = getAltitude();
  // convert humidity to a string with two digits before the comma and 2 digits for precision
  dtostrf(altitud, 2, 2, altitudString);
  // send humidity to the serial console
  Serial << "Enviando Altitud: " << altitudString <<"m" << endl;
  
  // send Altitude to the MQTT topic
  
  client.publish(mqtt_topic_A, altitudString);
  
  Serial << "Cerrando conexión MQTT ..." << endl;
  client.disconnect();

  Serial << "Cerrando conexión WiFi..." << endl;
  WiFi.disconnect();

  delay(100);

  Serial << "Entrando en modo deep sleep " << SLEEP_DELAY_IN_SECONDS << " segundos..." << endl;
  ESP.deepSleep(SLEEP_DELAY_IN_SECONDS * 1000000, WAKE_RF_DEFAULT);
  //ESP.deepSleep(10 * 1000, WAKE_NO_RFCAL);
  delay(500);   // wait for deep sleep to happen
}
