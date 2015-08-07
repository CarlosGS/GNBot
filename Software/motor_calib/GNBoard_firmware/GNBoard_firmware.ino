// GNBoard Firmware by Carlosgs (http://carlosgs.es)
// License: CC-BY-SA (http://creativecommons.org/licenses/by-sa/3.0/)

#include <Servo.h>
#include <Wire.h> //I2C Arduino Library
#include <HMC5883L.h> //Compass library
#include <XBee.h> // XBee library (usage based on examples by Andrew Rapp)
#include <dht11.h>
#include <EEPROM.h>

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




// Structures that store the calibration parameters
#define CALIB_EEPROM_ADDR 20
#define CALIB_OK_VAL 77 // Must be different from 0
struct MotorParamsOneDir {
    float maxSpeed;
    int maxSpeed_PWM;
    int zeroSpeed_PWM;
};
struct MotorParams {
    MotorParamsOneDir cw;
    MotorParamsOneDir ccw;
};
struct CalibrationParameters {
    char ok; // Stores CALIB_OK_VAL if the parameters are correct
    MotorParams servo1;
    MotorParams servo2;
    float maxWspeed;
    float speed_k;
    float kp;
    float ki;
    float kd;
};
CalibrationParameters calib;


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
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer

// orientation/motion vars
Quaternion q;           // [w, x, y, z]         quaternion container
VectorFloat gravity;    // [x, y, z]            gravity vector
float ypr[3];           // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector

int setupIMU() {
    // join I2C bus (I2Cdev library doesn't do this automatically)
    #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
      Wire.begin();
      TWBR = 24; // 400kHz I2C clock (200kHz if CPU is 8MHz)
    #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
      Fastwire::setup(400, true);
    #endif

    mpu.initialize();
    mpu.setZGyroOffset(0);
    
    delay(500);
    
    int16_t ax, ay, az;
    int16_t gx, gy, gz;
    int gzf = 0; // Filtered Z gyro measurement
    for(int i=0; i<16; i++) {
        mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
        gzf += (int)gz;
        delay(100);
    }
    gzf = round(((float)gzf)/16.);

    uint8_t devStatus = mpu.dmpInitialize(); //(0 = success, !0 = error)

    if (devStatus == 0) {
        // Apply the newly-calibrated offset
        mpu.setZGyroOffset(-gzf);

        //mpu.setRate(4); // 1khz / (1 + 4) = 200 Hz
        //mpu.setMotionDetectionThreshold(0);
        //mpu.setMotionDetectionDuration(0);
        //mpu.setZeroMotionDetectionThreshold(0);

        mpu.setDMPEnabled(true);
        mpuIntStatus = mpu.getIntStatus();
        // Get the expected DMP packet size for later comparison
        packetSize = mpu.dmpGetFIFOPacketSize();
        dmpReady = true;
    }
    return devStatus;
}

void readIMU_YawPitchRoll(float *data) {
    mpu.resetFIFO();

    // Wait for correct available data length
    fifoCount = mpu.getFIFOCount();
    while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount();

    // Read a packet from FIFO
    mpu.getFIFOBytes(fifoBuffer, packetSize);

    mpu.dmpGetQuaternion(&q, fifoBuffer);
    mpu.dmpGetGravity(&gravity, &q);
    mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);
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

int notes[] = {DO,RE,MI,FA,SOL,LA,SI};

void playNote(int frequency, int length) {
    if(frequency <= 0) frequency = DO;
    if(length <= 0) length = 100;
    tone(BUZZER_PIN,frequency);
    delay(length);
    noTone(BUZZER_PIN);
    delay(100);
}

void playMusicScale(int length) {
    for(int i=0; i<7; i++) playNote(notes[i], length);
}

void playInvertedMusicScale(int length) {
    for(int i=6; i>=0; i--) playNote(notes[i], length);
}


// --- BUTTON ---

void init_button_pin() {
    pinMode(BUTTON_1_PIN,INPUT);
    digitalWrite(BUTTON_1_PIN, HIGH); // Enable internal pull-up resistor
}

// Returns 1 if the button is being pressed, 0 if it is not
int button_is_pressed() {
    return !digitalRead(BUTTON_1_PIN);
}

