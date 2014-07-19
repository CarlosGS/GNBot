// GNBoard Firmware by Carlosgs (http://carlosgs.es)
// License: CC-BY-SA (http://creativecommons.org/licenses/by-sa/3.0/)

#include <Servo.h>
#include <Wire.h> //I2C Arduino Library
#include <HMC5883L.h> //Compass library
#include <XBee.h> // XBee library (usage based on examples by Andrew Rapp)

// create the XBee object
XBee xbee = XBee();

#define XbeeSerial Serial1

#define Serial Serial

#define NOSE_HEAT_PIN 2
#define NOSE_VOUT_PIN A10

#define LDR1_PIN A2
#define LDR2_PIN A3
#define LDR3_PIN A4
#define LDR4_PIN A1

#define IR1_PIN A5
#define IR2_PIN A6
#define IR3_PIN A7
#define IR4_PIN A8

#define LED_R_PIN 9
#define LED_G_PIN 10
#define LED_B_PIN 11

#define BATTERY_PIN A0

#define BUTTON_1_PIN 12

#define DHT_PIN A9

#define BUZZER_PIN 8

#define SERVO_1_PIN 7 // left
#define SERVO_2_PIN 6 // right

Servo Servo1;
Servo Servo2;


float mapf(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Magnetometer
// Store our compass as a variable.
HMC5883L compass;
float declinationAngle = 0;
float compass_yMax = -1;
float compass_yMin = 1;
float compass_xMax = -1;
float compass_xMin = 1;
float readMagnetometer() {
  MagnetometerRaw raw = compass.ReadRawAxis();
  return raw.XAxis;
  // Retrived the scaled values from the compass (scaled to the configured scale).
  MagnetometerScaled scaled = compass.ReadScaledAxis();
  if(scaled.YAxis > compass_yMax)
    compass_yMax = scaled.YAxis;
  if(scaled.YAxis < compass_yMin)
    compass_yMin = scaled.YAxis;
  if(scaled.XAxis > compass_xMax)
    compass_xMax = scaled.XAxis;
  if(scaled.XAxis < compass_xMin)
    compass_xMin = scaled.XAxis;
  float y = mapf(scaled.YAxis, compass_yMin, compass_yMax, -1, 1);
  float x = mapf(scaled.XAxis, compass_xMin, compass_xMax, -1, 1);
  float heading = atan2(y, x);
  return scaled.XAxis;
  Serial.print("YAxis:");
  Serial.print(scaled.YAxis);
  Serial.print("YAxis:");
  Serial.println(scaled.YAxis);
  
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
void magnetometerToZero() {
  // Set our initial offset angle
  MagnetometerScaled scaled = compass.ReadScaledAxis();
  float heading = atan2(scaled.YAxis, scaled.XAxis);
  declinationAngle = -heading;
}
void setupMagnetometer() {
  Serial.println("Starting the I2C interface.");
  Wire.begin(); // Start the I2C interface.
  
  Serial.println("Constructing new HMC5883L");
  compass = HMC5883L(); // Construct a new HMC5883 compass.
  
  Serial.println("Setting scale to +/- 1.3 Ga");
  compassError = compass.SetScale(0.88); // Set the scale of the compass.
  if(compassError != 0) // If there is an compassError, print it out.
    Serial.println(compass.GetErrorText(compassError));
    
  Serial.println("Setting measurement mode to continous.");
  compassError = compass.SetMeasurementMode(Measurement_Continuous); // Set the measurement mode to Continuous
  if(compassError != 0) // If there is an compassError, print it out.
    Serial.println(compass.GetErrorText(compassError));
    
  // Set our initial offset angle
  magnetometerToZero();
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

void playMusicTone1(int length) {
  playNote(MI, length);
  playNote(DO, length);
  playNote(MI, length);
  playNote(DO, length);
  playNote(DO, length);
  playNote(DO, length);
  playNote(DO, length*3);
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
unsigned long last_timestamp = 0;

int loggerMode = 0;

void initRobot() {
  
  init_button_pin();
  
  Servo1.write(90);
  Servo2.write(90);
  
  magnetometerToZero();
  
  loggerMode = 0;
  
  playMusicScale(20);
}







float analogReadAverage(int pin, int samples) {
  int result = 0;
  for(int i=0; i<samples; i++) {
    result += analogRead(pin);
  }
  return float(result)/float(samples);
}

float getBatteryVoltage() {
  int val = analogRead(BATTERY_PIN);
  return mapf(val,0,1023, 0,21.1765); // Voltage divider with 22k in series with 6k8
}

char inBuffer[128] = "";
int inBuffer_len = 0;

void readLine(char *out) {
  int gotLine = 0;
  while(Serial.available()) {
    char val = Serial.read();
    if(val == '\n') {
      gotLine = 1;
      break;
    }
    if(val != '\r') {
      inBuffer[inBuffer_len] = val;
      inBuffer_len++;
      //char strVal[2];
      //strVal[0] = val;
      //strVal[1] = '\0';
      //strcat(inBuffer,strVal);
    }
  }
  if(gotLine) {
    inBuffer[inBuffer_len] = '\0';
    strcpy(out,inBuffer);
    //*inBuffer = '\0';
    inBuffer_len = 0;
  } else
    *out = '\0';
}

char buffer[128];

int motors_oldL = 90;
int motors_oldR = 90;


int statusLed = LED_G_PIN;
int errorLed = LED_R_PIN;
void flashLed(int pin, int times, int wait) {

  for (int i = 0; i < times; i++) {
    digitalWrite(pin, HIGH);
    delay(wait);
    digitalWrite(pin, LOW);

    if (i + 1 < times) {
      delay(wait);
    }
  }
}
ZBTxStatusResponse txStatus = ZBTxStatusResponse();

// Modified from http://www.desert-home.com/2013/02/using-xbee-library-part-3.html
XBeeAddress64 CoordinatorAddress = XBeeAddress64(0x00000000, 0x00000000); // Address 0 is the coordinator
void sendXbee(const char* command){
  ZBTxRequest zbtx = ZBTxRequest(CoordinatorAddress, (uint8_t *)command, strlen(command));
  xbee.send(zbtx);
}

ZBRxResponse rx = ZBRxResponse();










void setup() {
  delay(1000);
  
  randomSeed(analogRead(A4));
  ledRandom(255/50);
  
  setupMagnetometer();
  
  //Servo1.attach(SERVO_1_PIN);
  //Servo2.attach(SERVO_2_PIN);
  Serial.begin(115200);
  
  XbeeSerial.begin(9600);
  xbee.setSerial(XbeeSerial);
  
  initRobot();
  
  ledRandom(255/50);
  
  while(!button_is_pressed()) delay(10);
  
  ledRandom(255/10);
  
  playMusicTone1(30);
  
  ledColor(0,0,0); // Turn off LED to minimize light interference
  
  magnetometerToZero();
  
  iniTime = millis();
  last_timestamp = iniTime;
  while(1) {
    xbee.readPacket();
    if (xbee.getResponse().isAvailable()) {
      if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
        //ledColor(64,0,0);
        //delay(10);
        //ledColor(0,0,0);
        xbee.getResponse().getZBRxResponse(rx);
        int len = rx.getDataLength();
        if(len == 2) {
          char L = rx.getData(0);
          char R = rx.getData(1);
          if(L == 0) Servo1.detach();
          else {
            Servo1.attach(SERVO_1_PIN);
            Servo1.write(90-L);
          }
          if(R == 0) Servo2.detach();
          else {
            Servo2.attach(SERVO_2_PIN);
            Servo2.write(90+R);
          }
        }
      }
    }
    
    iniTime = millis();
    if(iniTime-100 > last_timestamp) {
      last_timestamp = iniTime;
      int batt = analogRead(BATTERY_PIN);
      int ldr1 = round(analogReadAverage(LDR1_PIN,8));
      int ldr2 = round(analogReadAverage(LDR2_PIN,8));
      int ldr3 = round(analogReadAverage(LDR3_PIN,8));
      int ldr4 = round(analogReadAverage(LDR4_PIN,8));
      //float angle = readMagnetometer();
      
      int irDistance = round(analogReadAverage(A8,8));//IR distance sensor
      
      MagnetometerRaw magnetometer_raw = compass.ReadRawAxis();
      char data[256] = "";
      sprintf(data,"B%d %d %d %d %d %d %d %d %d", batt,magnetometer_raw.XAxis,magnetometer_raw.YAxis,magnetometer_raw.ZAxis,ldr1,ldr2,ldr3,ldr4,irDistance);
      sendXbee(data);
    }
    
    if(button_is_pressed()) {
      int spinVel = 5;
      delay(500);
      if(button_is_pressed()) spinVel = 10;
      Servo1.attach(SERVO_1_PIN);
      Servo2.attach(SERVO_2_PIN);
      Servo1.write(90-spinVel);
      Servo2.write(90-spinVel);
      delay(500);
    }
    
  }

    
  /*
  // flash TX indicator
  flashLed(statusLed, 1, 100);

  // after sending a tx request, we expect a status response
  // wait up to half second for the status response
  if (xbee.readPacket(500)) {
    // got a response!

    // should be a znet tx status            	
    if (xbee.getResponse().getApiId() == ZB_TX_STATUS_RESPONSE) {
      xbee.getResponse().getZBTxStatusResponse(txStatus);

      // get the delivery status, the fifth byte
      if (txStatus.getDeliveryStatus() == SUCCESS) {
        // success.  time to celebrate
        flashLed(statusLed, 5, 50);
      } else {
        // the remote XBee did not receive our packet. is it powered on?
        flashLed(errorLed, 3, 500);
      }
    }
  } else if (xbee.getResponse().isError()) {
    //nss.print("Error reading packet.  Error code: ");  
    //nss.println(xbee.getResponse().getErrorCode());
  } else {
    // local XBee did not provide a timely TX Status Response -- should not happen
    flashLed(errorLed, 2, 50);
  }*/
    
    
    /*Serial.print("Heat: ");
    Serial.print(heatVal);
    Serial.print("\t Nose: ");
    Serial.print(noseV);
    
    Serial.print("\t LDR1: ");
    Serial.print(analogRead(LDR1_PIN));
    Serial.print("\t LDR2: ");
    Serial.print(analogRead(LDR2_PIN));
    Serial.print("\t LDR3: ");
    Serial.print(analogRead(LDR3_PIN));
    Serial.print("\t LDR4: ");
    Serial.print(analogRead(LDR4_PIN));
    
    Serial.print("\t Compass: ");
    Serial.print(readMagnetometer());
    
    Serial.print("\t Battery: ");
    Serial.print(battV);
    
    Serial.println("V END");
    
    
    if(button_is_pressed()) {
      if(!prevButton) {
        if(heatVal == 0)
          heatVal = 128;
        else if(heatVal == 128)
          heatVal = 255;
        else heatVal = 0;
        analogWrite(NOSE_HEAT_PIN, heatVal);
      }
      prevButton = 1;
    } else prevButton = 0;*/
    
    //ledRandom(255/50);
    //delay(10000);
    //while(noseV_1 > 55) {
    //  noseV_1 = analogRead(NOSE_VOUT_PIN);
    //  sprintf(data,"Batt: %d Nose: %d %d %d %d %d %d %d", battV,noseV_1,noseV_2,noseV_3,noseV_4,noseV_5,noseV_6,noseV_7);
    //  sendXbee(data);
      delay(500);
    //}
  //}
}

void loop() {
  
  readLine(buffer);
  if(strlen(buffer) > 0) {
    // Process commands
    char sep[] = ": ,";
    char *pch = strtok(buffer,sep);
    
    if(strcmp(pch,"Init") == 0) { // Init:TestString
      pch = strtok(NULL,sep);
      initRobot();
      delay(500);
      Serial.print("OK:");
      Serial.print(pch);
      Serial.print('\n');
      
    } else if(strcmp(pch,"Log") == 0) { // Log:[ON,OFF]
      pch = strtok(NULL,sep);
      if(strcmp(pch,"ON") == 0) {
        loggerMode = 1;
        iniTime = millis();
        last_timestamp = 0;
      } else {
        loggerMode = 0;
      }
      Serial.print("Log:");
      Serial.println(loggerMode);
    
    } else if(strcmp(pch,"Buzz") == 0) { // Buzz:N,freq,length
      int number = 1;
      int freq = DO;
      int length = 60;
      
      pch = strtok(NULL,sep);
      if(pch != NULL)
        number = atoi(pch);
      
      pch = strtok(NULL,sep);
      if(pch != NULL)
        freq = atoi(pch);
      
      pch = strtok(NULL,sep);
      if(pch != NULL)
        length = atoi(pch);
      Serial.println("Playing notes");
      if (freq <= 0) {
        playMusicScale(length);
      } else {
        for(int i=0; i<number; i++) 
          playNote(freq,length);
      }
    
    } else if(strcmp(pch,"LedRGB") == 0) { // LedRGB:R,G,B
      int R = 20;
      int G = 50;
      int B = -1;
      
      pch = strtok(NULL,sep);
      if(pch != NULL)
        R = atoi(pch);
      
      pch = strtok(NULL,sep);
      if(pch != NULL)
        G = atoi(pch);
      
      pch = strtok(NULL,sep);
      if(pch != NULL)
        B = atoi(pch);
      
      Serial.println("Led RGB");
      
      if(B < 0) {
        for(int i=0; i<R; i++) {
          ledRandom(G);
          delay(100);
        }
      } else {
        ledColor(R,G,B);
      }
    
    } else if(strcmp(pch,"Motor") == 0) { // Motor:L,R
      int L = 0;
      int R = 0;
      
      pch = strtok(NULL,sep);
      if(pch != NULL)
        L = atoi(pch);
      
      pch = strtok(NULL,sep);
      if(pch != NULL)
        R = atoi(pch);
      
      Serial.println("Motor");
      
      L += 90;
      R += 90;
      
      L = 180-L;
      

      while(motors_oldL != L || motors_oldR != R) {
        if(motors_oldL < L) motors_oldL++;
        else if(motors_oldL > L) motors_oldL--;
        Servo1.write(motors_oldL);
        
        if(motors_oldR < R) motors_oldR++;
        else if(motors_oldR > R) motors_oldR--;
        Servo2.write(motors_oldR);
        
        delay(50);
      }
      
      motors_oldL = L;
      motors_oldR = R;
      
    } else if(strcmp(pch,"GoToAngle") == 0) { // GoToAngle:Degrees
      float angleDest = 0;
      float margin = 5.f;
      
      Serial.println("Go to angle");
      
      pch = strtok(NULL,sep);
      if(pch != NULL)
        angleDest = atoi(pch);
      
      pch = strtok(NULL,sep);
      if(pch != NULL)
        margin = (float)atoi(pch);
      
      
      //float M_degrees = readMagnetometer();
      
      int spinVel = 10;
      //float error = angleDest-M_degrees;
      //if(abs(error) < 180.f) spinVel *= -1;
      //Servo1.write(90-spinVel);
      //Servo2.write(90-spinVel);
      
      float error = 2*margin;
      float M_degrees;
      
      while(abs(error) > margin) {
        M_degrees = readMagnetometer();
        error = angleDest-M_degrees;
        if(error < 0.f) error += 360.f;
        
        if(abs(error) > 180.f) {
          spinVel *= -1;
          spinVel = spinVel/2;
        }
        
        Servo1.write(90-spinVel);
        Servo2.write(90-spinVel);
        
        delay(10);
      }
      
      Servo1.write(90);
      Servo2.write(90);
      
      Serial.print("Dest:");
      Serial.print(angleDest);
      Serial.print(" M:");
      Serial.print(M_degrees);
      Serial.print(" Error:");
      Serial.println(error);
      
    } else if(strcmp(pch,"MagSetZero") == 0) { // MagSetZero
      magnetometerToZero();
      
    } else if(strcmp(pch,"MagSensorAngle") == 0) { // MagSensorAngle
      delay(300);
      float M_degrees = readMagnetometer();
      Serial.print("M:");
      Serial.println(M_degrees);
      Serial.print('\n');
    
    } else {
      Serial.print("ERROR\n");
    }
  }
  
  //ledRandom(255/100);
  //Servo1.write(servo1pos);
  //Servo2.write(servo2pos);
  
  //if(button_is_pressed()) {
  //  playMusicScale(60);
  //  playInvertedMusicScale(30);
  //}
  
  if(button_is_pressed()) {
    Serial.print("BUTTON PRESSED (+5s delay)\n\r");
    delay(5000);
  }
  
  if(loggerMode) {
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
  }
  
  servo1pos += servo1inc;
  servo2pos += servo2inc;
  int servodiff = 20;
  if(servo1pos > 90+servodiff || servo1pos < 90-servodiff) servo1inc *= -1;
  if(servo2pos > 90+servodiff || servo2pos < 90-servodiff) servo2inc *= -1;
  
  while( (millis()-iniTime-last_timestamp) < 100-8 ); // Sample period: ~100ms
  
  //delay(10);
 }
