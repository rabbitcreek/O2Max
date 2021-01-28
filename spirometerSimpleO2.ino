/*
  Analog input, analog output, serial output

  Reads an analog input pin, maps the result to a range from 0 to 255 and uses
  the result to set the pulse width modulation (PWM) of an output pin.
  Also prints the results to the Serial Monitor.

  The circuit:
  - potentiometer connected to analog pin 0.
    Center pin of the potentiometer goes to the analog pin.
    side pins of the potentiometer go to +5V and ground
  - LED connected from digital pin 9 to ground

  created 29 Dec. 2008
  modified 9 Apr 2012
  by Tom Igoe

  This example code is in the public domain.

  http://www.arduino.cc/en/Tutorial/AnalogInOutSerial
*/

#include <Wire.h>
#include <Adafruit_ADS1015.h>
Adafruit_ADS1015 ads;    
double  calibrationv; //used to store calibrated value
int sensorcheck=0;//to check health on sensor. If value is 0 sensor works, if value is 1 sensor out of range or not connected
int Sensor_lowrange=58;//When sensor is healthy and new it reads 58 on low
int Sensor_highrange=106;//When sensor is healthy and new it reads 106 on high
// These constants won't change. They're used to give names to the pins used:
const int analogInPin = 33;  // Analog input pin that the potentiometer is attached to
const int analogOutPin = 9; // Analog output pin that the LED is attached to
float area_1 = 0.000531;
float area_2 = 0.000201;
float rho = 1.225; //Demsity of air in kg/m3;
float dt = 0;
float timerBreath;
float secondsBreath;
float minuteTotal;
int newBreath = 0;
float TimerNow = 0.0;
int totalBreath = 0;
float massFlow = 0;
float volFlow = 0;
float volumeTotal = 0;
int sensorValue = 0;        // value read from the pot
int outputValue = 0;        // value output to the PWM (analog out)
float voltage = 0.0;
float Pa = 0.0;
int16_t adc0=0;
    double result;//After calculations holds the current O2 percentage
    double currentmv; //the current mv put out by the oxygen sensor;
    double calibratev;
void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(115200);
  minuteTotal = millis();
  ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV
  
  ads.begin();
  calibrationv=calibrate();
  Serial.print("calibrate: ");
  Serial.println(calibrationv);
   if ((calibrationv > Sensor_highrange) || (calibrationv < Sensor_lowrange))
   {
    sensorcheck=1;
     //current_function=1;//Sensor needs to be calibrated
     need_calibrating();//print need calibrating message
   } 
  
}
void need_calibrating(){
   Serial.println("sensor out of range");
  
}
int calibrate(){
  int16_t adc0=0;
  int result;
  for(int i=0; i<=19; i++)
       {
         adc0=adc0+ads.readADC_SingleEnded(0);
       }
  
  result=adc0/20;
  return result;
  //timeChecker = millis();
}

void loop() {
  /*
  if((millis()- minuteTotal) > 60000){
    minuteTotal = millis();
    //Serial.print( "Breaths per minute:");
    //Serial.println(totalBreath);
    Serial.print("                                        ");
    Serial.println(totalBreath);
    totalBreath = 0;
  }*/
  // read the analog in value:
  sensorValue = analogRead(analogInPin);
  // map it to the range of the analog out:
 
  voltage =( 3.3 * (( sensorValue )/4095.0));//the usual voltage conversion for 12 bit esp32
  //Serial.print("sensor value");
  //Serial.println(voltage);
  Pa = (190.0 * voltage/3.3) - 31.0;  //equation from sensor data sheet
//Serial.print("Pa");
  //Serial.println(Pa);
  if( Pa > 5 ) {
    if (newBreath < 1) {
      timerBreath = millis();

 if(volumeTotal > 300){
      totalBreath = totalBreath + 1;
      Serial.print("Breathvolume:");
      Serial.print(volumeTotal);
       Serial.print("     TotalBreaths: ");
       Serial.println(totalBreath);
     
      }
      volumeTotal = 0;
      newBreath = 1;
      
     
    }
  
    
  
  // change the analog out value:
  //analogWrite(analogOutPin, outputValue);
massFlow = 1000*sqrt((abs(Pa)*2*rho)/((1/(pow(area_2,2)))-(1/(pow(area_1,2)))));
volFlow = massFlow/rho; //volumetric flow of air
volumeTotal = volFlow * (millis() - TimerNow) + volumeTotal;
  // print the results to the Serial Monitor:
  //Serial.print("sensor = ");
  //Serial.print(sensorValue);
  //Serial.print("VolumeTotal= ");
  //Serial.println(volumeTotal);
 
  //Serial.print("\t pressure = ");
  //Serial.println(Pa);

  // wait 2 milliseconds before the next loop for the analog-to-digital
  // converter to settle after the last reading:
  
  } else if(newBreath){
    newBreath = 0;
   
    secondsBreath = (millis() - timerBreath)/1000;
    if(secondsBreath > 0.1){
    Serial.print("time for breath");
   Serial.println(secondsBreath);
    O2dump();
    }
    //Serial.print(volumeTotal);
    
   //Serial.print("              ");
    //Serial.println(secondsBreath);
    //logfile.flush();
  }
  TimerNow = millis();
  delay(20);
}
void O2dump(){
   double modr;//Variable to hold mod value in
    int16_t adc0=0;
    double result;//After calculations holds the current O2 percentage
    double currentmv; //the current mv put out by the oxygen sensor;
    double calibratev;
  for(int i=0; i<=19; i++)
       {
         adc0=adc0+ads.readADC_SingleEnded(0);
       }
      
      
      currentmv = adc0/20;
      calibratev=calibrationv;
      result=(currentmv/calibratev)*20.9;
  Serial.print("O2%:  ");
  Serial.println(result);
}