void wait_for_button_press() {
    while(!button_is_pressed());
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






float analogReadAverage(int pin, int samples) {
    float result = 0;
    for(int i=0; i<samples; i++) {
        result += analogRead(pin);
    }
    return result/(float)samples;
}

float getDistanceCM() {
    float measurement = analogReadAverage(IR4_PIN,4);
    float ir_K = 4172.59;
    float ir_C = 43.74;
    if(measurement <= ir_C) return 150;
    float res = ir_K/(measurement-ir_C);
    if(res < 0 || res > 150) res = 150;
    return res;
}

float getBatteryVoltage() {
    int val = analogRead(BATTERY_PIN);
    return mapf(val,0,1023, 0,21.1765); // Voltage divider with 22k in series with 6k8
}

char inBuffer[128] = "";
int inBuffer_len = 0;

void readLine(char *out) {
    boolean gotLine = false;
    while(Serial.available()) {
        char val = Serial.read();
        if(val == '\n') {
            gotLine = true;
            break;
        }
        if(val != '\r') {
            inBuffer[inBuffer_len] = val;
            inBuffer_len++;
        }
    }
    if(gotLine) {
        inBuffer[inBuffer_len] = '\0';
        strcpy(out,inBuffer);
        inBuffer_len = 0;
    } else *out = '\0';
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
        if (i + 1 < times) delay(wait);
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






boolean Servo1_active = false;
void set_servo1_rot_speed(float omega) {
    if(abs(omega) < 0.001) {
        if(Servo1_active) {
            Servo1.detach();
            Servo1_active = false;
        }
    } else {
        if(!Servo1_active) {
            Servo1.attach(SERVO_1_PIN);
            Servo1_active = true;
        }
        if(omega > 0) Servo1.writeMicroseconds(round(mapf(omega,0,calib.servo1.cw.maxSpeed,calib.servo1.cw.zeroSpeed_PWM,calib.servo1.cw.maxSpeed_PWM)));
        else          Servo1.writeMicroseconds(round(mapf(omega,0,calib.servo1.ccw.maxSpeed,calib.servo1.ccw.zeroSpeed_PWM,calib.servo1.ccw.maxSpeed_PWM)));
    }
}

boolean Servo2_active = false;
void set_servo2_rot_speed(float omega) {
    if(abs(omega) < 0.001) {
        if(Servo2_active) {
            Servo2.detach();
            Servo2_active = false;
        }
    } else {
        if(!Servo2_active) {
            Servo2.attach(SERVO_2_PIN);
            Servo2_active = true;
        }
        if(omega > 0) Servo2.writeMicroseconds(round(mapf(omega,0,calib.servo2.cw.maxSpeed,calib.servo2.cw.zeroSpeed_PWM,calib.servo2.cw.maxSpeed_PWM)));
        else          Servo2.writeMicroseconds(round(mapf(omega,0,calib.servo2.ccw.maxSpeed,calib.servo2.ccw.zeroSpeed_PWM,calib.servo2.ccw.maxSpeed_PWM)));
    }
}


void audio_from_string(char *str, int ms, int led) {
    ledColor(0,0,0);
    while(*str) {
        if(*str >= 33 && *str <= 126) { // Only visible ASCII characters
            int note = round(mapf(*str,33,126,DO/2,DO*4));
            digitalWrite(led, HIGH);
            tone(BUZZER_PIN,note);
            delay(ms);
            noTone(BUZZER_PIN);
            digitalWrite(led, LOW);
            delay(ms/10);
        }
        if(*str == 32) {
            ledColor(0,0,0);
            delay(ms*2); // Space
        }
        str++;
    }
}



void unrecoverable_error(char *msg) {
    Serial.print("ERROR: ");
    Serial.println(msg);
    
    playNote(DO, 1000);
    audio_from_string(msg,50,LED_R_PIN);
    while(1) {
        ledColor(0,0,0);
        delay(500);
        ledColor(128,0,0);
        delay(500);
    }
}

void message(char *msg) {
    Serial.print("MESSAGE: ");
    Serial.println(msg);
    audio_from_string(msg,50,LED_B_PIN);
}




int motorPIDcontroller(float yawGoal_rad, boolean term_yawReached, float c_speed_rads, float IRdistGoal_cm, boolean term_IRdistReached, float yawEnd_rad, float distanceGoal_rad, boolean term_distanceReached) {
    float prev_error = 0;
    boolean saturated = false;
    boolean first_iteration = true;
    float error_integral = 0;
    float error_derivative = 0;
    unsigned long ts, prev_ts;
    float distance_integral = 0;
    while(1) {
      readIMU_YawPitchRoll(ypr);
      ts = millis();
      float dt = (float)(ts-prev_ts)/1000.;
      float targetYaw = yawGoal_rad;
      if(term_distanceReached) targetYaw = mapf(distance_integral,0,distanceGoal_rad,yawGoal_rad,yawEnd_rad);
      float yaw = (ypr[0]-targetYaw);
      while(yaw > M_PI) yaw -= 2.*M_PI;
      while(yaw <= -M_PI) yaw += 2.*M_PI;
      float error = -yaw;
      
      if(!first_iteration) {
        if(!saturated) error_integral += prev_error*dt;
        error_derivative = (error-prev_error)/dt;
        distance_integral += c_speed_rads*dt;
      } else first_iteration = false;
      float v = calib.kp*error + calib.ki*error_integral + calib.kd*error_derivative;
      
      if(abs(v)>calib.maxWspeed) {
        saturated = true;
      } else {
        if(saturated) error_integral = 0;
        saturated = false;
      }
      
      //if(v > calib.maxWspeed) v = calib.maxWspeed;
      //if(v < -calib.maxWspeed) v = -calib.maxWspeed;
      
      set_servo1_rot_speed(v+c_speed_rads);
      set_servo2_rot_speed(v-c_speed_rads);

      if(term_yawReached && abs(error)+abs(error_integral)+abs(error_derivative) < 0.5*M_PI/180.) return 1;
      if(term_IRdistReached) {
          float dist = getDistanceCM();
          if(c_speed_rads > 0) {
              if(dist < IRdistGoal_cm) return 2;
          } else {
              if(dist > IRdistGoal_cm) return 2;
          }
      }
      if(term_distanceReached) {
          if(c_speed_rads > 0) {
              if(distance_integral > distanceGoal_rad) return 3;
          } else {
              if(distance_integral < distanceGoal_rad) return 3;
          }
      }
      
      //delay(10);
      
      prev_error = error;
      prev_ts = ts;
    }
    return 0;
}

void pointToAngle(float yawGoal_rad) {
    motorPIDcontroller(yawGoal_rad, true, 0, 0, false, 0, 0, false);
    set_servo1_rot_speed(0);
    set_servo2_rot_speed(0);
}



void performSquare(float len_cm, float vel_cms) {
    readIMU_YawPitchRoll(ypr);
    float yaw = ypr[0];
    float len = len_cm*calib.speed_k;
    float vel = vel_cms*calib.speed_k;
    motorPIDcontroller(yaw, false, vel, 0, false, yaw, len, true);
    set_servo1_rot_speed(0);
    set_servo2_rot_speed(0);
    delay(500);
    
    yaw += 90.*M_PI/180.;
    pointToAngle(yaw);
    motorPIDcontroller(yaw, false, vel, 0, false, yaw, len, true);
    set_servo1_rot_speed(0);
    set_servo2_rot_speed(0);
    delay(500);
    
    yaw += 90.*M_PI/180.;
    pointToAngle(yaw);
    motorPIDcontroller(yaw, false, vel, 0, false, yaw, len, true);
    set_servo1_rot_speed(0);
    set_servo2_rot_speed(0);
    delay(500);
    
    yaw += 90.*M_PI/180.;
    pointToAngle(yaw);
    motorPIDcontroller(yaw, false, vel, 0, false, yaw, len, true);
    set_servo1_rot_speed(0);
    set_servo2_rot_speed(0);
    delay(500);
    
    yaw += 90.*M_PI/180.;
    pointToAngle(yaw);
}


void performArc(float len_cm, float vel_cms, float angle_deg) {
    readIMU_YawPitchRoll(ypr);
    float yaw = ypr[0];
    float len = len_cm*calib.speed_k;
    float vel = vel_cms*calib.speed_k;
    float angle = angle_deg*M_PI/180.;
    motorPIDcontroller(yaw, false, vel, 0, false, yaw+angle, len, true);
}





void setup() {
    delay(100);

    ledColor(0,0,128);

    int ret = setupIMU();
    unsigned long prev_ts = millis();
    unsigned long ts;

    //setupMagnetometer();
    //setupIMU();

    //Servo1.attach(SERVO_1_PIN);
    //Servo2.attach(SERVO_2_PIN);
    Serial.begin(115200);
    
    XbeeSerial.begin(9600);
    xbee.setSerial(XbeeSerial);

    init_button_pin();

    //Servo1.detach();
    //Servo2.detach();

    pinMode(NOSE_HEAT_PIN, OUTPUT);
    digitalWrite(NOSE_HEAT_PIN, HIGH);


    randomSeed(analogRead(BATTERY_PIN));
    
    delay(100);
    
    
   /*while(1) {
      float dist = 0;
      for(int i=0; i<100; i++) {
        dist += getDistanceCM();
        delay(1);
      }
      dist /= 100;
      Serial.println(dist);
    }*/
    
    
    // Low battery notification (program will stop here if 16.5V are not available)
    if(getBatteryVoltage() < 16.5)
      unrecoverable_error("Battery voltage is too low");

    //while(!button_is_pressed()) delay(10);

    //ledRandom(255/10);

    //playMusicTone1(30);

    //magnetometerToZero();

    iniTime = millis();
    last_timestamp = iniTime;
    last_timestamp_DHT11 = iniTime;
    
    // For testing raw PWM values
    /*Servo1.attach(SERVO_1_PIN);
    Servo2.attach(SERVO_2_PIN);
    Servo1.writeMicroseconds(1377); // LEFT
    Servo2.writeMicroseconds(1568); // RIGHT
    delay(1000);
    Servo1.detach();
    Servo2.detach();*/


    //delay(1000);
    //int ret = setupIMU();
    //unsigned long prev_ts = millis();
    //unsigned long ts;
    
    // For evaluating gyro drift
    /*while(1) {
      readIMU_YawPitchRoll(ypr);
      Serial.print("[");
      Serial.print(millis());
      Serial.print(",");
      Serial.print(ypr[0],5);
      Serial.println("],");
      delay(100);
    }*/
    
    boolean do_calibration = false;
    
    
    // Read calibration parameters from EEPROM
    EEPROM.get(CALIB_EEPROM_ADDR, calib);
    // Check whether the parameters are valid or not
    if(calib.ok != CALIB_OK_VAL) do_calibration = true;
    
    if(ret == 0) {
      // OK --> Blink green LED + stabilization delay
      Serial.println("Gyroscope calibration");
      readIMU_YawPitchRoll(ypr);
      float prevYaw = ypr[0];
      float rotSpeed_avg = 10;
      prev_ts = millis();
      while(rotSpeed_avg > 0.5) {
        readIMU_YawPitchRoll(ypr);
        ts = millis();
        float rotSpeed = (ypr[0]-prevYaw);
        while(rotSpeed > M_PI) rotSpeed -= 2*M_PI;
        while(rotSpeed <= -M_PI) rotSpeed += 2*M_PI;
        rotSpeed /= ((float)(ts-prev_ts))/1000.;
        rotSpeed_avg = 0.6*rotSpeed_avg + 0.4*abs(rotSpeed*60.*180./M_PI);
        prev_ts = ts;
        prevYaw = ypr[0];
        ledColor(0,64,0);
        delay(150);
        if(button_is_pressed()) do_calibration = true;
        if(do_calibration) ledColor(0,0,64);
        else ledColor(0,16,0);
        delay(50);
        Serial.println(rotSpeed_avg);
      }
      ledColor(0,128,0);
    } 
    else
      unrecoverable_error("Could not initialize IMU");
     
     /*
     while(!button_is_pressed());
     
     Servo1.writeMicroseconds(1000);
     
     float prevYaw_ = 0;
     float prevRotSpeed_ = 0;
     float rotSpeed_ = 0;
     while(1) {
     readIMU_YawPitchRoll(ypr);
     prevRotSpeed_ = rotSpeed_;
     rotSpeed_ = ypr[0]-prevYaw_;
     while(rotSpeed_ > M_PI) rotSpeed_ -= 2*M_PI;
     while(rotSpeed_ <= -M_PI) rotSpeed_ += 2*M_PI;
     rotSpeed_ /= 0.05;
     prevYaw_ = ypr[0];
     Serial.println(rotSpeed_);
     delay(50);
     }*/

    //while(!button_is_pressed());

    //delay(1000);


    readIMU_YawPitchRoll(ypr);
    float initialHeading = ypr[0];
    
    float initialDistance = 8;//getDistanceCM();
    
    // Motor PWM sweep logging
    /*while(1) {
      char data[256] = "";
      int pos = 0;
      data[pos] = 0; // PUT command
      pos += 1;
      sendXbee(data, pos+1);
      delay(200);
      if(button_is_pressed()) break;
    }
    playNote(DO, 100);
    delay(1000);
    
    Servo2.attach(SERVO_2_PIN);
    Servo2.writeMicroseconds(1000);
    delay(500);
    readIMU_YawPitchRoll(ypr);
    float rotSpeed = 0;
    //float rotSpeedFilt = 0;
    float prevYaw = ypr[0];
    unsigned long prev_ts = millis();
    unsigned long ts = prev_ts;
    for(int i=0; i<1000; i++) {
      delay(2*2*10);
      readIMU_YawPitchRoll(ypr);
      
      rotSpeed = ypr[0]-prevYaw;
      while(rotSpeed > M_PI) rotSpeed -= 2*M_PI;
      while(rotSpeed <= -M_PI) rotSpeed += 2*M_PI;
      ts = millis();
      rotSpeed /= ((float)(ts-prev_ts))/1000.;
      prev_ts = ts;
      prevYaw = ypr[0];
      
      //rotSpeedFilt = rotSpeed*0.8 + rotSpeed*0.2;
      
      int motorPulse = 1000+i;
      Servo2.writeMicroseconds(motorPulse);
      
      if( (i%3) == 0 ) {
          unsigned int avgSpeed_tushort = round(mapf(rotSpeed,-10,10,0,65535));
          
          char data[256] = "";
          int pos = 0;
          
          data[pos] = 0; // PUT command
          pos += 1;
          
          data[pos] = 27; // Type: average speed
          data[pos+1] = avgSpeed_tushort >> 8;
          data[pos+2] = avgSpeed_tushort & 0x00ff;
          pos += 3;
          
          data[pos] = 29; // Type: average R motor input
          data[pos+1] = motorPulse >> 8;
          data[pos+2] = motorPulse & 0x00ff;
          pos += 3;
          
          sendXbee(data, pos+1);
      }
    }
    Servo2.detach();
    
    
    delay(1000);
    
    
    Servo1.attach(SERVO_1_PIN);
    Servo1.writeMicroseconds(1000);
    delay(1000);
    readIMU_YawPitchRoll(ypr);
    rotSpeed = 0;
    prevYaw = ypr[0];
    prev_ts = millis();
    ts = prev_ts;
    for(int i=0; i<1000; i++) {
      delay(2*2*10);
      readIMU_YawPitchRoll(ypr);
      
      rotSpeed = ypr[0]-prevYaw;
      while(rotSpeed > M_PI) rotSpeed -= 2*M_PI;
      while(rotSpeed <= -M_PI) rotSpeed += 2*M_PI;
      ts = millis();
      rotSpeed /= ((float)(ts-prev_ts))/1000.;
      prev_ts = ts;
      prevYaw = ypr[0];
      
      //rotSpeedFilt = rotSpeed*0.8 + rotSpeed*0.2;
      
      int motorPulse = 1000+i;
      Servo1.writeMicroseconds(motorPulse);
      
      if( (i%3) == 0 ) {
          unsigned int avgSpeed_tushort = round(mapf(rotSpeed,-10,10,0,65535));
          
          char data[256] = "";
          int pos = 0;
          
          data[pos] = 0; // PUT command
          pos += 1;
          
          data[pos] = 27; // Type: average speed
          data[pos+1] = avgSpeed_tushort >> 8;
          data[pos+2] = avgSpeed_tushort & 0x00ff;
          pos += 3;
          
          data[pos] = 28; // Type: average L motor input
          data[pos+1] = motorPulse >> 8;
          data[pos+2] = motorPulse & 0x00ff;
          pos += 3;
          
          sendXbee(data, pos+1);
      }
    }
    Servo1.detach();
    
    while(1);*/



    if(do_calibration) {

        Serial.println("Motor calibration. Minimum speeds:");

        Servo1.attach(SERVO_1_PIN);
        int Servo1_minPulseA = 1472;
        Servo1.writeMicroseconds(Servo1_minPulseA);
        readIMU_YawPitchRoll(ypr);
        float prevYaw = ypr[0];
        float rotSpeed = 0;
        prev_ts = millis();
        while(abs(rotSpeed) < 0.15) {
          readIMU_YawPitchRoll(ypr);
          rotSpeed = (ypr[0]-prevYaw);
          ts = millis();
          rotSpeed /= ((float)(ts-prev_ts))/1000.;
          prev_ts = ts;
          prevYaw = ypr[0];
          Servo1.writeMicroseconds(Servo1_minPulseA);
          Servo1_minPulseA++;
          delay(50);
        }
        float Servo1_minSpeedA = rotSpeed;
        Servo1.detach();
        playNote(DO, 100);
        ledColor(0,0,128);
        Serial.print("Servo1_A\t");
        Serial.print(Servo1_minSpeedA,5);
        Serial.print(" \t--->\t");
        Serial.println(Servo1_minPulseA);

        delay(100);




        Servo1.attach(SERVO_1_PIN);
        int Servo1_minPulseB = 1472;
        Servo1.writeMicroseconds(Servo1_minPulseB);
        readIMU_YawPitchRoll(ypr);
        prevYaw = ypr[0];
        rotSpeed = 0;
        prev_ts = millis();
        while(abs(rotSpeed) < 0.15) {
          readIMU_YawPitchRoll(ypr);
          rotSpeed = (ypr[0]-prevYaw);
          ts = millis();
          rotSpeed /= ((float)(ts-prev_ts))/1000.;
          prev_ts = ts;
          prevYaw = ypr[0];
          Servo1.writeMicroseconds(Servo1_minPulseB);
          Servo1_minPulseB--;
          delay(50);
        }
        float Servo1_minSpeedB = rotSpeed;
        Servo1.detach();
        playNote(MI, 100);
        ledColor(0,0,128);
        Serial.print("Servo1_B\t");
        Serial.print(Servo1_minSpeedB,5);
        Serial.print(" \t--->\t");
        Serial.println(Servo1_minPulseB);

        delay(100);





        Servo2.attach(SERVO_2_PIN);
        int Servo2_minPulseA = 1472;
        Servo2.writeMicroseconds(Servo2_minPulseA);
        readIMU_YawPitchRoll(ypr);
        prevYaw = ypr[0];
        rotSpeed = 0;
        prev_ts = millis();
        while(abs(rotSpeed) < 0.15) {
          readIMU_YawPitchRoll(ypr);
          rotSpeed = (ypr[0]-prevYaw);
          ts = millis();
          rotSpeed /= ((float)(ts-prev_ts))/1000.;
          prev_ts = ts;
          prevYaw = ypr[0];
          Servo2.writeMicroseconds(Servo2_minPulseA);
          Servo2_minPulseA++;
          delay(50);
        }
        float Servo2_minSpeedA = rotSpeed;
        Servo2.detach();
        playNote(SOL, 100);
        ledColor(0,0,128);
        Serial.print("Servo2_A\t");
        Serial.print(Servo2_minSpeedA,5);
        Serial.print(" \t--->\t");
        Serial.println(Servo2_minPulseA);

        delay(100);





        Servo2.attach(SERVO_2_PIN);
        int Servo2_minPulseB = 1472;
        Servo2.writeMicroseconds(Servo2_minPulseB);
        readIMU_YawPitchRoll(ypr);
        prevYaw = ypr[0];
        rotSpeed = 0;
        prev_ts = millis();
        while(abs(rotSpeed) < 0.15) {
          readIMU_YawPitchRoll(ypr);
          rotSpeed = (ypr[0]-prevYaw);
          ts = millis();
          rotSpeed /= ((float)(ts-prev_ts))/1000.;
          prev_ts = ts;
          prevYaw = ypr[0];
          Servo2.writeMicroseconds(Servo2_minPulseB);
          Servo2_minPulseB--;
          delay(50);
        }
        float Servo2_minSpeedB = rotSpeed;
        Servo2.detach();
        playNote(DO*2, 100);
        ledColor(0,0,128);
        Serial.print("Servo2_B\t");
        Serial.print(Servo2_minSpeedB,5);
        Serial.print(" \t--->\t");
        Serial.println(Servo2_minPulseB);

        delay(100);
        
        




        Serial.println("Motor calibration. Maximum speeds:");

        Servo1.attach(SERVO_1_PIN);
        int Servo1_maxPulseA = 1650;
        Servo1.writeMicroseconds(Servo1_maxPulseA);
        delay(500);

        readIMU_YawPitchRoll(ypr);
        prev_ts = millis();
        prevYaw = ypr[0];
        delay(500);
        readIMU_YawPitchRoll(ypr);
        ts = millis();
        
        Servo1.detach();

        rotSpeed = ypr[0]-prevYaw;
        while(rotSpeed > M_PI) rotSpeed -= 2*M_PI;
        while(rotSpeed <= -M_PI) rotSpeed += 2*M_PI;
        rotSpeed /= ((float)(ts-prev_ts))/1000.;

        float Servo1_maxSpeedA = rotSpeed;
        playNote(RE, 100);
        ledColor(0,0,128);
        Serial.print("Servo1_A\t");
        Serial.print(Servo1_maxSpeedA,5);
        Serial.print(" \t--->\t");
        Serial.println(Servo1_maxPulseA);

        delay(100);



        Servo1.attach(SERVO_1_PIN);
        int Servo1_maxPulseB = 1300;
        Servo1.writeMicroseconds(Servo1_maxPulseB);
        delay(500);

        readIMU_YawPitchRoll(ypr);
        prev_ts = millis();
        prevYaw = ypr[0];
        delay(500);
        readIMU_YawPitchRoll(ypr);
        ts = millis();
        Servo1.detach();

        rotSpeed = ypr[0]-prevYaw;
        while(rotSpeed > M_PI) rotSpeed -= 2*M_PI;
        while(rotSpeed <= -M_PI) rotSpeed += 2*M_PI;
        rotSpeed /= ((float)(ts-prev_ts))/1000.;

        float Servo1_maxSpeedB = rotSpeed;
        playNote(FA, 100);
        ledColor(0,0,128);
        Serial.print("Servo1_B\t");
        Serial.print(Servo1_maxSpeedB,5);
        Serial.print(" \t--->\t");
        Serial.println(Servo1_maxPulseB);

        delay(100);






        Servo2.attach(SERVO_2_PIN);
        int Servo2_maxPulseA = 1650;
        Servo2.writeMicroseconds(Servo2_maxPulseA);
        delay(500);

        readIMU_YawPitchRoll(ypr);
        prev_ts = millis();
        prevYaw = ypr[0];
        delay(500);
        readIMU_YawPitchRoll(ypr);
        ts = millis();
        Servo2.detach();

        rotSpeed = ypr[0]-prevYaw;
        while(rotSpeed > M_PI) rotSpeed -= 2*M_PI;
        while(rotSpeed <= -M_PI) rotSpeed += 2*M_PI;
        rotSpeed /= ((float)(ts-prev_ts))/1000.;

        float Servo2_maxSpeedA = rotSpeed;
        playNote(LA, 100);
        ledColor(0,0,128);
        Serial.print("Servo2_A\t");
        Serial.print(Servo2_maxSpeedA,5);
        Serial.print(" \t--->\t");
        Serial.println(Servo2_maxPulseA);

        delay(100);



        Servo2.attach(SERVO_2_PIN);
        int Servo2_maxPulseB = 1300;
        Servo2.writeMicroseconds(Servo2_maxPulseB);
        delay(500);

        readIMU_YawPitchRoll(ypr);
        prev_ts = millis();
        prevYaw = ypr[0];
        delay(500);
        readIMU_YawPitchRoll(ypr);
        ts = millis();
        Servo2.detach();

        rotSpeed = ypr[0]-prevYaw;
        while(rotSpeed > M_PI) rotSpeed -= 2*M_PI;
        while(rotSpeed <= -M_PI) rotSpeed += 2*M_PI;
        rotSpeed /= ((float)(ts-prev_ts))/1000.;

        float Servo2_maxSpeedB = rotSpeed;
        playNote(RE*2, 100);
        ledColor(0,0,128);
        Serial.print("Servo2_B\t");
        Serial.print(Servo2_maxSpeedB,5);
        Serial.print(" \t--->\t");
        Serial.println(Servo2_maxPulseB);

        delay(100);



        
        
        
        // Get fitting parameters from the measurements we just took
        
        // Check that the measurements have been taken correctly
        if((Servo1_minSpeedA * Servo1_minSpeedB >= 0) || (Servo2_minSpeedA * Servo2_minSpeedB >= 0)) {
            calib.ok = 0;
            EEPROM.put(CALIB_EEPROM_ADDR, calib);
            unrecoverable_error("Initial calibration lead to incorrect values");
        }
        
        if(Servo1_maxSpeedA > Servo1_maxSpeedB) {
            calib.servo1.cw.maxSpeed = Servo1_maxSpeedA;
            calib.servo1.cw.maxSpeed_PWM = Servo1_maxPulseA;
            calib.servo1.cw.zeroSpeed_PWM = round(mapf(0,Servo1_minSpeedA,Servo1_maxSpeedA, Servo1_minPulseA,Servo1_maxPulseA)); // Find crossing point with X axis
            
            calib.servo1.ccw.maxSpeed = Servo1_maxSpeedB;
            calib.servo1.ccw.maxSpeed_PWM = Servo1_maxPulseB;
            calib.servo1.ccw.zeroSpeed_PWM = round(mapf(0,Servo1_minSpeedB,Servo1_maxSpeedB, Servo1_minPulseB,Servo1_maxPulseB)); // Find crossing point with X axis
          
        } else { // Invert CW/CCW
            calib.servo1.ccw.maxSpeed = Servo1_maxSpeedA;
            calib.servo1.ccw.maxSpeed_PWM = Servo1_maxPulseA;
            calib.servo1.ccw.zeroSpeed_PWM = round(mapf(0,Servo1_minSpeedA,Servo1_maxSpeedA, Servo1_minPulseA,Servo1_maxPulseA)); // Find crossing point with X axis
            
            calib.servo1.cw.maxSpeed = Servo1_maxSpeedB;
            calib.servo1.cw.maxSpeed_PWM = Servo1_maxPulseB;
            calib.servo1.cw.zeroSpeed_PWM = round(mapf(0,Servo1_minSpeedB,Servo1_maxSpeedB, Servo1_minPulseB,Servo1_maxPulseB)); // Find crossing point with X axis
        }
        
        
        
        if(Servo2_maxSpeedA > Servo2_maxSpeedB) {
            calib.servo2.cw.maxSpeed = Servo2_maxSpeedA;
            calib.servo2.cw.maxSpeed_PWM = Servo2_maxPulseA;
            calib.servo2.cw.zeroSpeed_PWM = round(mapf(0,Servo2_minSpeedA,Servo2_maxSpeedA, Servo2_minPulseA,Servo2_maxPulseA)); // Find crossing point with X axis
            
            calib.servo2.ccw.maxSpeed = Servo2_maxSpeedB;
            calib.servo2.ccw.maxSpeed_PWM = Servo2_maxPulseB;
            calib.servo2.ccw.zeroSpeed_PWM = round(mapf(0,Servo2_minSpeedB,Servo2_maxSpeedB, Servo2_minPulseB,Servo2_maxPulseB)); // Find crossing point with X axis
          
        } else { // Invert CW/CCW
            calib.servo2.ccw.maxSpeed = Servo2_maxSpeedA;
            calib.servo2.ccw.maxSpeed_PWM = Servo2_maxPulseA;
            calib.servo2.ccw.zeroSpeed_PWM = round(mapf(0,Servo2_minSpeedA,Servo2_maxSpeedA, Servo2_minPulseA,Servo2_maxPulseA)); // Find crossing point with X axis
            
            calib.servo2.cw.maxSpeed = Servo2_maxSpeedB;
            calib.servo2.cw.maxSpeed_PWM = Servo2_maxPulseB;
            calib.servo2.cw.zeroSpeed_PWM = round(mapf(0,Servo2_minSpeedB,Servo2_maxSpeedB, Servo2_minPulseB,Servo2_maxPulseB)); // Find crossing point with X axis
        }

        calib.maxWspeed = min(min(abs(calib.servo1.cw.maxSpeed),abs(calib.servo1.ccw.maxSpeed)),min(abs(calib.servo2.cw.maxSpeed),abs(calib.servo2.ccw.maxSpeed)));

        
        // Test linear motion
        /*int t = 0;
        while(1) {
            float v = sin((float)t/10.);
            set_servo1_rot_speed(v);
            set_servo2_rot_speed(-v);
            t++;
            delay(100);
        }*/


        readIMU_YawPitchRoll(ypr);
        float yawGoal = ypr[0]+M_PI/4.;
        boolean nextTurnLeft = true;
        unsigned long last_zero_cross_ts = millis();
        float Tu = 0; // Store the limit response for PID auto-tuning
        float Ku = calib.maxWspeed/(M_PI/180.);// Calculated so full speed is applied at 1deg error
        float Tu_ok = 0;
        float Ku_ok = 0;
        float prev_error = 0;
        float oscillation_peak = 0;
        float oscillation_peak_last = 0;
        float trialStartTS = millis();
        int oscillation_count = 0;
        while(1) {
          if(millis()-trialStartTS > 2000) {
            set_servo1_rot_speed(0);
            set_servo2_rot_speed(0);
            delay(200);

            if(oscillation_peak_last < 0.5*M_PI/180. || oscillation_count < 3) break;
            
            readIMU_YawPitchRoll(ypr);
            if(nextTurnLeft) yawGoal = ypr[0]-M_PI/4.;
            else yawGoal = ypr[0]+M_PI/4.;
            nextTurnLeft = !nextTurnLeft;
            Ku /= 1.5;

            Tu_ok = Tu;
            Ku_ok = Ku;
            
            last_zero_cross_ts = millis();
            trialStartTS = millis();
            oscillation_count = 0;
            oscillation_peak_last = 0;
          }
          
          readIMU_YawPitchRoll(ypr);
          ts = millis();
          float dt = (float)(ts-prev_ts)/1000.;
          float yaw = (ypr[0]-yawGoal);
          while(yaw > M_PI) yaw -= 2.*M_PI;
          while(yaw <= -M_PI) yaw += 2.*M_PI;
          float error = -yaw;

          oscillation_peak = max(oscillation_peak,abs(error));
          
          float v = Ku*error;
          if(prev_error*error < 0) { // Oscillation detection via zero-cross
            if(oscillation_peak > 1.*M_PI/180.) oscillation_count++;
            oscillation_peak_last = oscillation_peak;
            oscillation_peak = 0;
            
            Tu = 0.4*2.*(float)(ts-last_zero_cross_ts)/1000. + 0.6*Tu;
            Serial.print("Tu="); //Tu=0.24  Ku=45.
            Serial.print(Tu);
            Serial.print("\tKu=");
            Serial.print(Ku);
            Serial.print("\tPeak=");
            Serial.println(oscillation_peak_last);
            
            last_zero_cross_ts = ts;
          }
          if(v > calib.maxWspeed) v = calib.maxWspeed;
          if(v < -calib.maxWspeed) v = -calib.maxWspeed;
          set_servo1_rot_speed(v);
          set_servo2_rot_speed(v);
          //delay(10);
          prev_error = error;
          prev_ts = ts;

          delay(10);

          // PID auto-tune data logging
          /*Serial.print("[");
          Serial.print(millis());
          Serial.print(",");
          Serial.print(Ku);
          Serial.print(",");
          Serial.print(Tu);
          Serial.print(",");
          Serial.print(oscillation_peak_last,3);
          Serial.print(",");
          Serial.print(error,3);
          Serial.println("],");*/
        }

        Tu = Tu_ok;
        Ku = Ku_ok;

        calib.kp = Ku/2.2; // Tyreus-Luyben Tuning http://www.chem.mtu.edu/~tbco/cm416/zn.html
        float Ti = 2.2*Tu;
        float Td = Tu/6.3;
        
        calib.ki = calib.kp/Ti;
        calib.kd = calib.kp*Td;

        playNote(RE*2, 100);
        ledColor(0,0,16);



        // Calibrate linear velocity constant
        pointToAngle(initialHeading);

        float vel = calib.maxWspeed/2.;
        float speed_k = 0;
        for(int i=0; i<4; i++) {
            // Move backwards
            motorPIDcontroller(initialHeading, false, -calib.maxWspeed, 30, true, 0, 0, false);
            
            set_servo1_rot_speed(0);
            set_servo2_rot_speed(0);
            
    
            // Move forwards
            motorPIDcontroller(initialHeading, false, vel, 25, true, 0, 0, false);
            float dist1 = getDistanceCM();
            prev_ts = millis();
            motorPIDcontroller(initialHeading, false, vel, 10, true, 0, 0, false);
            float dist2 = getDistanceCM();
            ts = millis();
            float elapsed = (float)(ts-prev_ts)/1000.;
            float speed_fw = (dist1-dist2)/elapsed;
    
            speed_k += vel/speed_fw;
    
            set_servo1_rot_speed(0);
            set_servo2_rot_speed(0);
        }
        calib.speed_k = speed_k/4.;
        
        playNote(RE*3, 100);
        ledColor(0,16,0);

        // Store calibration parameters into non-volatile memory
        calib.ok = CALIB_OK_VAL;
        EEPROM.put(CALIB_EEPROM_ADDR, calib);

        ledColor(0,128,0);
        delay(1000);
    }// End of calibration


    Serial.println("Calibration parameters:");
    
    Serial.print("calib.servo1.cw.maxSpeed=\t");
    Serial.println(calib.servo1.cw.maxSpeed);
    Serial.print("calib.servo1.cw.maxSpeed_PWM=\t");
    Serial.println(calib.servo1.cw.maxSpeed_PWM);
    Serial.print("calib.servo1.cw.zeroSpeed_PWM=\t");
    Serial.println(calib.servo1.cw.zeroSpeed_PWM);
    Serial.print("calib.servo1.ccw.maxSpeed=\t");
    Serial.println(calib.servo1.ccw.maxSpeed);
    Serial.print("calib.servo1.ccw.maxSpeed_PWM=\t");
    Serial.println(calib.servo1.ccw.maxSpeed_PWM);
    Serial.print("calib.servo1.ccw.zeroSpeed_PWM=\t");
    Serial.println(calib.servo1.ccw.zeroSpeed_PWM);
    
    Serial.println();
    
    Serial.print("calib.servo2.cw.maxSpeed=\t");
    Serial.println(calib.servo2.cw.maxSpeed);
    Serial.print("calib.servo2.cw.maxSpeed_PWM=\t");
    Serial.println(calib.servo2.cw.maxSpeed_PWM);
    Serial.print("calib.servo2.cw.zeroSpeed_PWM=\t");
    Serial.println(calib.servo2.cw.zeroSpeed_PWM);
    Serial.print("calib.servo2.ccw.maxSpeed=\t");
    Serial.println(calib.servo2.ccw.maxSpeed);
    Serial.print("calib.servo2.ccw.maxSpeed_PWM=\t");
    Serial.println(calib.servo2.ccw.maxSpeed_PWM);
    Serial.print("calib.servo2.ccw.zeroSpeed_PWM=\t");
    Serial.println(calib.servo2.ccw.zeroSpeed_PWM);

    Serial.println();
    Serial.print("calib.maxWspeed=\t");
    Serial.println(calib.maxWspeed);

    Serial.println();
    Serial.print("calib.kp=\t");
    Serial.println(calib.kp);
    Serial.print("calib.ki=\t");
    Serial.println(calib.ki);
    Serial.print("calib.kd=\t");
    Serial.println(calib.kd);

    Serial.println();
    Serial.print("calib.speed_k=\t");
    Serial.println(calib.speed_k);

    // Lines
    while(!button_is_pressed());
    delay(3000);

    readIMU_YawPitchRoll(ypr);
    float yaw = ypr[0];
    float len = -100.*calib.speed_k;
    float vel = -5.*calib.speed_k;
    
    while(1) {
      pointToAngle(yaw);
      motorPIDcontroller(yaw, false, vel, 0, false, yaw, len, true);
      set_servo1_rot_speed(0);
      set_servo2_rot_speed(0);
      delay(500);
      pointToAngle(yaw+M_PI);
      motorPIDcontroller(yaw+M_PI, false, vel, 0, false, yaw+M_PI, len, true);
      set_servo1_rot_speed(0);
      set_servo2_rot_speed(0);
      delay(500);
    }
    
    while(1);

    // Squares
    /*while(!button_is_pressed());
    delay(3000);
    
    // Perform squares at distinct speeds
    for(int i=1; i<=5; i++) {
      float vel = i*2; // cm/s
      float l = 2*15; // cm
      performSquare(l, vel);
    }


    while(!button_is_pressed());
    delay(3000);

    readIMU_YawPitchRoll(ypr);
    initialHeading = ypr[0];
    pointToAngle(initialHeading+M_PI/4.);
    
    // Perform squares with distinct sizes
    for(int i=1; i<=5; i++) {
      float vel = 5; // cm/s
      float l = 2.*(float)(6-i)*2.*2.5; // cm
      performSquare(l, vel);
    }
    
    while(1);*/


    // Circles
    /*while(!button_is_pressed());
    delay(3000);
    
    // Perform circles at distinct speeds
    for(int i=1; i<=5; i++) {
      float vel = i*2; // cm/s
      float r = 15; // cm
      performArc(2.*M_PI*r, vel, 360.);
    }
    set_servo1_rot_speed(0);
    set_servo2_rot_speed(0);


    while(!button_is_pressed());
    delay(3000);
    
    // Perform circles with distinct diameters
    for(int i=1; i<=5; i++) {
      float vel = 5; // cm/s
      float r = (float)(6-i)*2.*2.5; // cm
      performArc(2.*M_PI*r, vel, 360.);
    }
    set_servo1_rot_speed(0);
    set_servo2_rot_speed(0);
    
    while(1);*/

    readIMU_YawPitchRoll(ypr);
    initialHeading = ypr[0];

    //motorPIDcontroller(initialHeading, false, 5.*calib.speed_k, 0, false, 0, 0, false);
    /*float r = 30;
    motorPIDcontroller(initialHeading, false, 3.*calib.speed_k, 0, false, initialHeading+90.*M_PI/180., 0.25*2.*M_PI*r*calib.speed_k, true);
    motorPIDcontroller(initialHeading+90.*M_PI/180., false, 3.*calib.speed_k, 0, false, initialHeading+180.*M_PI/180., 0.25*2.*M_PI*r*calib.speed_k, true);
    motorPIDcontroller(initialHeading+180.*M_PI/180., false, 3.*calib.speed_k, 0, false, initialHeading+270.*M_PI/180., 0.25*2.*M_PI*r*calib.speed_k, true);
    motorPIDcontroller(initialHeading+270.*M_PI/180., false, 3.*calib.speed_k, 0, false, initialHeading, 0.25*2.*M_PI*r*calib.speed_k, true);
    */
    while(1);

        // Friction evaluation (measure differences in FW/BW linear velocity)
        /*pointToAngle(initialHeading);
        
        motorPIDcontroller(initialHeading, false, calib.maxWspeed/2., 10, true, 0, false);
        set_servo1_rot_speed(0);
        set_servo2_rot_speed(0);

        for(int i=1; i<30; i++) {
            float vel = -((float)i)/20.;// Move backwards
            motorPIDcontroller(initialHeading, false, vel, 15, true, 0, false);
            prev_ts = millis();
            motorPIDcontroller(initialHeading, false, vel, 25, true, 0, false);
            ts = millis();
            float elapsed = (float)(ts-prev_ts)/1000.;
            float speed_bw = -10./elapsed;
            
            set_servo1_rot_speed(0);
            set_servo2_rot_speed(0);



            vel *= -1;// Move forwards
            motorPIDcontroller(initialHeading, false, vel, 20, true, 0, false);
            prev_ts = millis();
            motorPIDcontroller(initialHeading, false, vel, 10, true, 0, false);
            ts = millis();
            elapsed = (float)(ts-prev_ts)/1000.;
            float speed_fw = 10./elapsed;
            
            set_servo1_rot_speed(0);
            set_servo2_rot_speed(0);
            Serial.print("[");
            Serial.print(vel);
            Serial.print(",");
            Serial.print(speed_bw);
            Serial.print(",");
            Serial.print(speed_fw);
            Serial.println("],");
        }*/
    
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
        mpu.resetFIFO();
        fifoCount = mpu.getFIFOCount();
        // wait for correct available data length, should be a VERY short wait
        while (fifoCount < packetSize) fifoCount = mpu.getFIFOCount();

        // read a packet from FIFO
        mpu.getFIFOBytes(fifoBuffer, packetSize);

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
}

