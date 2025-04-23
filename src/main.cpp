#include <Arduino.h>
#include <DHT.h>
#include <Servo.h>

// Define pins
#define ANALOG_IN A0  // Shared analog pin
#define DHT_PIN 5          // DHT sensor data pin (connected to GPIO5)
#define DHT_TYPE DHT11      // Change to DHT22 if using a DHT22 sensor
#define PHOTORESISTOR_PIN 4
#define MOISTURE_PIN 12
#define SERVO_PIN 2
#define RED_PIN 14
#define BLUE_PIN 15
#define GREEN_PIN 13

Servo myServo;

// Initialize DHT sensor    
DHT dht(DHT_PIN, DHT_TYPE);

// Variable to track the last time data was displayed
unsigned long lastReadingTime = 0;
const unsigned long readingInterval = 10000;  // Interval for sending data (10 seconds)

void setColor(int red, int green, int blue) {
    analogWrite(RED_PIN, red);
    analogWrite(GREEN_PIN, green);
    analogWrite(BLUE_PIN, blue);
}

void setup() {

  // Set up system to read moisture and light from only one analog pin
  pinMode(PHOTORESISTOR_PIN, OUTPUT);
  pinMode(MOISTURE_PIN, OUTPUT);

  digitalWrite(MOISTURE_PIN, LOW);
  digitalWrite(PHOTORESISTOR_PIN, LOW);

  // Set up LED's
  pinMode(RED_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);

  setColor(0, 0, 0);

  // Servo setup
  myServo.attach(SERVO_PIN);

  // Initialize Serial Monitor
  Serial.begin(115200);
  Serial.println("Starting Sensors...");

  // Initialize DHT sensor
  dht.begin();
}

void loop() {
  // Get the current time
  unsigned long now = millis();

  // Only read and display values every 10 seconds
  if (now - lastReadingTime > readingInterval) {
    lastReadingTime = now;

    // test colors
    setColor(255, 0, 0); 
    delay(1000);
    setColor(0, 255, 0); 
    delay(1000);
    setColor(0, 0, 255); 
    delay(1000);
    setColor(0, 0, 0); 
    myServo.write(0);
    Serial.println("Moving to 180 degrees");
    myServo.write(180);
    delay(500);
    Serial.println("Moving to default position");
    myServo.write(0);

    // Read soil moisture
    digitalWrite(MOISTURE_PIN, HIGH);
    digitalWrite(PHOTORESISTOR_PIN, LOW);
    delay(1000);
    int moistureValue = analogRead(ANALOG_IN);
    Serial.print("Moisture Value: ");
    Serial.println(moistureValue);
    digitalWrite(MOISTURE_PIN, LOW);

    // Read photoresistor
    digitalWrite(PHOTORESISTOR_PIN, HIGH);
    delay(1000);
    int photoValue = analogRead(ANALOG_IN);
    Serial.print("Photoresistor Value: ");
    Serial.println(photoValue);
    digitalWrite(PHOTORESISTOR_PIN, LOW);

    // Read temperature and humidity from DHT sensor
    float temperature = dht.readTemperature();  // Read temperature (in Celsius)
    float humidity = dht.readHumidity();

    // Check if readings are valid
    if (isnan(temperature) || isnan(humidity)) {
      Serial.println("Failed to read from DHT sensor!");
    } else {
      Serial.print("Temperature: ");
      Serial.print(temperature);
      Serial.println(" °C");

      Serial.print("Humidity: ");
      Serial.print(humidity);
      Serial.println(" %");
    }

    Serial.println("-----------------------------");
  }
}
