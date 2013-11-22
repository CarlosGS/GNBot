// Source: Jeff's Skinner Bo
// http://jeffskinnerbox.wordpress.com/2013/01/30/configuration-utilities-for-xbee-radios/
// http://jeffskinnerbox.wordpress.com/2013/03/20/the-simplest-xbee-network/
#define BAUDRATE 9600
 
void setup()
{
    delay(1000);
    pinMode(13, OUTPUT);
    pinMode(12, INPUT);
    
    digitalWrite(12, HIGH);
    
    Serial.begin(BAUDRATE);
    
    delay(3000);
    
    Serial.println("Arduino #1 up and running.");
}

int i = 0;
void loop() {
    // Echo data back
    //while (Serial.available()) {
    //    Serial.print((char)Serial.read());
    //}
    
    if(digitalRead(12) == LOW) {
      Serial.print("USER INPUT DETECTED ");
      Serial.println(i);
      while(digitalRead(12) == LOW);
      i++;
    }
}

