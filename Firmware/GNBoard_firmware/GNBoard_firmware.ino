// GNBoard Firmware by Carlosgs (http://carlosgs.es)
// License: CC-BY-SA (http://creativecommons.org/licenses/by-sa/3.0/)

#include <Servo.h>
#include <Wire.h> //I2C Arduino Library
#include <HMC5883L.h> //Compass library

#define Serial Serial1

#define LED_PIN 13

#define LED_R_PIN 9
#define LED_G_PIN 10
#define LED_B_PIN 11

#define BATTERY_PIN A0

#define BUTTON_1_PIN 12

#define DHT_PIN A9

#define BUZZER_PIN 8

#define SERVO_1_PIN 7
#define SERVO_2_PIN 6

Servo Servo1;
Servo Servo2;

// Magnetometer
// Store our compass as a variable.
HMC5883L compass;
float declinationAngle = 0;
float readMagnetometer() {
  // Retrived the scaled values from the compass (scaled to the configured scale).
  MagnetometerScaled scaled = compass.ReadScaledAxis();
  float heading = atan2(scaled.YAxis, scaled.XAxis);
  
  // Once you have your heading, you must then add your 'Declination Angle', which is the 'compassError' of the magnetic field in your location.
  // Find yours here: http://www.magnetic-declination.com/
  // Mine is: 2� 37' W, which is 2.617 Degrees, or (which we need) 0.0456752665 radians, I will use 0.0457
  // If you cannot find your Declination, comment out these two lines, your compass will be slightly off.
  //float declinationAngle = 0;
  heading += declinationAngle;
  
  // Correct for when signs are reversed.
  if(heading < 0)
    heading += 2*PI;
    
  // Check for wrap due to addition of declination.
  if(heading > 2*PI)
    heading -= 2*PI;
   
  // Convert radians to degrees for readability.
  float headingDegrees = heading * 180/M_PI;
  return headingDegrees;
}
// Record any compassErrors that may occur in the compass.
int compassError = 0;
void setupMagnetometer() {
  Serial.println("Starting the I2C interface.");
  Wire.begin(); // Start the I2C interface.
  
  Serial.println("Constructing new HMC5883L");
  compass = HMC5883L(); // Construct a new HMC5883 compass.
  
  Serial.println("Setting scale to +/- 1.3 Ga");
  compassError = compass.SetScale(1.3); // Set the scale of the compass.
  if(compassError != 0) // If there is an compassError, print it out.
    Serial.println(compass.GetErrorText(compassError));
    
  Serial.println("Setting measurement mode to continous.");
  compassError = compass.SetMeasurementMode(Measurement_Continuous); // Set the measurement mode to Continuous
  if(compassError != 0) // If there is an compassError, print it out.
    Serial.println(compass.GetErrorText(compassError));
    
  // Set our initial offset angle
  MagnetometerScaled scaled = compass.ReadScaledAxis();
  float heading = atan2(scaled.YAxis, scaled.XAxis);
  declinationAngle -= heading;
}

// Music notes
#define DO 262
#define RE 294
#define MI 330
#define FA 349
#define SOL 392
#define LA 440
#define SI 494

void playNote(int frequency, int length) {
  if(frequency <= 0) frequency = DO;
  if(length <= 0) length = 100;
  tone(BUZZER_PIN,frequency);
  delay(length);
  noTone(BUZZER_PIN);
  delay(100);
}

void playMusicScale(int length) {
  playNote(DO, length);
  playNote(RE, length);
  playNote(MI, length);
  playNote(FA, length);
  playNote(SOL, length);
  playNote(LA, length);
  playNote(SI, length);
}

void playInvertedMusicScale(int length) {
  playNote(SI, length);
  playNote(LA, length);
  playNote(SOL, length);
  playNote(FA, length);
  playNote(MI, length);
  playNote(RE, length);
  playNote(DO, length);
}



// --- BUTTON ---

