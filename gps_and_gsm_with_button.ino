#include <SoftwareSerial.h>
#include <TinyGPS++.h>

// Pin Definitions
const int buttonPin = 6;  // Button input pin

// GSM Module
SoftwareSerial gsmSerial(9, 10); // RX, TX for GSM module
const String phoneNumber = "+919370978144";  // Replace with the recipient's phone number

// GPS Module
TinyGPSPlus gps;
SoftwareSerial gpsSerial(4, 3); // RX, TX for GPS module

// Button State
int buttonState = 0;
int lastButtonState = 0;

// GPS Location Variables
double latitude, longitude;

// ESP32-CAM Video Link
String videoLink = "http://192.168.236.17/stream"; // Replace with the actual video link

void setup() {
  // Initialize Serial Monitor
  Serial.begin(9600);
  gsmSerial.begin(9600);  // GSM Module Serial
  gpsSerial.begin(9600);  // GPS Module Serial

  // Initialize button pin
  pinMode(buttonPin, INPUT);

  // Send Initial AT command to GSM module
  sendGSMCommand("AT");
  delay(1000);
}

void loop() {
  // Read GPS data
  while (gpsSerial.available() > 0) {
    gps.encode(gpsSerial.read());
    if (gps.location.isUpdated()) {
      latitude = gps.location.lat();
      longitude = gps.location.lng();
    }
  }

  // Read button state
  buttonState = digitalRead(buttonPin);
  
  // If button is pressed and wasn't pressed previously
  if (buttonState == LOW && lastButtonState == HIGH) {
    // Send SMS with GPS location and video link
    sendSMSWithLocation();
    delay(500);  // debounce delay
  }

  lastButtonState = buttonState;
}

void sendGSMCommand(String command) {
  gsmSerial.println(command);
  delay(1000);
}

void sendSMSWithLocation() {
  String message = "I am not safe:\n";
  message += "Location: " + String(latitude, 6) + ", " + String(longitude, 6) + "\n";
  message += "Video: " + videoLink;

  // Send SMS command to GSM module
  gsmSerial.println("AT+CMGF=1");  // Set SMS mode to text
  delay(1000);
  gsmSerial.print("AT+CMGS=\"");
  gsmSerial.print(phoneNumber);
  gsmSerial.println("\"");
  delay(1000);
  gsmSerial.println(message);
  gsmSerial.write(26);  // Send Ctrl+Z to send SMS
  delay(5000); // Wait for the message to send
}
