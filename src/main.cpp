#include <Arduino.h>
#include <DHT.h>
#include <Servo.h>

// Define pins
#define ANALOG_IN A0  // Shared analog pin
#define DHT_PIN D1          // DHT sensor data pin (connected to GPIO5)
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
const unsigned long readingInterval = 3600000;  // Interval for sending data (10 seconds)

// Arrays to store the last 4 readings
float tempReadings[4] = {0};
float humidityReadings[4] = {0};
int moistureReadings[4] = {0};
int photoReadings[4] = {0};
int readingIndex = 0;

// Function to calculate the average of an array
template <typename T>
T calculateAverage(T readings[], int size) {
    T sum = 0;
    for (int i = 0; i < size; i++) {
        sum += readings[i];
    }
    return sum / size;
}

// Function to set RGB LED color
void setColor(int red, int green, int blue) {
    analogWrite(RED_PIN, red);
    analogWrite(GREEN_PIN, green);
    analogWrite(BLUE_PIN, blue);
}

// Function to move the servo
void moveServo() {
    myServo.write(180);
    delay(500);
    myServo.write(0);
}

// Queue to handle multiple conditions
bool tempCondition = false;
bool photoCondition = false;
bool moistureCondition = false;
bool humidityCondition = false;

// Simulated time of day using millis() (for demonstration purposes)
// Assuming a full day (24 hours) is simulated in 24 * 60 * 60 * 1000 milliseconds
unsigned long millisInADay = 24L * 60L * 60L * 1000L;  // 24 hours in milliseconds
unsigned long simulatedStartTime = 8L * 60L * 60L * 1000L;  // Simulated start time at 8:00 AM
unsigned long simulatedEndTime = 20L * 60L * 60L * 1000L;  // Simulated end time at 8:00 PM

// Correct offset to align millis() with Berlin time (10:40 AM)
unsigned long berlinOffset = 9L * 60L * 60L * 1000L + 40L * 60L * 1000L;  // 9,600,000 ms

bool isWithinDaytime() {
    // Adjust the current time using the offset
    unsigned long currentMillis = (millis() + berlinOffset) % millisInADay;  // Adjusted simulated time

    // Debugging: Print the current simulated time
    // Serial.print("Simulated time (ms): ");
    // Serial.println(currentMillis);

    // Debugging: Print the daytime range
    // Serial.print("Daytime starts at (ms): ");
    // Serial.println(simulatedStartTime);
    // Serial.print("Daytime ends at (ms): ");
    // Serial.println(simulatedEndTime);

    // Check if the current time is within the daytime range
    return (currentMillis >= simulatedStartTime && currentMillis <= simulatedEndTime);
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

        // Read soil moisture
        digitalWrite(MOISTURE_PIN, HIGH);
        digitalWrite(PHOTORESISTOR_PIN, LOW);
        delay(1000);
        int moistureValue = analogRead(ANALOG_IN);
        digitalWrite(MOISTURE_PIN, LOW);

        // Read photoresistor (only if within daytime)
        int photoValue = 0;
        if (isWithinDaytime()) {
            digitalWrite(PHOTORESISTOR_PIN, HIGH);
            delay(1000);
            photoValue = analogRead(ANALOG_IN);
            digitalWrite(PHOTORESISTOR_PIN, LOW);
        }

        // Read temperature and humidity from DHT sensor
        float temperature = dht.readTemperature();  // Read temperature (in Celsius)
        float humidity = dht.readHumidity();

        // Store the readings in arrays
        tempReadings[readingIndex] = temperature;
        humidityReadings[readingIndex] = humidity;
        moistureReadings[readingIndex] = moistureValue;
        photoReadings[readingIndex] = photoValue;

        // Update the reading index
        readingIndex = (readingIndex + 1) % 4;

        // Calculate averages
        float avgTemp = calculateAverage(tempReadings, 4);
        float avgHumidity = calculateAverage(humidityReadings, 4);
        int avgMoisture = calculateAverage(moistureReadings, 4);
        int avgPhoto = calculateAverage(photoReadings, 4);

        // Print the values
        Serial.println("-----------------------------");
        Serial.print("Temperature: ");
        Serial.print(temperature);
        Serial.print(" °C (Avg: ");
        Serial.print(avgTemp);
        Serial.println(" °C)");

        Serial.print("Humidity: ");
        Serial.print(humidity);
        Serial.print(" % (Avg: ");
        Serial.print(avgHumidity);
        Serial.println(" %)");

        Serial.print("Moisture Value: ");
        Serial.print(moistureValue);
        Serial.print(" (Avg: ");
        Serial.print(avgMoisture);
        Serial.println(")");

        if (isWithinDaytime()) {
            Serial.print("Photoresistor Value: ");
            Serial.print(photoValue);
            Serial.print(" (Avg: ");
            Serial.print(avgPhoto);
            Serial.println(")");
        } else {
            Serial.println("Photoresistor Value: Skipped (Outside daytime)");
        }
        Serial.println("-----------------------------");

        // Check conditions and set flags
        tempCondition = (avgTemp < 12 || avgTemp > 28);
        photoCondition = isWithinDaytime() && (avgPhoto < 640);
        moistureCondition = (avgMoisture < 108);
        humidityCondition = (avgHumidity < 25);
    }

    // Handle conditions in sequence
    if (tempCondition) {
        setColor(255, 0, 0);  // Red light
        moveServo();
        tempCondition = false;  // Resolve condition
    } else if (photoCondition) {
        setColor(255, 255, 0);  // Yellow light
        moveServo();
        photoCondition = false;  // Resolve condition
    } else if (moistureCondition) {
        setColor(0, 0, 255);  // Blue light
        moveServo();
        moistureCondition = false;  // Resolve condition
    } else if (humidityCondition) {
        setColor(0, 255, 0);  // Green light
        moveServo();
        humidityCondition = false;  // Resolve condition
    } else {
        setColor(0, 0, 0);  // Turn off light if no conditions are active
    }
}
