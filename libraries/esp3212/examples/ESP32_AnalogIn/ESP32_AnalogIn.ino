/*
   This example code is in the public domain.

 */
 
#define RLED 16

#define InPin 33
#define DAC_CHANNEL_1 25

void setup() {
  Serial.begin(115200);
  pinMode(RLED, OUTPUT);
  
}

int x ;

void loop()
{
  digitalWrite(RLED, LOW);
  delay(100);
  x = analogRead( InPin );
  Serial.println( x );
  digitalWrite(RLED, HIGH);
  analogWrite( DAC_CHANNEL_1, 128 );
  delay(100);
}
