// GNBoard Firmware by Carlosgs (http://carlosgs.es)
// License: CC-BY-SA (http://creativecommons.org/licenses/by-sa/3.0/)

#include <Servo.h>
#include <Wire.h> //I2C Arduino Library
#include <HMC5883L.h> //Compass library
#include <XBee.h> // XBee library (usage based on examples by Andrew Rapp)
#include <dht11.h>

// create the XBee object
XBee xbee = XBee();

// create the DHT11 object (sensor for temperature and humidity)
dht11 DHT11;

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

// IMU
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif
MPU6050 mpu;

// MPU control/status vars
bool dmpReady = false;  // set true if DMP init was successful
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
//uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
//uint16_t fifoCount;     // count of all bytes currently in FIFO
//uint8_t fifoBuffer[64]; // FIFO storage buffer

// orientation/motion vars
Quaternion q;           // [w, x, y, z]         quaternion container
VectorFloat gravity;    // [x, y, z]            gravity vector
float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

void setupIMU() {
    // join I2C bus (I2Cdev library doesn't do this automatically)
    #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
        Wire.begin();
        TWBR = 24; // 400kHz I2C clock (200kHz if CPU is 8MHz)
    #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
        Fastwire::setup(400, true);
    #endif
    
    
    mpu.initialize();
    
    delay(100);
    
    uint8_t devStatus = mpu.dmpInitialize(); //(0 = success, !0 = error)
    
    
    // Calibration offsets
    mpu.setXGyroOffset(34);
    mpu.setYGyroOffset(-26);
    mpu.setZGyroOffset(24);
//    mpu.setZAccelOffset(1788);
    
    
    if (devStatus == 0) {
        mpu.setRate(4); // 1khz / (1 + 4) = 200 Hz
        mpu.setFIFOEnabled(false);
        mpu.setDMPEnabled(true);
        mpuIntStatus = mpu.getIntStatus();
        // get expected DMP packet size for later comparison
        //packetSize = mpu.dmpGetFIFOPacketSize();
        dmpReady = true;
    }
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
  // Mine is: 2'' 37' W, which is 2.617 Degrees, or (which we need) 0.0456752665 radians, I will use 0.0457
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

int frequencies[] = {
  DO,RE,MI,FA,SOL,LA,SI};

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
unsigned long currTime;
unsigned long last_timestamp = 0;

unsigned long last_timestamp_DHT11 = 0;

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

float getDistanceCM() {
  float measurement = analogReadAverage(IR4_PIN,4);
  float ir_K = 4244.64;
  float ir_C = 37.28;
  return ir_K/(measurement-ir_C);
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
  } 
  else
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
void sendXbee(const char* command, int len){
  ZBTxRequest zbtx = ZBTxRequest(CoordinatorAddress, (uint8_t *)command, len);
  xbee.send(zbtx);
}

ZBRxResponse rx = ZBRxResponse();




unsigned int imu_yaw_tushort;
unsigned int imu_pitch_tushort;
unsigned int imu_roll_tushort;


void setup() {
  delay(500);

  randomSeed(analogRead(A4));
  ledRandom(255/50);

  //setupMagnetometer();
  setupIMU();

  Servo1.attach(SERVO_1_PIN);
  Servo2.attach(SERVO_2_PIN);
  Serial.begin(115200);

  XbeeSerial.begin(9600);
  xbee.setSerial(XbeeSerial);

  initRobot();

  ledRandom(255/50);

  Servo1.detach();
  Servo2.detach();

  pinMode(NOSE_HEAT_PIN, OUTPUT);
  digitalWrite(NOSE_HEAT_PIN, HIGH);


  //while(!button_is_pressed()) delay(10);

  ledRandom(255/10);

  //playMusicTone1(30);

  ledColor(0,0,128);

  //magnetometerToZero();

  iniTime = millis();
  last_timestamp = iniTime;
  last_timestamp_DHT11 = iniTime;
  
  
  while(!button_is_pressed());
  
  //if(button_is_pressed()) {
    setupIMU();
    int velocity = 0;
    ledColor(128,0,0);
    delay(3000);
    
    int avgL_log[4];
    int avgR_log[4];
    float avgSpeed_log[4];
    
    //while(1) {
    for(int i=0;i<4;i++) {
      if(i == 0) velocity = 50;//50
      if(i == 1) velocity = -50;
      if(i == 2) velocity = 168;//168
      if(i == 3) velocity = -168;
      
      float avgSpeed = 0;
      iniTime = millis();
      float oldDistance = getDistanceCM();
      Servo1.attach(SERVO_1_PIN);
      Servo2.attach(SERVO_2_PIN);
      int avgL = 1467-velocity;
      int avgR = 1467+velocity;
      int integral_error = 0;
      while(1) {
        if(dmpReady) {
          //mpu.resetFIFO();
          //fifoCount = mpu.getFIFOCount();
          // wait for correct available data length, should be a VERY short wait
          //while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount();
          
          // read a packet from FIFO
          //mpu.getFIFOBytes(fifoBuffer, packetSize);
          
          mpu.dmpGetQuaternion(&q, fifoBuffer);
          mpu.dmpGetGravity(&gravity, &q);
          mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
        }
        
        currTime = millis();
        float distance = getDistanceCM();
        if((velocity > 0) && (distance < 8)) break;
        if((velocity < 0) && (distance > 38)) break;
        if((currTime-iniTime) > 90) {
        avgSpeed = avgSpeed*0.8+0.2*1000*(distance-oldDistance)/(currTime-iniTime);
        iniTime = currTime;
        oldDistance = distance;
        }
        
        float yaw_normalized = ypr[0]/M_PI;
        int error = round(max(min(yaw_normalized*800*2,800),-800));
        int L = 1467-(velocity-error-integral_error/10);
        int R = 1467+(velocity+error+integral_error/10);
        Servo1.writeMicroseconds(L);
        Servo2.writeMicroseconds(R);
        avgL = avgL*0.9+L*0.1;
        avgR = avgR*0.9+R*0.1;
        
        integral_error = integral_error + error;
        
//        Serial.print(avgL);
//        Serial.print("\t");
//        Serial.print(avgR);
//        Serial.print("\t");
//        Serial.println(avgSpeed);
        
        delay(100);
      }
      Servo1.detach();
      Servo2.detach();
      delay(500);
      
      avgL_log[i] = avgL;
      avgR_log[i] = avgR;
      avgSpeed_log[i] = avgSpeed;
      
    }
    
    ledColor(0,128,0);
    
    float speed_list[] = {10,-10, 4,-4, 7,-7, 11,-11, 2,-2};
    
    float desiredSpeed = 0;
    int Lval = 0;
    int Rval = 0;
    
    int curr = 0;
    while(1) {
      if(button_is_pressed()) {
        delay(500);
        
        Servo1.attach(SERVO_1_PIN);
        Servo2.attach(SERVO_2_PIN);
        
        for(int i=-16;i<=16;i=i+2) {
          if((i>=-4) && (i<=4)) {
            ledColor(0,0,128);
            continue;
          }

        
        float desiredSpeed = i;//speed_list[curr];
        
        if(desiredSpeed > 0) {
            Lval = round(mapf(desiredSpeed, avgSpeed_log[0],avgSpeed_log[2], avgL_log[0],avgL_log[2]));
            Rval = round(mapf(desiredSpeed, avgSpeed_log[0],avgSpeed_log[2], avgR_log[0],avgR_log[2]));
        } else {
            Lval = round(mapf(desiredSpeed, avgSpeed_log[3],avgSpeed_log[1], avgL_log[3],avgL_log[1]));
            Rval = round(mapf(desiredSpeed, avgSpeed_log[3],avgSpeed_log[1], avgR_log[3],avgR_log[1]));
        }
        
        Servo1.writeMicroseconds(Lval);
        Servo2.writeMicroseconds(Rval);
        
        delay(2000);
        
        /*curr++;
        if(curr >= sizeof(speed_list)/sizeof(speed_list[0])) {
            curr = 0;
            ledColor(0,0,128);
        }*/
        }
        Servo1.detach();
        Servo2.detach();
      }
    }
    
  //}
}

int sampleTime = 3000;

bool buttonHasBeenPressed = 0;

int battV = 0;
int battV_min = 1023;
int battV_max = 0;

int noseV = 0;
int noseV_min = 1023;
int noseV_max = 0;

int irDistance = 0;
int irDistance_min = 1023;
int irDistance_max = 0;

int temperature = -1;
int humidity = -1;



int dht11_count = 0;
void loop() {
  xbee.readPacket();
  if (xbee.getResponse().isAvailable()) {
    if (xbee.getResponse().getApiId() == ZB_RX_RESPONSE) {
      //ledColor(64,0,0);
      //delay(10);
      //ledColor(0,0,0);
      xbee.getResponse().getZBRxResponse(rx);
      int datalen = rx.getDataLength();
      
      if(datalen > 1) {
      char command = rx.getData(0);
      if(command == 0) { // PUT command
        int N_values = (datalen-1)/3;
        for(int i=0; i<N_values; i++) {
          int istart = i*3+1;
          char valType = rx.getData(istart);
          int value = (short)(rx.getData(istart+1)<<8|rx.getData(istart+2));
          if(valType==6) Serial.println(value);
          int L, R, freq;
          switch ( valType ) {
          case 1: // ledR_PWM
            analogWrite(LED_R_PIN, value);
            break;
          case 2: // ledG_PWM
            analogWrite(LED_G_PIN, value);
            break;
          case 3: // ledB_PWM
            analogWrite(LED_B_PIN, value);
            break;
          case 4: // motorL
            L = value;
            if(L == 0) Servo1.detach();
            else {
              Servo1.attach(SERVO_1_PIN);
              Servo1.write(90-L);
            }
            break;
          case 5: // motorR
            R = value;
            if(R == 0) Servo2.detach();
            else {
              Servo2.attach(SERVO_2_PIN);
              Servo2.write(90+R);
            }
            break;
          case 6: // tone
            freq = value;
            tone(BUZZER_PIN,freq);
            break;
          case 7: // noseHeater_PWM
            analogWrite(NOSE_HEAT_PIN, value);
            break;
          case 14: // notone
            noTone(BUZZER_PIN);
            delay(value);
            break;
          case 15: // delay
            delay(value);
            break;
          case 16: // toneMs
            // Not implemented
            break;
          case 23: // sampletime
            sampleTime = value;
            break;
          default:
            break;
          }
        }
      }
      }
      
/*      if(len == 2) { // set motors*/
/*        char L = rx.getData(0);*/
/*        char R = rx.getData(1);*/
/*        if(L == 0) Servo1.detach();*/
/*        else {*/
/*          Servo1.attach(SERVO_1_PIN);*/
/*          Servo1.write(90-L);*/
/*        }*/
/*        if(R == 0) Servo2.detach();*/
/*        else {*/
/*          Servo2.attach(SERVO_2_PIN);*/
/*          Servo2.write(90+R);*/
/*        }*/
/*      } else if(len >= 4) {*/
/*        unsigned int value = rx.getData(2);*/
/*        if(value != 0) { // play note (1-> DO, 1-> RE... etc)*/
/*          int note = value;*/
/*          int freq = frequencies[(note-1)%7]*(1+(note-1)/7);*/
/*          int len = 10*(int)rx.getData(3);*/
/*          //playNote(freq, len);*/
/*          tone(BUZZER_PIN,freq);*/
/*          delay(len);*/
/*          noTone(BUZZER_PIN);*/
/*        } */
/*        else if(len == 5) { // set LED color*/
/*          ledColor(rx.getData(2), rx.getData(3), rx.getData(4));*/
/*        }*/
/*      }*/
    }
  }

  battV = analogRead(BATTERY_PIN);
  if(battV > battV_max) battV_max = battV;
  if(battV < battV_min) battV_min = battV;

  if(!buttonHasBeenPressed) buttonHasBeenPressed = button_is_pressed();

  noseV = analogRead(NOSE_VOUT_PIN);
  if(noseV > noseV_max) noseV_max = noseV;
  if(noseV < noseV_min) noseV_min = noseV;

  irDistance = analogRead(IR4_PIN);
  if(irDistance > irDistance_max) irDistance_max = irDistance;
  if(irDistance < irDistance_min) irDistance_min = irDistance;

  iniTime = millis();
  if(sampleTime > 0 && iniTime-sampleTime > last_timestamp) {
    last_timestamp = iniTime;

    //MagnetometerRaw magnetometer_raw = compass.ReadRawAxis();

    if(dmpReady) {
        //mpuIntStatus = mpu.getIntStatus();
        //fifoCount = mpu.getFIFOCount();
        //if (mpuIntStatus & 0x02) {
            //mpu.resetFIFO();
            //fifoCount = mpu.getFIFOCount();
            // wait for correct available data length, should be a VERY short wait
            //while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount();
            
            // read a packet from FIFO
            //mpu.getFIFOBytes(fifoBuffer, packetSize);
            
            // track FIFO count here in case there is > 1 packet available
            // (this lets us immediately read more without waiting for an interrupt)
            //fifoCount -= packetSize;
            
            mpu.dmpGetQuaternion(&q, fifoBuffer);
            mpu.dmpGetGravity(&gravity, &q);
            mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
            
            imu_yaw_tushort = round(mapf(ypr[0],-M_PI,M_PI,0,65535));
            imu_pitch_tushort = round(mapf(ypr[1],-M_PI,M_PI,0,65535));
            imu_roll_tushort = round(mapf(ypr[2],-M_PI,M_PI,0,65535));
        //}
    }

    char data[256] = "";
    int pos = 0;
    
    data[pos] = 0; // PUT command
    pos += 1;
    
    data[pos] = 8; // Type: noseMax
    data[pos+1] = noseV_max >> 8;
    data[pos+2] = noseV_max & 0x00ff;
    pos += 3;
    
    data[pos] = 9; // Type: noseMin
    data[pos+1] = noseV_min >> 8;
    data[pos+2] = noseV_min & 0x00ff;
    pos += 3;
    
    data[pos] = 10; // Type: distMax
    data[pos+1] = irDistance_max >> 8;
    data[pos+2] = irDistance_max & 0x00ff;
    pos += 3;
    
    data[pos] = 11; // Type: distMin
    data[pos+1] = irDistance_min >> 8;
    data[pos+2] = irDistance_min & 0x00ff;
    pos += 3;
    
    data[pos] = 12; // Type: batteryMax
    data[pos+1] = battV_max >> 8;
    data[pos+2] = battV_max & 0x00ff;
    pos += 3;
    
    data[pos] = 13; // Type: batteryMin
    data[pos+1] = battV_min >> 8;
    data[pos+2] = battV_min & 0x00ff;
    pos += 3;
    
    data[pos] = 17; // Type: humidity
    data[pos+1] = humidity >> 8;
    data[pos+2] = humidity & 0x00ff;
    pos += 3;
    
    data[pos] = 18; // Type: temperature
    data[pos+1] = temperature >> 8;
    data[pos+2] = temperature & 0x00ff;
    pos += 3;
    
/*    data[pos] = 19; // Type: magnetometerX*/
/*    data[pos+1] = magnetometer_raw.XAxis >> 8;*/
/*    data[pos+2] = magnetometer_raw.XAxis & 0x00ff;*/
/*    pos += 3;*/
/*    */
/*    data[pos] = 20; // Type: magnetometerY*/
/*    data[pos+1] = magnetometer_raw.YAxis >> 8;*/
/*    data[pos+2] = magnetometer_raw.YAxis & 0x00ff;*/
/*    pos += 3;*/
/*    */
/*    data[pos] = 21; // Type: magnetometerZ*/
/*    data[pos+1] = magnetometer_raw.ZAxis >> 8;*/
/*    data[pos+2] = magnetometer_raw.ZAxis & 0x00ff;*/
/*    pos += 3;*/
    
    data[pos] = 22; // Type: button
    data[pos+1] = buttonHasBeenPressed >> 8;
    data[pos+2] = buttonHasBeenPressed & 0x00ff;
    pos += 3;
    
    data[pos] = 24; // Type: IMUyaw
    data[pos+1] = imu_yaw_tushort >> 8;
    data[pos+2] = imu_yaw_tushort & 0x00ff;
    pos += 3;
    
    data[pos] = 25; // Type: IMUpitch
    data[pos+1] = imu_pitch_tushort >> 8;
    data[pos+2] = imu_pitch_tushort & 0x00ff;
    pos += 3;
    
    data[pos] = 26; // Type: IMUroll
    data[pos+1] = imu_roll_tushort >> 8;
    data[pos+2] = imu_roll_tushort & 0x00ff;
    pos += 3;
    
    //sprintf(data,"%d %d %d %d %d %d %d", batt,noseV_min,noseV_max,irDistance_min,irDistance_max, temperature, humidity);
    sendXbee(data, pos+1);

    battV_min = 1023;
    battV_max = 0;
    noseV_min = 1023;
    noseV_max = 0;
    irDistance_min = 1023;
    irDistance_max = 0;
    buttonHasBeenPressed = 0;
  }

  if(iniTime-1500 > last_timestamp_DHT11) {
    last_timestamp_DHT11 = iniTime;
    int chk = DHT11.read(DHT_PIN);
    if(chk == DHTLIB_OK) {
      humidity = DHT11.humidity; // % humidity
      temperature = DHT11.temperature; // degrees celsius
    }
  }

  /**if(button_is_pressed()) {
    int spinVel = 5;
    delay(500);
    if(button_is_pressed()) spinVel = 10;
    Servo1.attach(SERVO_1_PIN);
    Servo2.attach(SERVO_2_PIN);
    Servo1.write(90-spinVel);
    Servo2.write(90-spinVel);
    delay(500);
  }**/
}


