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
#include "DFRobot_OxygenSensor.h"

#define COLLECT_NUMBER    10             // collect number, the collection range is 1-100.
#define Oxygen_IICAddress ADDRESS_3
/*   iic slave Address, The default is ADDRESS_3.
       ADDRESS_0               0x70      // iic device address.
       ADDRESS_1               0x71
       ADDRESS_2               0x72
       ADDRESS_3               0x73
*/

DFRobot_OxygenSensor Oxygen;
#include <Wire.h>

#include "SparkFun_SCD30_Arduino_Library.h" //Click here to get the library: http://librarymanager/All#SparkFun_SCD30
SCD30 airSensor;

   

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
float volumeMinute = 0;
int sensorValue = 0;        // value read from the pot
int outputValue = 0;        // value output to the PWM (analog out)
float voltage = 0.0;
float lastTemp = 0.0;
float lastCotwo = 0.0;
float lastOtwo = 0.0;
float vo2Max = 0.0;
float Pa = 0.0;

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(115200);
   Wire.begin();
 while(!Oxygen.begin(Oxygen_IICAddress)) {
    Serial.println("I2c device number error !");
    delay(1000);
  }
  Serial.println("I2c connect success !");
  if (airSensor.begin() == false)
  {
    Serial.println("Air sensor not detected. Please check wiring. Freezing...");
    while (1)
      ;
  }
  minuteTotal = millis();
  
  
}


void loop() {
  if(totalBreath == 0)minuteTotal = millis();
  if((millis()- minuteTotal) > 60000){
    minuteTotal = millis();
    Serial.print( "Breaths per minute:");
    
    Serial.println(totalBreath);
    Serial.print("Volume Exhaled One Minute: ");
    Serial.println(volumeMinute);
    goFigure();
    totalBreath = 0;
    volumeMinute = 0;
  }
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
      volumeMinute = volumeMinute + volumeTotal;
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
    CO2dump();
    }
    //Serial.print(volumeTotal);
    
   //Serial.print("              ");
    //Serial.println(secondsBreath);
    //logfile.flush();
  }
  TimerNow = millis();
  delay(20);
}
void CO2dump(){
  if (airSensor.dataAvailable())
  {
    Serial.print("co2(ppm):");
    Serial.print(airSensor.getCO2());
lastCotwo = airSensor.getCO2();
    Serial.print(" temp(C):");
    Serial.print(airSensor.getTemperature(), 1);
lastTemp = airSensor.getTemperature();
    Serial.print(" humidity(%):");
    Serial.print(airSensor.getHumidity(), 1);

    Serial.println();
  }
  else
    Serial.println("Waiting for new data");

  delay(500);
}
void O2dump(){
  float oxygenData = Oxygen.ReadOxygenData(COLLECT_NUMBER);
  Serial.print(" Oxygen concentration is ");
  Serial.print(oxygenData);
  Serial.println(" %vol");
  //delay(1000);
  //Serial.print("O2%:  ");
  //Serial.println(result);
  lastOtwo = oxygenData;
}
void goFigure(){
  float percentN2exp;
  float co2;
  
  volumeMinute = volumeMinute/1000.0; //gives liters of air VE
  Serial.print("liters/min uncorrected");
  Serial.print(volumeMinute);
  co2 = lastCotwo/10000.0;
  //lastOtwo = 17.5;
  Serial.print("CO2  ");
  Serial.print(co2);
  Serial.print("02  ");
  Serial.println(lastOtwo);
  percentN2exp = (100.0 - (co2 + lastOtwo));
  Serial.print("%N  ");
  Serial.println(percentN2exp);
  volumeMinute = volumeMinute * (273/(273 + lastTemp)) * ((760.0 - 25.2)/760);
  Serial.print("liters/min corrected  ");
  Serial.print(volumeMinute);
  vo2Max = volumeMinute * (((percentN2exp/100.0) * 0.265) - (lastOtwo/100.0));
  Serial.print(lastOtwo/100.0);
  Serial.print("This: ");
  Serial.print((percentN2exp/100.0 * 0.265) - (lastOtwo/100.0));
  Serial.print("VO2Max!  ");
  Serial.print(vo2Max);
  vo2Max = (vo2Max * 1000.0)/(152.0/2.2);
  Serial.print("VO2Max!!!ml/kg:  ");
  Serial.print(vo2Max);
  
  
}
