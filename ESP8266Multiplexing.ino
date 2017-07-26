/*
  ReadAnalogVoltage
  Reads an analog input on pin 0, converts it to voltage, and prints the result to the serial monitor.
  Graphical representation is available using serial plotter (Tools > Serial Plotter menu)
  Attach the center pin of a potentiometer to pin A0, and the outside pins to +5V and ground.

  This example code is in the public domain.
*/

#define S0 7
#define S1 8
#define S2 9
#define S3 10
#define Alarm 6
#define AnalogSignal A0
int sensorData[14];

void SetPin(int pin, int i)
{
  if( i == 1)
  {
    digitalWrite(pin, HIGH);
  }
  else
  {
    digitalWrite(pin, LOW);
  } 
}


void SetControl(int s0, int s1, int s2, int s3)
{
  SetPin(S0, s0);
  SetPin(S1, s1);
  SetPin(S2, s2);
  SetPin(S3, s3);
}

int ReadSensor(uint8_t i)
{
  
  SetControl(i & 0x01, (i & 0x02) >> 1, (i & 0x04) >> 2, (i & 0x08) >> 3 );
  int sensorValue = analogRead(AnalogSignal);
  return sensorValue;
}

void setup() {
  // initialize serial communication at 9600 bits per second:

  pinMode(Alarm, OUTPUT);
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(0,INPUT_PULLUP);  
  pinMode(1,INPUT_PULLUP);  
  pinMode(2,INPUT_PULLUP);  
  pinMode(3,INPUT_PULLUP);  
  pinMode(5,INPUT_PULLUP);
  Serial.begin(9600);
  delay(10000);
  Serial.println("CLEARDATA");  

//  columns
  Serial.println("LABEL,Computer_Time, BaseRightFront,BaseRightBack, BackRightL1,BackRightL2,Top,BackRightL3,BackRightL4,BackLeftL4,BackLeftL3,BackLeftL2, Middle, BackLeftL1, BaseLeftBack, BaseLeftRight,Posture");  
  
}
void loop() {
  int j =0;
 // Serial.println("New:");
  //Serial.print("Label: ");
  int l1 = digitalRead(0);
  int l2 = digitalRead(1);
  int l3 = digitalRead(2);
  int l4 = digitalRead(3);
  int l5 = digitalRead(5);

  Serial.print("DATA,TIME,");
  for(j=0; j< 14;j++)
  {
    sensorData[j] = ReadSensor(j == 7 ? 14 : j);
    Serial.print(sensorData[j]); 
    Serial.print(",");
    delay(10);
  }
  Serial.println(l1*10000 + l2*1000 + l3* 100 + l4*10 + l5);
  //delay(1000);
   Buzzoff();
}


void Buzz()
{
  digitalWrite(Alarm, HIGH);
  delay(1000);
}

void UnBuzz()
{
  digitalWrite(Alarm, LOW);
}


void Buzzoff()
{
  if(sensorData[0] > 20 && sensorData[1] > 20 && (sensorData[2] > 20 || sensorData[3] > 20 || sensorData[4] > 20 || sensorData[5] > 20))
  {
    Buzz();
  }
  else if(sensorData[0] > 20 && sensorData[1] > 20 && sensorData[12] > 20 && sensorData[13] > 20 && sensorData[11] > 20 && sensorData[5] < 20 && sensorData[8] < 20 )
  {
    Buzz();
    
  }
  else if(sensorData[0] > 20 && sensorData[13] > 20 && sensorData[1] < 20 && sensorData[12] < 20 && (sensorData[4] > 20 || sensorData[7] > 20 || sensorData[6] > 20))
  {
    Buzz();
    
  }
  else
  {
    UnBuzz();
  }
  
}

