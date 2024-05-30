#include <ESP8266WiFi.h>
#include <WiFiClient.h>

// Replace with your network credentials
const char* ssid = "pass";
const char* password = "pass@123";
// Replace with your ThingSpeak API key
const char* apiKey = "LLD0F8UJV2QWDJ5T";
// ThingSpeak server
const char* server = "api.thingspeak.com";

// IR sensor pins
const int entrySensor1 = D1;
const int entrySensor2 = D2;
const int entrySensor3 = D3;
const int exitSensor4 = D4;
const int exitSensor5 = D5;
const int exitSensor6 = D6;

// Initialize vehicle counts
int countEntry1 = 0;
int countEntry2 = 0;
int countEntry3 = 0;

// Initialize timers
unsigned long entry1HighTime = 0;
unsigned long entry2HighTime = 0;
unsigned long entry3HighTime = 0;
unsigned long exit4HighTime = 0;
unsigned long exit5HighTime = 0;
unsigned long exit6HighTime = 0;

// Flags to track if a sensor was high for 60 seconds
bool entry1Flag = false;
bool entry2Flag = false;
bool entry3Flag = false;
bool exit4Flag = false;
bool exit5Flag = false;
bool exit6Flag = false;

WiFiClient client;

void setup() {
    Serial.begin(9600);
 
    // Initialize WiFi connection
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    // Set up sensor pins as inputs
    pinMode(entrySensor1, INPUT);
    pinMode(entrySensor2, INPUT);
    pinMode(entrySensor3, INPUT);
    pinMode(exitSensor4, INPUT);
    pinMode(exitSensor5, INPUT);
    pinMode(exitSensor6, INPUT);
}

void loop() {
    // Read sensor states
    bool entry1 = digitalRead(entrySensor1) == LOW;
    bool entry2 = digitalRead(entrySensor2) == LOW;
    bool entry3 = digitalRead(entrySensor3) == LOW;
    bool exit4 = digitalRead(exitSensor4) == LOW;
    bool exit5 = digitalRead(exitSensor5) == LOW;
    bool exit6 = digitalRead(exitSensor6) == LOW;

    unsigned long currentTime = millis();

    // Handle entry sensors
    if (entry1) {
        if (entry1HighTime == 0) {
            entry1HighTime = currentTime;
        } else if (currentTime - entry1HighTime >= 60000) {
            entry1Flag = true;
        }
    } else {
        if (!entry1Flag) {
            countEntry1++;
            Serial.println("entrySensor1 triggered: Incrementing countEntry1");
        }
        entry1HighTime = 0;
        entry1Flag = false;
        delay(1000); // debounce delay
    }

    if (entry2) {
        if (entry2HighTime == 0) {
            entry2HighTime = currentTime;
        } else if (currentTime - entry2HighTime >= 60000) {
            entry2Flag = true;
        }
    } else {
        if (!entry2Flag) {
            countEntry2++;
            Serial.println("entrySensor2 triggered: Incrementing countEntry2");
        }
        entry2HighTime = 0;
        entry2Flag = false;
        delay(1000); // debounce delay
    }

    if (entry3) {
        if (entry3HighTime == 0) {
            entry3HighTime = currentTime;
        } else if (currentTime - entry3HighTime >= 60000) {
            entry3Flag = true;
        }
    } else {
        if (!entry3Flag) {
            countEntry3++;
            Serial.println("entrySensor3 triggered: Incrementing countEntry3");
        }
        entry3HighTime = 0;
        entry3Flag = false;
        delay(1000); // debounce delay
    }

    // Handle exit sensors
    if (exit4) {
        if (exit4HighTime == 0) {
            exit4HighTime = currentTime;
        } else if (currentTime - exit4HighTime >= 60000) {
            exit4Flag = true;
        }
    } else {
        if (!exit4Flag && countEntry1 > 0) {
            countEntry1--;
            Serial.println("exitSensor4 triggered: Decrementing countEntry1");
        }
        exit4HighTime = 0;
        exit4Flag = false;
        delay(1000); // debounce delay
    }

    if (exit5) {
        if (exit5HighTime == 0) {
            exit5HighTime = currentTime;
        } else if (currentTime - exit5HighTime >= 60000) {
            exit5Flag = true;
        }
    } else {
        if (!exit5Flag && countEntry2 > 0) {
            countEntry2--;
            Serial.println("exitSensor5 triggered: Decrementing countEntry2");
        }
        exit5HighTime = 0;
        exit5Flag = false;
        delay(1000); // debounce delay
    }

    if (exit6) {
        if (exit6HighTime == 0) {
            exit6HighTime = currentTime;
        } else if (currentTime - exit6HighTime >= 60000) {
            exit6Flag = true;
        }
    } else {
        if (!exit6Flag && countEntry3 > 0) {
            countEntry3--;
            Serial.println("exitSensor6 triggered: Decrementing countEntry3");
        }
        exit6HighTime = 0;
        exit6Flag = false;
        delay(1000); // debounce delay
    }

    // Calculate total count of all entries
    int totalEntries = countEntry1 + countEntry2 + countEntry3;

    // Update ThingSpeak every 10 seconds
    if (millis() % 10000 == 0) {
        updateThingSpeak(countEntry1, countEntry2, countEntry3, totalEntries);
    }
}

void updateThingSpeak(int countEntry1, int countEntry2, int countEntry3, int totalEntries) {
    if (WiFi.status() == WL_CONNECTED) {
        String url = "/update?api_key=";
        url += apiKey;
        url += "&field1=";
        url += countEntry1;
        url += "&field2=";
        url += countEntry2;
        url += "&field3=";
        url += countEntry3;
        url += "&field4=";
        url += totalEntries;
        Serial.print("Updating ThingSpeak: ");
        Serial.println(url);
        if (client.connect(server, 80)) {
            client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                         "Host: " + server + "\r\n" +
                         "Connection: close\r\n\r\n");
            client.stop();
        } else {
            Serial.println("Error connecting to ThingSpeak");
        }
    } else {
        Serial.println("WiFi Disconnected");
    }
}
