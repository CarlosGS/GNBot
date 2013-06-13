// GNBoard Test by Carlosgs (http://carlosgs.es)
// License: CC-BY-SA (http://creativecommons.org/licenses/by-sa/3.0/)

#include <Servo.h>

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

void setup() {
  delay(1000);
  
  init_button_pin();
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  randomSeed(analogRead(A5));
  ledRandom(255/10);
  
  //wait_for_button_press();
  
  Servo1.attach(SERVO_1_PIN);
  Servo2.attach(SERVO_2_PIN);
  Serial.begin(9600);
  digitalWrite(LED_PIN, HIGH);
  Servo1.write(90);
  Servo2.write(90);
  
  wait_for_button_press();
}

float mapf(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void loop() {
  ledRandom(255/5);
  Servo1.write(servo1pos);
  Servo2.write(servo2pos);
  
  if(button_is_pressed()) {
    playMusicScale(60);
    playInvertedMusicScale(30);
  }
  
  Serial.print("Analog measure: ");
  int val = analogRead(BATTERY_PIN);
  Serial.print(val);
  Serial.print(" (");
  float voltage = mapf(val,0,1023,0,5*2); // There is a 1/2 voltage divider
  Serial.print(voltage);
  Serial.print("V). ");
  
  servo1pos += servo1inc;
  servo2pos += servo2inc;
  int servodiff = 20;
  if(servo1pos > 90+servodiff || servo1pos < 90-servodiff) servo1inc *= -1;
  if(servo2pos > 90+servodiff || servo2pos < 90-servodiff) servo2inc *= -1;
  
  delay(100);
 }
