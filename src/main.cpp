#include <SPI.h>
#include <Wire.h>
#include "DHT.h"
#include "nRF24L01.h"
#include "RF24.h"
#include "RF24Network.h"
#include "printf.h"
#include <TroykaMQ.h>

// define sensor pins
const uint8_t DHT22_PIN = 2; // air temp and humidity sensor
const uint8_t MQ135_PIN = A0; // co2 sensor
const uint8_t LIGHT_SENSOR_PIN = A1;

const uint8_t HUMIDITY_SENSOR_PIN =  A2;
const uint16_t HUMIDITY_SENSOR_MAX = 1024;
const uint16_t HUMIDITY_SENSOR_MIN = 200;
const uint8_t HUMIDITY_THRESHOLD = 30;

// creating nrf24 radio
RF24 radio(9, 10);
RF24Network radioNetwork(radio);

// creating sensors
DHT dht(DHT22_PIN, DHT22);
MQ135 mq135(MQ135_PIN);

// array that stores sensor data
uint16_t t_data[6] = {0, 0, 0, 0, 0, 0};

// define pipe addresses for radio communication
// with master controller
const uint16_t ADDR = 02;
const uint16_t MASTER_ADDR = 00;

// send status
bool state;
const uint32_t SLEEP_DURATION = 300000; // 5 minutes

void setup() {
  SPI.begin();
  // Serial.begin(115200); // debug console
  pinMode(DHT22_PIN, INPUT);
  // Starting radio
  radio.begin();
  radioNetwork.begin(90, ADDR);
  radio.setDataRate(RF24_250KBPS);
  
  dht.begin(); // initialize dht22 sensor
  mq135.calibrate(); // calibrating co2 sensor
}
void loop() {
  radioNetwork.update();
  // reading sensors data
  t_data[0] = dht.readTemperature();
  t_data[1] = dht.readHumidity();
  t_data[2] = map(
    analogRead(HUMIDITY_SENSOR_PIN),
    HUMIDITY_SENSOR_MIN,
    HUMIDITY_SENSOR_MAX,
    100,
    0
  );
  t_data[3] = mq135.readCO2();
  // setting network header to send data to master node
  RF24NetworkHeader header(MASTER_ADDR);
  // sending data
  state = radioNetwork.write(header, &t_data, sizeof(t_data));
  // sleep
  delay(SLEEP_DURATION);
}