// Function for initiating the pin used by the button
void init_button_pin() {
  pinMode(BUTTON_1_PIN,INPUT);

  digitalWrite(BUTTON_1_PIN, HIGH); // Enable internal pull-up resistor
}

// Function for reading the button value
// Returns 1 if the button is being pressed, 0 if it is not
int button_is_pressed() {
  return !digitalRead(BUTTON_1_PIN);
}

// Function that waits until the button is pressed
void wait_for_button_press() {
  while( button_is_pressed() == 0 ) {
    delay(100);
  }
}



void ledColor(int R, int G, int B) {
  analogWrite(LED_R_PIN, R);
  analogWrite(LED_G_PIN, G);
  analogWrite(LED_B_PIN, B);
}

void ledRandom(int maxVal) {
  ledColor(random(maxVal+1),random(maxVal+1),random(maxVal+1));
}

int servo1pos = 90,servo1inc=1;
int servo2pos = 90,servo2inc=-2;

unsigned long iniTime;

void setup() {
  delay(1000);
  
  init_button_pin();
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  randomSeed(analogRead(A5));
  ledRandom(255/50);
  
  wait_for_button_press();
  playMusicScale(60);
  playInvertedMusicScale(30);
  
  Servo1.attach(SERVO_1_PIN);
  Servo2.attach(SERVO_2_PIN);
  Serial.begin(9600);
  digitalWrite(LED_PIN, HIGH);
  Servo1.write(90);
  Servo2.write(90);
  
  setupMagnetometer();
  
  wait_for_button_press();
  ledRandom(0);
  
  iniTime = millis();
}

float mapf(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

float analogReadAverage(int pin, int samples) {
  int result = 0;
  for(int i=0; i<samples; i++) {
    result += analogRead(pin);
  }
  return float(result)/float(samples);
}

float getBatteryVoltage() {
  int val = analogReadAverage(BATTERY_PIN,5);
  return mapf(val,0,1023,0,5*2);
}

unsigned long last_timestamp = 0;

void loop() {
  //ledRandom(255/100);
  Servo1.write(servo1pos);
  Servo2.write(servo2pos);
  
  //if(button_is_pressed()) {
  //  playMusicScale(60);
  //  playInvertedMusicScale(30);
  //}
  
  if(button_is_pressed()) {
    Serial.print("BUTTON PRESSED (+5s delay)\n\r");
    delay(5000);
  }
  
  if(Serial.available()) {
    Serial.print("RECEIVED: ");
    while(Serial.available())
      Serial.print((char)Serial.read());
    Serial.print("\n\r");
    //delay(1000);
  }
  
  int averageWindow = 5;
  Serial.print("L1:");
  Serial.print(analogReadAverage(A1,averageWindow));
  Serial.print(" L2:");
  Serial.print(analogReadAverage(A2,averageWindow));
  Serial.print(" L3:");
  Serial.print(analogReadAverage(A3,averageWindow));
  Serial.print(" L4:");
  Serial.print(analogReadAverage(A4,averageWindow));
  
  Serial.print(" M:");
  float M_degrees = readMagnetometer();
  Serial.print(M_degrees);
  
  //ledRandom(255.f*0.5f*float(M_degrees/360.f));
  
  Serial.print(" V:");
  Serial.print(getBatteryVoltage());
  
  unsigned long timestamp = millis()-iniTime;
  Serial.print(" T:");
  Serial.print(timestamp);
  
  Serial.print(" diffT:");
  Serial.print(timestamp-last_timestamp);
  
  last_timestamp = timestamp;
  
  Serial.print("\n\r");
  
  servo1pos += servo1inc;
  servo2pos += servo2inc;
  int servodiff = 20;
  if(servo1pos > 90+servodiff || servo1pos < 90-servodiff) servo1inc *= -1;
  if(servo2pos > 90+servodiff || servo2pos < 90-servodiff) servo2inc *= -1;
  
  while( (millis()-iniTime-last_timestamp) < 90 ); // Sample period: ~100ms
  
  //delay(10);
 }
