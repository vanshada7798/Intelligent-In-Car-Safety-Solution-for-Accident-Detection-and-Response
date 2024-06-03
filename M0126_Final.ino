#include <SoftwareSerial.h>
#include <TinyGPS++.h>
String textMessage;
#include <Wire.h>
#include <avr/wdt.h>

float a[2];
#define RXPin 7 //gps
#define TXPin 6 //gps
SoftwareSerial gpsSerial(RXPin, TXPin); //RX,TX
SoftwareSerial SIM900(5, 4); //gsm
#define Force A3
TinyGPSPlus gps;

float longitude;
float lattitude;
const int buttonPin = 8;
unsigned long buttonPressStartTime = 0;
bool countdownActive = false;
int b;
bool x = true;
unsigned long startTime;
const unsigned long waitTime = 3000;
const int xInput = A0;
const int yInput = A1;
const int zInput = A2;
int RawMin = 0;
int RawMax = 1023;
const int sampleSize = 10;
bool q = true;
void setup() {
  Serial.begin(9600);
  gpsSerial.begin(9600);
  SIM900.begin(9600);
  delay(1000);
  SIM900.println("AT");
  SIM900.println("AT+CMGF=1");
  pinMode(buttonPin, INPUT);

  //analogReference(EXTERNAL);
}

void loop() {
  b = analogRead(Force);
  Serial.println(b);
  if (b == 1)
  {
    Serial.println("Minor accident");
    if (waitForButtonPress()) {
      button();

    } else {
      *get_gps();
      smss();
      delay(1000);
     
    }
  }
  else if ( b > 1)
  {
    Serial.println("Major accident");
    *get_gps();
    sms();
    delay(1000);
  }
  delay(100);
  Adxl();
}

float *get_gps()
{
  gpsSerial.listen();
  Serial.println("INSIDE get_gps");
  while (1) {
    while (gpsSerial.available() > 0) {
      gps.encode(gpsSerial.read());
    }
    if (gps.location.isUpdated()) {
      Serial.print("LAT=");
      Serial.println(gps.location.lat(), 6);
      Serial.print("LONG=");
      Serial.println(gps.location.lng(), 6);
      lattitude = gps.location.lat();
      longitude = gps.location.lng();
      break;
    }
  }
  return a;
}

void button()
{
  if (digitalRead(buttonPin) == HIGH) {
    if (!countdownActive) {
      countdownActive = true;
      buttonPressStartTime = millis();
      Serial.println("Countdown started!");
    }

    unsigned long elapsedTime = millis() - buttonPressStartTime;

    if (elapsedTime < 10000) {
      Serial.print("Elapsed Time: ");
      Serial.print(elapsedTime / 1000);
      Serial.println(" seconds");
      wdt_enable(WDTO_15MS);
      while (1) {}
    } else {
      Serial.println("10 seconds completed!");
      countdownActive = false;
    }
  } else {
    if (x == true)
    {
      *get_gps();
      sms();
    }
    x == false;
    countdownActive = false;
  }
  delay(1000);
}
void sms() {
  delay(1000);
  sendATCommand("AT+CMGF=1");
  sendATCommand("AT+CMGS=\"+917798229767\""); // Replace +1234567890 with the recipient's phone number
  delay(1000);
  SIM900.print("Major Accident Detected\n");
  SIM900.print("Latitude: ");
  delay(500);
  SIM900.print((gps.location.lat()));
  SIM900.print("\n");
  SIM900.print("Longitude: ");
  SIM900.print((gps.location.lng()));
  delay(100);
  SIM900.write(26);
  delay(1000);
  Serial.println("SMS Sent.");
}
void smss() {
  delay(1000);
  sendATCommand("AT+CMGF=1");
  sendATCommand("AT+CMGS=\"+917798229767\""); // Replace +1234567890 with the recipient's phone number
  delay(1000);
  SIM900.print("Minor Accident Detected\n");
  SIM900.print("Latitude: ");
  delay(500);
  SIM900.print((gps.location.lat()));
  SIM900.print("\n");
  SIM900.print("Longitude: ");
  SIM900.print((gps.location.lng()));
  delay(100);
  SIM900.write(26);
  delay(1000);
  Serial.println("SMS Sent.");
}


void sendATCommand(String command) {
  SIM900.println(command);
  delay(500);
  while (SIM900.available()) {
    Serial.write(SIM900.read());
  }
  Serial.println();
}
bool waitForButtonPress() {
  startTime = millis();
  while ((millis() - startTime) < waitTime) {
    if (digitalRead(buttonPin) == HIGH) {
      return true;
    }
  }
  return false;
}
void Adxl()
{
  b = analogRead(Force);
  int xRaw = ReadAxis(xInput);
  int yRaw = ReadAxis(yInput);
  int zRaw = ReadAxis(zInput);

  // Convert raw values to 'milli-Gs"
  long xScaled = map(xRaw, RawMin, RawMax, -3000, 3000);
  long yScaled = map(yRaw, RawMin, RawMax, -3000, 3000);
  long zScaled = map(zRaw, RawMin, RawMax, -3000, 3000);

  // re-scale to fractional Gs
  float xAccel = xScaled / 1000.0;
  float yAccel = yScaled / 1000.0;
  float zAccel = zScaled / 1000.0;

  Serial.print("X, Y, Z  :: ");
  Serial.print(xRaw);
  Serial.print(", ");
  Serial.print(yRaw);
  Serial.print(", ");
  Serial.print(zRaw);
  Serial.print(" :: ");
  Serial.print(xAccel, 0);
  Serial.print("G, ");
  Serial.print(yAccel, 0);
  Serial.print("G, ");
  Serial.print(zAccel, 0);
  Serial.println("G");

  delay(200);
  if (xRaw < 290 && yRaw > 350 && zRaw < 361)
  {
    Serial.println("Left");
    if (q == true)
    {
      *get_gps();
     sms_tilt();
     //sms();
      q = false;
    }
  }
  if (xRaw > 421 && yRaw > 340 && zRaw > 326)
  {
    Serial.println("Right");
    if (q == true)
    {
      *get_gps();
      sms_tilt();
      //sms();
      q = false;
    }
  }
}
// Take samples and return the average
int ReadAxis(int axisPin)
{
  long reading = 0;
  analogRead(axisPin);
  delay(1);
  for (int i = 0; i < sampleSize; i++)
  {
    reading += analogRead(axisPin);
  }
  return reading / sampleSize;
}
void sms_tilt() {
  delay(1000);
  sendATCommand("AT+CMGF=1");
  sendATCommand("AT+CMGS=\"+917798229767\""); // Replace +1234567890 with the recipient's phone number
  delay(1000);
  SIM900.print("Car Tilted, Accident Detected\n");
  SIM900.print("Latitude: ");
  delay(500);
  SIM900.print((gps.location.lat()));
  SIM900.print("\n");
  SIM900.print("Longitude: ");
  SIM900.print((gps.location.lng()));
  delay(100);
  SIM900.write(26);
  delay(1000);
  Serial.println("SMS Sent.");
}
